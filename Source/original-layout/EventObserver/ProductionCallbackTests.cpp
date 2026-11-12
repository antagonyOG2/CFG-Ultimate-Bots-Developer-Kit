// Deliberately compile the actual callback implementation into this test EXE.
// No test bypass/seam is compiled into the production DLL. Native preparation
// exports are NOT invoked. Every object/module below is synthetic owned memory.
#include "OfflineEventObserver.cpp"
#include <cstdio>

namespace {
unsigned failures=0,forwarded=0;
void* lastObject=nullptr;
void* lastFunction=nullptr;
void* lastParameters=nullptr;
constexpr DWORD FixtureException=0xE0414354;
constexpr std::uintptr_t RaiseToken=0x1111,NestedToken=0x2222,InnerToken=0x3333;
#define CHECK(c) do { if (!(c)) { std::printf("FAIL %d: %s\n",__LINE__,#c); ++failures; } } while(false)
void __fastcall OwnedOriginal(void* object,void* function,void* parameters) {
    ++forwarded; lastObject=object; lastFunction=function; lastParameters=parameters;
    if (reinterpret_cast<std::uintptr_t>(function)==RaiseToken) RaiseException(FixtureException,0,0,nullptr);
    if (reinterpret_cast<std::uintptr_t>(function)==NestedToken)
        Observe(object,reinterpret_cast<void*>(InnerToken),nullptr);
}
bool CatchOriginalException(void* object) {
    __try { Observe(object,reinterpret_cast<void*>(RaiseToken),reinterpret_cast<void*>(0xAAAA)); }
    __except (GetExceptionCode()==FixtureException?EXCEPTION_EXECUTE_HANDLER:EXCEPTION_CONTINUE_SEARCH) { return true; }
    return false;
}
struct Memory {
    std::array<void*,32> blocks{}; std::size_t count=0;
    ~Memory() { for (std::size_t i=0;i<count;++i) VirtualFree(blocks[i],0,MEM_RELEASE); }
    std::uintptr_t Reserve(std::size_t bytes) {
        if (count==blocks.size()) return 0;
        void* p=VirtualAlloc(nullptr,bytes,MEM_RESERVE,PAGE_READWRITE);
        if (p) blocks[count++]=p;
        return reinterpret_cast<std::uintptr_t>(p);
    }
    template<class T> bool Put(std::uintptr_t address,const T& value) {
        const auto page=address&~std::uintptr_t{4095};
        const auto last=(address+sizeof(T)-1)&~std::uintptr_t{4095};
        if (!VirtualAlloc(reinterpret_cast<void*>(page),static_cast<SIZE_T>(last-page+4096),MEM_COMMIT,PAGE_READWRITE)) return false;
        std::memcpy(reinterpret_cast<void*>(address),&value,sizeof(value)); return true;
    }
};
bool SetupOwnedContext(Memory& memory) {
    plan={}; plan.bytes=sizeof(plan); plan.threadId=GetCurrentThreadId();
    plan.module=memory.Reserve(0x03644000);
    std::array<std::uintptr_t*,10> fields{&plan.engine,&plan.viewport,&plan.world,&plan.mode,&plan.controller,
        &plan.pawn,&plan.player,&plan.gameInstance,&plan.localPlayerData,&plan.objectArray.data};
    for (auto* field:fields) { *field=memory.Reserve(8192); if (!*field) return false; }
    if (!plan.module) return false;
    plan.objectArray.count=4;
    plan.dispatch={1,plan.threadId,memory.Reserve(8192),plan.module+cfgprobe::dispatch::NativeActorDispatcherRva};
    const std::array<std::uintptr_t,4> objects{plan.world,plan.mode,plan.controller,plan.pawn};
    for (std::size_t i=0;i<4;++i) {
        auto& slot=plan.objects[i]; slot={objects[i],static_cast<std::int32_t>(i),static_cast<std::int32_t>(100+i)};
        if (!memory.Put(slot.object+cfgprobe::lifetime::ObjectIndexOffset,slot.index) ||
            !memory.Put(plan.objectArray.data+i*cfgprobe::lifetime::ItemBytes,slot.object) ||
            !memory.Put(plan.objectArray.data+i*cfgprobe::lifetime::ItemBytes+cfgprobe::lifetime::ItemSerialOffset,slot.serial)) return false;
    }
    using namespace cfgprobe;
    return memory.Put(plan.module+EngineGlobalCandidate,plan.engine) &&
        memory.Put(plan.engine+EngineGameViewport,plan.viewport) && memory.Put(plan.viewport+ViewportWorld,plan.world) &&
        memory.Put(plan.world+WorldNetDriver,std::uintptr_t{0}) && memory.Put(plan.world+WorldGameMode,plan.mode) &&
        memory.Put(plan.world+WorldGameInstance,plan.gameInstance) &&
        memory.Put(plan.gameInstance+GameInstanceLocalPlayers,cfgobserver::LocalPlayers{plan.localPlayerData,1,1}) &&
        memory.Put(plan.localPlayerData,plan.player) && memory.Put(plan.player+PlayerController,plan.controller) &&
        memory.Put(plan.controller+ControllerPlayer,plan.player) && memory.Put(plan.controller+ControllerAcknowledgedPawn,plan.pawn) &&
        memory.Put(plan.controller+ControllerPawn,plan.pawn) &&
        memory.Put(plan.module+lifetime::ObjectArrayRva+lifetime::ArrayDataOffset,plan.objectArray.data) &&
        memory.Put(plan.module+lifetime::ObjectArrayRva+lifetime::ArrayCountOffset,plan.objectArray.count) &&
        memory.Put(plan.module+dispatch::ThreadFlagRva,plan.dispatch.flag) && memory.Put(plan.module+dispatch::ThreadIdRva,plan.threadId) &&
        memory.Put(plan.pawn,plan.dispatch.vtable) && memory.Put(plan.dispatch.vtable+dispatch::EventSlotBytes,plan.dispatch.target);
}
}
int main() {
    Memory memory;
    if (!SetupOwnedContext(memory)) { std::puts("Owned fixture allocation failed"); return 1; }
    Reader reader; CHECK(cfgobserver::ContextUnchanged(reader,plan) && reader.reads==30);
    originalActorDispatcher=&OwnedOriginal; CHECK(capture.Arm(plan.pawn,plan.threadId));
    auto* pawn=reinterpret_cast<void*>(plan.pawn);
    Observe(pawn,reinterpret_cast<void*>(0x4444),reinterpret_cast<void*>(0x5555));
    CHECK(forwarded==1 && capture.Count()==1 && report.publishedCount==1 && !observerNested);
    CHECK(lastObject==pawn && lastFunction==reinterpret_cast<void*>(0x4444) && lastParameters==reinterpret_cast<void*>(0x5555));
    Sleep(20); CHECK(CatchOriginalException(pawn));
    CHECK(forwarded==2 && capture.Count()==2 && !observerNested);
    Sleep(20); Observe(pawn,reinterpret_cast<void*>(0x6666),nullptr);
    CHECK(forwarded==3 && capture.Count()==3 && report.events[2].function==0x6666 && !observerNested);
    Sleep(20); Observe(pawn,reinterpret_cast<void*>(NestedToken),nullptr);
    CHECK(forwarded==5 && capture.Count()==4 && report.events[3].function==NestedToken && !observerNested);
    Observe(reinterpret_cast<void*>(plan.mode),reinterpret_cast<void*>(0x7777),nullptr);
    CHECK(forwarded==6 && capture.Count()==4 && !observerNested);
    CHECK(memory.Put(plan.world+cfgprobe::WorldNetDriver,std::uintptr_t{0x10000}));
    Sleep(20); Observe(pawn,reinterpret_cast<void*>(0x8888),nullptr);
    CHECK(forwarded==7 && capture.Count()==4 && !capture.Armed() && !observerNested &&
        report.status==static_cast<DWORD>(cfgobserver::Status::Stopped) &&
        report.stopReason==static_cast<DWORD>(cfgport::callbacks::Stop::IdentityChanged));
    CHECK(memory.Put(plan.world+cfgprobe::WorldNetDriver,std::uintptr_t{0}));
    Observe(pawn,reinterpret_cast<void*>(0x9999),nullptr);
    CHECK(forwarded==8 && capture.Count()==4 && report.publishedCount==4 && !observerNested);
    std::printf("CFG production callback/exception forwarding test: %s (%u failures); %u calls, no game access\n",
        failures?"FAIL":"PASS",failures,forwarded);
    return failures?1:0;
}
