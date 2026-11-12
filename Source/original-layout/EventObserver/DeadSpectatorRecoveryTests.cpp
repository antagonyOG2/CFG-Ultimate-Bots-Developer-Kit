#define CFG_SPECTATOR_MEMBER_FIXTURE_ONLY
#include "SpectatorRuntimeMembersTests.cpp"
#include "../Core/DeadSpectatorRecoveryPolicy.hpp"
struct RecoveryFixture:SpectatorFixture {
    cfgspectatorrecovery::State bundleDeadSpectatorRecovery;
    unsigned traces=0;bool restored=false;
    void DeadSpectatorRecoveryTrace(std::uintptr_t,std::uintptr_t,bool returned){++traces;restored=returned;}
#include "DeadSpectatorRecoveryMembers.hpp"
    RecoveryFixture(){
        r_.cap=100000;
        r_.Put(plan.module+cfgescape::disk::SCPlayerStateReflection,r_.classes[cfgspawn::PlayerState]);
        r_.Put(plan.module+cfgescape::disk::SCPlayerStateConstructor,r_.classes[cfgspawn::PlayerState]);
        CameraView(fixtureSpectator);Step(0,false);Step(50,false);
        r_.Put(plan.controller+cfgspectator::disk::SpectatingPlayer,plan.pawn);
    }
    bool Recover(std::uint64_t now){
        r_.reads=0;r_.cap=1536;calls_=0;spectatorSafetyProved_=spectatorTerminalProved_=false;
        return RecoverDeadSpectator(now);
    }
};
int main(){
    {cfgspectatorrecovery::State p;CHECK(p.Due(0)&&!p.Due(499)&&p.Due(500));
     CHECK(!p.Dead(1,2,0)&&!p.Dead(1,2,499)&&p.Dead(1,2,500));p.Consume();
     CHECK(!p.Dead(1,2,5000)&&!p.Dead(1,3,5001)&&p.Dead(1,3,5501));p.Lost();CHECK(!p.object);
     CHECK(!p.Dead(1,2,6000)&&!p.Dead(1,2,6500));}
    {cfgspectatorrecovery::State p;for(unsigned i=0;i<16;++i){
        CHECK(!p.Dead(i+1,1,i*1000)&&p.Dead(i+1,1,i*1000+500));p.Consume();p.Lost();}
     CHECK(!p.Dead(100,1,20000)&&!p.Dead(100,1,20500));}
    {RecoveryFixture f;CHECK(f.Recover(1000)&&!f.traces);CHECK(f.Recover(1500));
     std::printf("Dead selection recovery traces=%u returned=%u reads=%u/1536 selections=%u blends=%u failure=%u link=%u\n",
       f.traces,f.restored?1u:0u,f.r_.reads,f.selections,f.blends,f.bundleReport.spectatorFailure,f.bundleReport.spectatorLinkFailure);
     CHECK(f.traces==1&&f.restored&&f.r_.reads<=1536&&f.selections==1&&f.blends==1&&!f.teleports);
     CHECK(f.Recover(2000)&&f.traces==1);}
    for(unsigned fault=0;fault<7;++fault){RecoveryFixture f;CHECK(f.Recover(1000));
        if(fault==0)fixturePaused=true;
        if(fault==1)f.r_.Put(f.plan.controller+cfgspectator::disk::SpectatingPlayer,f.r_.bot);
        if(fault==2)f.r_.Put(f.plan.world+cfgprobe::WorldNetDriver,f.plan.controller);
        if(fault==3)f.r_.Put(f.localState+cfgescape::disk::bDead,std::uint8_t{});
        if(fault==4)f.CameraView(f.r_.bot);
        if(fault==5)f.Serial(f.plan.pawn,20);
        if(fault==6)f.r_.Put(f.r_.bot+cfghunter::disk::Health,0.f);
        CHECK(f.Recover(1500));CHECK(!f.traces&&!f.selections&&!f.blends&&!f.teleports);fixturePaused=false;
    }
    std::printf("Dead selected counselor recovery: %d failures; no game access.\n",failures);return failures?1:0;
}
