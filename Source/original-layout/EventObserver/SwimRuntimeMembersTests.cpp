#define CFG_SPECTATOR_MEMBER_FIXTURE_ONLY
#include "SpectatorRuntimeMembersTests.cpp"
#include "../RuntimeProbe/SwimActionBindings.hpp"
#include "../RuntimeProbe/KillBindings.generated.hpp"
struct SwimFixture : SpectatorFixture {
    std::uint64_t bundleSwimNext=0;
    struct {unsigned swimChecks=0,swimActions=0,swimRefused=0,swimFailure=0,swimGuard=0;int swimNativeAdmission=0;} bundleReport;
    unsigned queryCalls=0,actionCalls=0,mutation=0,chaseCalls=0;int nativeAdmission=1;
    bool Move(std::uintptr_t target){++chaseCalls;return Spend(1)&&target==r_.bot;}
    float fixtureDistance=250.f;bool fixtureBusy=false,fixtureDistanceRead=true,fixtureBusyRead=true;
    std::uintptr_t water_=0,contextActor_=0,level_=0,contextType_=0,levelType_=0;
    bool HostedRosterLive(std::uintptr_t){return false;}
    bool KnownLiving(std::uintptr_t target){float health=0;return target==r_.bot&&r_.Read(target+cfghunter::disk::Health,health)&&health>0;}
    bool Spend(unsigned count){if(calls_+count>cfgbundle::CallCap)return false;calls_+=count;return true;}
    bool Distance(std::uintptr_t target,float& squared){squared=fixtureDistance*fixtureDistance;return Spend(2)&&fixtureDistanceRead&&target==r_.bot;}
    bool GrabBusy(bool& busy){busy=fixtureBusy;return Spend(2)&&fixtureBusyRead;}
    bool NativeBool(std::uintptr_t function,bool& result){++calls_;result=(mutation==6&&function==cfghunter::disk::Stunned)||(mutation==7&&function==cfghunter::disk::Attacking);return true;}
    bool Location(std::uintptr_t actor,cfgspawn::Vec3& position){position={100,200,300};return Spend(1)&&actor==bundleJason.pawn.object;}
    bool NativeClass(std::uintptr_t reflection,std::uintptr_t constructor,std::uintptr_t& type){std::uintptr_t other=0;return r_.Read(plan.module+reflection,type)&&r_.Read(plan.module+constructor,other)&&cfgprobe::MatchingClassCache(type,other);}
    bool Invoke(std::uintptr_t object,std::uintptr_t function,void* parameters){
        ++calls_;if(object!=bundleJason.pawn.object)return false;
        if(function==functions[cfgswim::disk::CanInteractUnderwater.cache]){++queryCalls;
            struct Admission {std::uintptr_t character;cfgspawn::Vec3 location,rotation;int result;unsigned pad;};
            auto& query=*static_cast<Admission*>(parameters);CHECK(query.character==object);query.result=nativeAdmission;
            if(mutation==1)r_.Put(plan.world+cfgprobe::WorldNetDriver,plan.controller);
            if(mutation==2)r_.Put(r_.bot+cfgswim::disk::WaterVolume,r_.bot);
            if(mutation==3)r_.Put(r_.bot+cfghunter::disk::Health,0.f);
            if(mutation==4)r_.Put(level_+cfgkill::disk::OwningWorld,plan.controller);
            if(mutation==5)r_.Put(r_.bot+cfgswim::disk::WaterVolume,std::uintptr_t{});
            return true;
        }
        if(function==functions[cfgswim::disk::OnInteractUnderwater.cache]){++actionCalls;
            CHECK(*static_cast<std::uintptr_t*>(parameters)==object);return true;}
        return false;
    }
#include "SwimRuntimeMembers.hpp"
    SwimFixture(){r_.cap=100000;r_.reads=0;
        water_=r_.Object();contextType_=r_.Object();contextActor_=r_.Object(contextType_);levelType_=r_.Object();level_=r_.Object(levelType_);
        r_.Put(r_.bot+cfgswim::disk::WaterVolume,water_);r_.Put(bundleJason.pawn.object+cfgswim::disk::ContextActor,contextActor_);
        r_.Put(contextActor_+0x20,level_);r_.Put(water_+0x20,level_);r_.Put(level_+cfgkill::disk::OwningWorld,plan.world);
        r_.Put(plan.module+cfgkill::disk::LevelReflection,levelType_);r_.Put(plan.module+cfgkill::disk::LevelConstructor,levelType_);
        const auto bind=[&](const auto& binding,std::uintptr_t executor){Binding(binding,executor);
            r_.Put(plan.module+binding.ownerReflection,r_.killerBase);r_.Put(plan.module+binding.ownerConstructor,r_.killerBase);
            r_.Put(functions[binding.cache]+0x20,r_.killerBase);};
        bind(cfgswim::disk::CanInteractUnderwater,cfgswim::disk::CanInteractUnderwaterExecutor);
        bind(cfgswim::disk::OnInteractUnderwater,cfgswim::disk::OnInteractUnderwaterExecutor);
        r_.Put(plan.module+cfgswim::disk::CanInteractUnderwaterExecutor,cfgswim::disk::CanInteractUnderwaterCode);
        r_.Put(plan.module+cfgswim::disk::CanInteractUnderwaterBody,cfgswim::disk::CanInteractUnderwaterBodyCode);
        r_.Put(plan.module+cfgswim::disk::OnInteractUnderwaterExecutor,cfgswim::disk::OnInteractUnderwaterCode);
        r_.Put(plan.module+cfgswim::disk::OnInteractUnderwaterBody,cfgswim::disk::OnInteractUnderwaterBodyCode);
        r_.reads=0;r_.cap=1536;calls_=0;
    }
};
#ifndef CFG_SWIM_MEMBER_FIXTURE_ONLY
int main(){
    SwimFixture valid;CHECK(valid.SwimPhase(valid.r_.bot,100));
    CHECK(valid.queryCalls==1&&valid.actionCalls==1&&valid.bundleReport.swimFailure==0);
    CHECK(valid.calls_==11); // production warm Distance2,busy2,location1,query1,busy2,stun1,attack1,action1
    const auto peak=valid.r_.reads;CHECK(peak<=1536&&valid.calls_<=12);
    CHECK(!valid.SwimPhase(valid.r_.bot,101)&&valid.actionCalls==1);
    for(unsigned inherited=2;inherited<=6;++inherited){SwimFixture f;f.calls_=inherited;
        CHECK(f.SwimPhase(f.r_.bot,100)&&!f.queryCalls&&!f.actionCalls);
        CHECK(f.bundleReport.swimFailure==2&&f.bundleReport.swimGuard==18&&f.calls_<=12);}
    {SwimFixture f;f.calls_=1;CHECK(f.SwimPhase(f.r_.bot,100)&&f.actionCalls==1&&f.calls_==12);}
    {SwimFixture f;CHECK(f.SwimDue(f.r_.bot,100));f.bundleSwimNext=101;
     CHECK(!f.SwimDue(f.r_.bot,100)&&!f.SwimDue(0,200));}
    for(int denied:{0,2,-1}){SwimFixture f;f.nativeAdmission=denied;CHECK(!f.SwimPhase(f.r_.bot,100));CHECK(!f.actionCalls&&f.queryCalls==1);}
    for(unsigned mutation=1;mutation<=7;++mutation){SwimFixture f;f.mutation=mutation;f.SwimPhase(f.r_.bot,100);CHECK(!f.actionCalls&&f.queryCalls==1);CHECK(f.r_.reads<=1536);}
    for(unsigned fault=0;fault<5;++fault){SwimFixture f;
        if(fault==0)f.r_.Put(f.r_.bot+cfgswim::disk::WaterVolume,std::uintptr_t{});
        if(fault==1)f.r_.Put(f.water_+0x20,f.plan.controller);
        if(fault==2)f.r_.Put(f.level_+cfgkill::disk::OwningWorld,f.plan.controller);
        if(fault==3)f.r_.Put(f.plan.module+cfgswim::disk::OnInteractUnderwaterBody,std::uint8_t{});
        if(fault==4)f.r_.Put(f.functions[cfgswim::disk::OnInteractUnderwater.cache]+cfgreflection::arraydisk::FunctionParameterBytes,std::uint16_t{16});
        f.SwimPhase(f.r_.bot,100);CHECK(!f.actionCalls);CHECK(f.r_.reads<=1536);
        if(fault==1)CHECK(f.bundleReport.swimGuard==8);
        if(fault==2)CHECK(f.bundleReport.swimGuard==8);}
    for(unsigned fault=0;fault<4;++fault){SwimFixture f;
        if(fault==0)f.fixtureDistanceRead=false;
        if(fault==1)f.fixtureDistance=601.f;
        if(fault==2)f.fixtureBusyRead=false;
        if(fault==3)f.fixtureBusy=true;
        const auto acted=f.SwimPhase(f.r_.bot,100);
        CHECK(acted==(fault==1)&&!f.queryCalls&&!f.actionCalls);
        CHECK(f.chaseCalls==(fault==1?1u:0u));
        CHECK(f.bundleReport.swimGuard==11+fault);
    }
    {SwimFixture f;f.fixtureDistance=1500;CHECK(f.SwimPhase(f.r_.bot,100)&&f.chaseCalls==1&&!f.actionCalls);
     CHECK(!f.SwimPhase(f.r_.bot,200)&&f.chaseCalls==1);f.calls_=0;f.r_.reads=0;f.fixtureDistance=250;
     CHECK(f.SwimPhase(f.r_.bot,600)&&f.actionCalls==1&&f.chaseCalls==1);}
    {SwimFixture f;f.r_.Put(f.bundleJason.pawn.object+cfgswim::disk::ContextActor,std::uintptr_t{});
     CHECK(f.SwimPhase(f.r_.bot,100)&&f.queryCalls==1&&f.actionCalls==1);}
    std::printf("Included underwater member: failures=%d peak=%u/1536; native admission and action mocked, no game access\n",failures,peak);
    return failures?1:0;
}
#endif
