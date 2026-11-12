#define main ExistingKillRuntimeSuiteMain
#include "KillRuntimeTests.cpp"
#undef main
#include "../Core/SweaterLifecyclePolicy.hpp"
struct PamelaHost:Host {
    float health=100,squaredAtUse=100;unsigned weaponReads=0;
    bool PamelaDamageEvent(std::uintptr_t eventFunction){return eventFunction==8||eventFunction==9||eventFunction==10;}
    bool PamelaHealthValue(float& value){value=health;return true;}
    bool CurrentWeapon(std::uintptr_t pawn,std::uintptr_t& weapon,Weapon& type){++weaponReads;return Host::CurrentWeapon(pawn,weapon,type);}
    bool PamelaFinisher(std::uintptr_t& pawn,std::uintptr_t& weapon,Weapon& type){pawn=4;return KnownLiving(pawn)&&CurrentWeapon(pawn,weapon,type);}
    cfgsweater::KillArmResult ArmSweaterKill(Runtime& runtime,const Identity& id,std::uintptr_t wearer,std::uint64_t used){
        return runtime.policy.PamelaUsed(id,wearer,used,true,squaredAtUse,health)?cfgsweater::KillArmResult::Accepted:cfgsweater::KillArmResult::Rejected;
    }
};
#if !defined(CFG_PAMELA_HOST_ONLY)
int main(){unsigned checks=0;const auto check=[&](bool value){++checks;assert(value);};
    for(const auto event:{8u,9u,10u}){
        PamelaHost h;h.kind=Weapon::Other;h.packet.damage=0;Runtime r;
        check(r.policy.PamelaUsed(h.identity,4,100,true,250.f*250.f,100)&&r.policy.until==30100);
        check(r.ObservePamelaEvent(h,2,event,9,101,true)&&r.accepted==1&&!h.weaponReads);
        check(r.policy.finishingStrike&&r.policy.until==60101&&r.policy.stage==Stage::AwaitContext);
        check(!r.ObservePamelaEvent(h,2,event,9,102,true)&&r.accepted==1);
        check(r.Tick(h,103)==Step::StanceDispatched&&h.stances==1&&!r.policy.finisher);
        h.context={10,11,12,false};
        check(r.Tick(h,104)==Step::InteractionDispatched&&h.interactions==1&&r.policy.until==60101&&!r.policy.interactionAttempted);
        check(r.Tick(h,105)==Step::Waiting&&h.interactions==1&&!r.policy.interactionAttempted);
        h.context.active=true;check(r.Tick(h,106)==Step::NativeActive&&r.policy.interactionAttempted);
    }
    for(unsigned fault=0;fault<7;++fault){
        PamelaHost h;Runtime r;check(r.policy.PamelaUsed(h.identity,4,100,true,100,100));
        std::uint64_t now=101;std::uintptr_t receiver=2,event=8;bool post=true;
        switch(fault){case 0:now=30100;break;case 1:now=99;break;case 2:receiver=99;break;
            case 3:event=77;break;case 4:post=false;break;case 5:h.scope=false;break;case 6:++h.identity.serial;break;}
        check(!r.ObservePamelaEvent(h,receiver,event,9,now,post)&&!r.accepted&&!h.stances&&!h.interactions);
    }
    for(const auto squared:{250.01f*250.01f,-1.f,std::numeric_limits<float>::quiet_NaN()}){
        PamelaHost h;Runtime r;check(!r.policy.PamelaUsed(h.identity,4,100,true,squared,100));
        check(r.policy.stage==Stage::Idle&&!r.ObservePamelaEvent(h,2,8,9,101,true));
    }
    {PamelaHost h;Runtime r;check(r.policy.PamelaUsed(h.identity,4,100,true,100,100));
        h.health=99.96f;check(!r.ObservePamelaHealth(h,101));
        h.health=99.9f;check(r.ObservePamelaHealth(h,102)&&r.accepted==1&&r.policy.finishingStrike&&!h.weaponReads);
        check(!r.ObservePamelaHealth(h,103)&&r.accepted==1);}
    {PamelaHost h;Runtime r;cfgsweater::Policy sweater;
        sweater.stage=cfgsweater::Stage::Owned;sweater.clearObserved=true;
        check(sweater.ObserveUsed(100,1));check(sweater.armPending);
        check(sweater.ServiceArm(h,r,4,101)&&r.policy.pamela&&r.policy.until==30100&&sweater.rearmAt==11100);
        check(sweater.Decide(11099,true,true,true)==cfgsweater::Action::None);
        check(sweater.Decide(11100,true,true,true)==cfgsweater::Action::Rearm&&r.policy.stage==Stage::AwaitHit&&r.policy.until==30100);
        r.policy.Expire(30100);check(r.policy.stage==Stage::Expired&&r.policy.RecoveryPending(30100));
        check(sweater.Decide(30101,true,true,true)==cfgsweater::Action::Rearm);
        check(r.policy.RequestRecovery(30101));r.policy.recoveryReturned=true;
        check(r.policy.ConfirmRecovery(30102,true,true)&&r.policy.stage==Stage::Idle);
        check(!h.stances&&!h.interactions);}
    std::printf("B1 donor Pamela30s/250cm/any3events/non-axe/health-delta/60s/expiry-recharge: %u checks PASS; native calls stubbed.\n",checks);
    return 0;
}
#endif
