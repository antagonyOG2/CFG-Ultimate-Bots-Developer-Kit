#define CFG_CALLBACK_BOUNDARY_TRACE
#include "OfflineEventObserver.cpp"
#include <cstdio>
namespace {
unsigned forwarded=0;constexpr DWORD TestException=0xE0414354;
void __fastcall Stub(void* object,void* function,void*){
    ++forwarded;
    if(function==reinterpret_cast<void*>(0x30000))Observe(object,reinterpret_cast<void*>(0x30008),nullptr);
    if(function==reinterpret_cast<void*>(0x30010))RaiseException(TestException,0,0,nullptr);
}
bool Catch(){__try{Observe(reinterpret_cast<void*>(plan.pawn),reinterpret_cast<void*>(0x30010),nullptr);}
    __except(GetExceptionCode()==TestException?EXCEPTION_EXECUTE_HANDLER:EXCEPTION_CONTINUE_SEARCH){return true;}return false;}
template<class T>bool Put(std::uintptr_t rva,T value){
    const auto address=plan.module+rva;
    if(!VirtualAlloc(reinterpret_cast<void*>(address&~std::uintptr_t{4095}),4096,MEM_COMMIT,PAGE_READWRITE))return false;
    std::memcpy(reinterpret_cast<void*>(address),&value,sizeof(value));return true;
}
}
#define CHECK(c) do{if(!(c)){std::printf("FAIL %d: %s\n",__LINE__,#c);return 1;}}while(false)
int main(){
    plan.module=reinterpret_cast<std::uintptr_t>(VirtualAlloc(nullptr,0x03644000,MEM_RESERVE,PAGE_READWRITE));
    CHECK(plan.module);plan.pawn=0x20000;plan.threadId=GetCurrentThreadId();originalActorDispatcher=Stub;
    CHECK(Put(cfgcompat::cfg35::functions::ActorReceiveTick.cacheRva,std::uintptr_t{0x30000}));
    CHECK(Put(cfgcompat::cfg35::lookup::SerializationFlagRva,std::uint8_t{}));
    CHECK(Put(cfgcompat::cfg35::lookup::GarbageCollectionStateRva,std::uint32_t{}));
    boundaryTrace.Arm(plan.module,plan.pawn,plan.threadId);
    // Opaque invalid UObject addresses are forwarded/captured without reads.
    Observe(reinterpret_cast<void*>(plan.pawn),reinterpret_cast<void*>(0x30000),nullptr);
    auto& trace=boundaryTrace.Data();CHECK(forwarded==2&&trace.published==2&&!boundaryDepth&&!boundaryRoot);
    CHECK(trace.events[0].depth==0&&trace.events[1].depth==1&&trace.events[0].returned&&trace.events[1].returned);
    CHECK(trace.events[0].before.valid&&trace.events[0].after.valid&&trace.events[0].returnPc);
    CHECK(Put(cfgcompat::cfg35::functions::ActorReceiveTick.cacheRva,std::uintptr_t{0x30010}));
    CHECK(Catch()&&forwarded==3&&trace.published==3&&!trace.events[2].returned&&trace.events[2].finished);
    CHECK(!boundaryDepth&&!boundaryRoot);
    Observe(reinterpret_cast<void*>(1),reinterpret_cast<void*>(2),nullptr);
    CHECK(forwarded==4&&trace.published==3);
    ++plan.threadId;Observe(reinterpret_cast<void*>(plan.pawn),reinterpret_cast<void*>(0x30010+8),nullptr);
    CHECK(forwarded==5&&trace.published==3);--plan.threadId;
    boundarySamples=256;
    Observe(reinterpret_cast<void*>(plan.pawn),reinterpret_cast<void*>(0x30000),nullptr);
    CHECK(forwarded==7&&trace.published==5&&boundarySamples==256&&!boundaryDepth&&!boundaryRoot);
    CHECK(!trace.events[3].before.valid&&!trace.events[3].after.valid);
    VirtualFree(reinterpret_cast<void*>(plan.module),0,MEM_RELEASE);
    std::puts("PASS actual boundary detour forwards once, records nesting, preserves native exceptions and refuses foreign threads/objects; synthetic memory only");return 0;
}
