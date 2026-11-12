#include "SpawnResultObservation.hpp"
#include <map>
#include <cstring>
#include <cstdio>
namespace {
int failures=0;
#define CHECK(c) do { if (!(c)) { std::printf("FAIL %d: %s\n",__LINE__,#c); ++failures; } } while(false)
struct Fixture {
    cfgobserver::Plan plan;
    std::map<std::uintptr_t,std::array<unsigned char,16>> memory;
    unsigned reads=0,cap=512;
    std::uintptr_t trigger=0,mutation=0,replacement=0;
    unsigned triggerCount=0,triggerAt=2;
    std::uintptr_t array=0x60000000,pawn=0x40000000,controller=0x40010000,brain=0x40020000;
    std::uintptr_t pawnClass=0x50000000,counselorClass=0x50010000,aiClass=0x50020000,brainClass=0x50030000,gameState=0x40030000;
    template<class T> void Put(std::uintptr_t address,const T& value) {
        static_assert(sizeof(T)<=16,"cell bound"); auto& bytes=memory[address]; bytes={}; std::memcpy(bytes.data(),&value,sizeof(value));
    }
    template<class T> bool Read(std::uintptr_t address,T& out) {
        if (reads++>=cap) return false;
        if (address==trigger && ++triggerCount==triggerAt) Put(mutation,replacement);
        const auto found=memory.find(address); if (found==memory.end()) return false;
        std::memcpy(&out,found->second.data(),sizeof(out)); return true;
    }
    void Slot(std::uintptr_t object,std::int32_t index,std::int32_t serial) {
        Put(object+0xC,index); const auto at=array+static_cast<std::uintptr_t>(index)*24;
        Put(at,object); Put(at+8,std::uint32_t{0}); Put(at+16,serial);
    }
    Fixture() {
        using namespace cfgprobe;
        plan.bytes=sizeof(plan); plan.threadId=77; plan.module=0x10000000;
        plan.engine=0x20000000; plan.viewport=0x20010000; plan.world=0x20020000; plan.mode=0x20030000;
        plan.controller=0x20040000; plan.pawn=0x20050000; plan.player=0x20060000;
        plan.gameInstance=0x20070000; plan.localPlayerData=0x20080000;
        plan.objectArray={0x30000000,16};
        plan.dispatch={1,77,0x20090000,plan.module+dispatch::NativeActorDispatcherRva};
        const std::array<std::uintptr_t,4> objects{plan.world,plan.mode,plan.controller,plan.pawn};
        for (unsigned i=0;i<4;++i) { plan.objects[i]={objects[i],static_cast<std::int32_t>(i),static_cast<std::int32_t>(100+i)}; Slot(objects[i],static_cast<std::int32_t>(i),static_cast<std::int32_t>(100+i)); }
        Put(plan.module+EngineGlobalCandidate,plan.engine); Put(plan.engine+EngineGameViewport,plan.viewport);
        Put(plan.viewport+ViewportWorld,plan.world); Put(plan.world+WorldNetDriver,std::uintptr_t{0});
        Put(plan.world+WorldGameMode,plan.mode); Put(plan.world+WorldGameInstance,plan.gameInstance); Put(plan.world+WorldGameState,gameState);
        Put(plan.gameInstance+GameInstanceLocalPlayers,cfgobserver::LocalPlayers{plan.localPlayerData,1,1});
        Put(plan.localPlayerData,plan.player); Put(plan.player+PlayerController,plan.controller);
        Put(plan.controller+ControllerPlayer,plan.player); Put(plan.controller+ControllerAcknowledgedPawn,plan.pawn); Put(plan.controller+ControllerPawn,plan.pawn);
        Put(plan.module+lifetime::ObjectArrayRva+lifetime::ArrayDataOffset,array);
        Put(plan.module+lifetime::ObjectArrayRva+lifetime::ArrayCountOffset,std::int32_t{64});
        Put(plan.module+dispatch::ThreadFlagRva,plan.dispatch.flag); Put(plan.module+dispatch::ThreadIdRva,plan.dispatch.threadId);
        Put(plan.pawn,plan.dispatch.vtable); Put(plan.dispatch.vtable+dispatch::EventSlotBytes,plan.dispatch.target);
        Slot(pawn,40,501); Slot(controller,41,502);
        Put(pawn+ObjectClass,pawnClass); Put(pawnClass+ClassSuper,counselorClass); Put(counselorClass+ClassSuper,std::uintptr_t{0});
        Put(controller+ObjectClass,aiClass); Put(aiClass+ClassSuper,std::uintptr_t{0});
        Put(brain+ObjectClass,brainClass); Put(brainClass+ClassSuper,std::uintptr_t{0});
        Put(pawn+0x3A0,controller); Put(controller+0x370,pawn); Put(controller+0x418,brain);
    }
    bool Verify(std::uintptr_t& out) {
        reads=0;
        return cfgspawn::ObserveCounselorSpawnResult(*this,plan,gameState,pawnClass,counselorClass,aiClass,brainClass,pawn,out);
    }
};
}
int main() {
    Fixture f; std::uintptr_t controller=0;
    CHECK(f.Verify(controller) && controller==f.controller && f.reads<512); // grew AND reallocated array
    for (unsigned n=0;n<7;++n) {
        Fixture bad;
        if (n==0) bad.Put(bad.plan.controller+cfgprobe::ControllerAcknowledgedPawn,bad.pawn);
        if (n==1) bad.Put(bad.array+3*24+16,std::int32_t{999});
        if (n==2) bad.Put(bad.pawn+0x3A0,bad.plan.controller);
        if (n==3) bad.Put(bad.controller+0x370,bad.plan.pawn);
        if (n==4) bad.Put(bad.array+40*24+16,std::int32_t{0});
        if (n==5) bad.Put(bad.plan.world+cfgprobe::WorldGameState,std::uintptr_t{0});
        if (n==6) bad.Put(bad.brain+cfgprobe::ObjectClass,bad.aiClass);
        controller=123; CHECK(!bad.Verify(controller) && !controller);
    }
    auto missing=f; missing.memory.erase(missing.controller+0x418); CHECK(!missing.Verify(controller));
    auto bounded=f; bounded.cap=20; CHECK(!bounded.Verify(controller));
    for (unsigned n=0;n<3;++n) {
        Fixture changing;
        changing.trigger=n==0?changing.plan.module+cfgprobe::lifetime::ObjectArrayRva+cfgprobe::lifetime::ArrayCountOffset:
            n==1?changing.pawn+0x3A0:changing.array+40*24+16;
        changing.mutation=changing.trigger; changing.replacement=n==0?65:n==1?changing.plan.controller:999;
        controller=123; CHECK(!changing.Verify(controller) && !controller);
    }
    std::printf("CFG actual post-spawn observation %s (%d failures). Owned synthetic memory only.\n",failures?"FAIL":"PASS",failures);
    return failures?1:0;
}
