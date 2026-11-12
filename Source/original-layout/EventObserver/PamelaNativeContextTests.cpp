#define CFG_PAMELA_HOST_ONLY
#include "PamelaKillTests.cpp"
#include "PamelaFinisherEligibility.hpp"
#include "../Core/JasonDeathCompletion.hpp"
#include "../Core/PamelaContextAdmission.hpp"
#include <map>
inline bool testHostedAdoptionOwnsDriver=false;
inline bool HostedAdoptionOwnsDriver(){return testHostedAdoptionOwnsDriver;}
namespace cfgclosedtrace {inline void Mark(const char*,std::uintptr_t,std::uint64_t){}}
struct NativeContextHost {
    struct Memory {std::uintptr_t hint=0;bool readable=true;
        bool Read(std::uintptr_t,std::uintptr_t& out){out=hint;return readable;}}r_;
    struct {struct {std::uintptr_t object=2;}pawn;}bundleJason;
    struct {unsigned killStage=0;}bundleReport;
    Runtime bundleKill;Identity identity{1,2,3};Context observedDeath{};
    bool scope=true,valid=true,installed=false,installOK=true;
    unsigned installs=0,stops=0,scopeCalls=0;std::uintptr_t pamelaStoppedContext_=0;std::uint64_t pamelaStoppedUseAt_=0;
    cfgpamela::ContextRetry pamelaContextRetry_{};
    bool Scope(Identity& id){++scopeCalls;id=identity;return scope;}
    bool DeathContext(Context& out){out=observedDeath;return valid;}
    bool InstallKillEligibility(const Context&){if(!installed&&installOK){installed=true;++installs;}return installOK;}
    bool PamelaContextStop(){++stops;pamelaStoppedContext_=bundleKill.policy.context;pamelaStoppedUseAt_=bundleKill.policy.usedAt;return true;}
#include "PamelaNativeContextRuntimeMembers.hpp"
};
int main(){unsigned checks=0;const auto check=[&](bool x){++checks;assert(x);};
    NativeContextHost h;auto& r=h.bundleKill;
    check(r.policy.PamelaUsed(h.identity,4,100,true,100,100));
    check(!h.PamelaNativeContextPhase(200)&&!r.accepted&&!h.installs&&!h.stops);
    h.r_.hint=10;h.observedDeath={10,11,12,false};
    check(h.PamelaNativeContextPhase(201)&&r.policy.stage==Stage::NativeContext&&r.policy.pamelaNativeContextOnly);
    check(!r.policy.hostedNativeContextRecovery); // offline default remains closed
    check(r.accepted==0&&r.stanceDispatches==0&&h.installs==1&&h.stops==1);
    for(unsigned i=202;i<260;++i)check(h.PamelaNativeContextPhase(i)&&h.installs==1&&h.stops==1);
    EligibilityTicket female{h.identity,10,12,4,true,true,true,true,true};
    check(PamelaNativeOverrideAllowed(r.policy,female,12,4,260)); // no Hunter/weapon-class/final-hit prerequisite
    auto other=female;other.counselor=99;
    check(PamelaNativeOverrideAllowed(r.policy,other,12,99,260)); // any verified counselor, not just local human
    check(!PamelaNativeOverrideAllowed(r.policy,female,13,4,260));
    female.living=false;check(!PamelaNativeOverrideAllowed(r.policy,female,12,4,260));female.living=true;
    PamelaHost native;native.context=h.observedDeath; // Native-only path never synthesizes a final interaction
    check(r.Tick(native,261)==Step::ContextReady&&!native.stances&&!native.interactions);
    check(h.PamelaNativeContextPhase(28000)&&r.policy.until==58000); // donor near-expiry context extension
    r.policy.Expire(58000);
    check(!PamelaNativeOverrideAllowed(r.policy,female,12,4,58000)&&!h.PamelaNativeContextPhase(58000));
    {NativeContextHost active;auto& policy=active.bundleKill.policy;cfgdeathend::Policy ending;
        check(policy.PamelaUsed(active.identity,4,100,true,100,100));active.r_.hint=10;active.observedDeath={10,11,12,false};
        check(active.PamelaNativeContextPhase(101)&&!policy.interactionAttempted);
        check(!ending.Confirm(policy,true,102)); // a kneel alone cannot end the match
        active.observedDeath.active=true;testHostedAdoptionOwnsDriver=true;
        check(active.PamelaNativeContextPhase(103)&&policy.interactionAttempted&&policy.nativeActiveObserved);
        check(policy.hostedNativeContextRecovery&&policy.hostedNativeContextUsedAt==policy.usedAt);
        check(!ending.Confirm(policy,false,104)); // actual dying flag still mandatory
        check(ending.Confirm(policy,true,105)&&active.stops==1&&!active.bundleKill.accepted);
        // Active -> inactive, no dying flag, stun ended, and only after the
        // native timeout: permit cleanup of the reservation, never a kill.
        active.observedDeath.active=false;
        policy.Expire(60103);
        check(policy.stage==Stage::Expired&&policy.RecoveryPending(60103));
        const Identity current=active.identity;
        check(!policy.RecoveryRuntimeProof(60103,current,true,false,10,true,10,100)); // still active
        check(!policy.RecoveryRuntimeProof(60103,current,true,true,10,false,10,100)); // real death
        check(!policy.RecoveryRuntimeProof(60103,current,true,false,99,false,10,100)); // replaced context
        check(!policy.RecoveryRuntimeProof(60103,Identity{9,2,3},true,false,10,false,10,100)); // replaced world
        check(!policy.RecoveryRuntimeProof(60103,Identity{1,9,3},true,false,10,false,10,100)); // replaced Jason
        check(!policy.RecoveryRuntimeProof(60103,current,false,false,10,false,10,100)); // retired adoption
        check(!policy.RecoveryRuntimeProof(60103,current,true,false,10,false,10,101)); // replaced use
        check(!policy.RecoveryRuntimeProof(60103,current,true,false,10,false,99,100)); // stopped wrong context
        policy.pamelaDispatchReturned=true;
        check(!policy.RecoveryPending(60103)&&!policy.RecoveryRuntimeProof(60103,current,true,false,10,false,10,100));
        policy.pamelaDispatchReturned=false;
        policy.pamelaDispatchPending=true;
        check(!policy.RecoveryPending(60103)&&!policy.RecoveryRuntimeProof(60103,current,true,false,10,false,10,100));
        policy.pamelaDispatchPending=false;
        policy.stage=Stage::Uncertain;
        check(!policy.RecoveryPending(60103)&&!policy.RecoveryRuntimeProof(60103,current,true,false,10,false,10,100));
        policy.stage=Stage::Expired;
        check(policy.RecoveryRuntimeProof(60103,current,true,false,10,false,10,100));
        check(policy.RequestRecovery(60103)&&!policy.RequestRecovery(60104));
        policy.recoveryReturned=true;
        check(!policy.ConfirmRecovery(60104,false,true)&&!policy.ConfirmRecovery(60104,true,false));
        check(policy.ConfirmRecovery(60104,true,true)&&policy.stage==Stage::Idle&&!active.bundleKill.accepted);
        testHostedAdoptionOwnsDriver=false;
    }
    {NativeContextHost offline;auto& policy=offline.bundleKill.policy;
        check(policy.PamelaUsed(offline.identity,4,100,true,100,100));offline.r_.hint=10;
        offline.observedDeath={10,11,12,true};
        check(offline.PamelaNativeContextPhase(101)&&policy.nativeActiveObserved);
        policy.Expire(60101);
        check(!policy.RecoveryPending(60101)); // no hosted opt-in
    }
    {NativeContextHost inactive;auto& policy=inactive.bundleKill.policy;
        check(policy.PamelaUsed(inactive.identity,4,100,true,100,100));inactive.r_.hint=10;
        inactive.observedDeath={10,11,12,false};testHostedAdoptionOwnsDriver=true;
        check(inactive.PamelaNativeContextPhase(101)&&policy.pamelaNativeContextOnly&&!policy.nativeActiveObserved);
        check(!policy.hostedNativeContextRecovery); // never-active contexts retain the existing safe route
        policy.Expire(30100);
        check(policy.stage==Stage::Expired&&policy.RecoveryPending(30100));
        check(policy.RecoveryRuntimeProof(30100,inactive.identity,true,false,inactive.observedDeath.actor,
            inactive.observedDeath.active,inactive.pamelaStoppedContext_,inactive.pamelaStoppedUseAt_));
        check(policy.RequestRecovery(30100));
        policy.recoveryReturned=true;
        check(policy.ConfirmRecovery(30101,true,true)&&policy.stage==Stage::Idle&&!inactive.bundleKill.accepted);
        testHostedAdoptionOwnsDriver=false;
    }
    {NativeContextHost accepted;auto& policy=accepted.bundleKill.policy;
        check(policy.PamelaUsed(accepted.identity,4,100,true,100,100)&&policy.PamelaAccept(accepted.identity,101));
        accepted.r_.hint=10;accepted.observedDeath={10,11,12,true};testHostedAdoptionOwnsDriver=true;
        check(!accepted.PamelaNativeContextPhase(102)&&policy.nativeActiveObserved&&
            !policy.pamelaNativeContextOnly&&!policy.hostedNativeContextRecovery);
        testHostedAdoptionOwnsDriver=false;
    }
    for(unsigned fault=0;fault<5;++fault){NativeContextHost bad;
        check(bad.bundleKill.policy.PamelaUsed(bad.identity,4,100,true,100,100));bad.r_.hint=10;bad.observedDeath={10,11,12,false};
        switch(fault){case 0:bad.scope=false;break;case 1:bad.valid=false;break;case 2:++bad.identity.serial;break;
            case 3:bad.observedDeath={};break;case 4:bad.r_.readable=false;break;}
        bad.PamelaNativeContextPhase(101);check(!bad.installs&&!bad.stops&&!bad.bundleKill.accepted);
    }
    {NativeContextHost retry;retry.bundleKill.policy.PamelaUsed(retry.identity,4,100,true,100,100);
        retry.r_.hint=10;retry.observedDeath={10,11,12,false};retry.valid=false;
        check(!retry.PamelaNativeContextPhase(101)&&retry.scopeCalls==1);
        for(unsigned at=102;at<351;++at)check(!retry.PamelaNativeContextPhase(at)&&retry.scopeCalls==1);
        retry.valid=true;check(retry.PamelaNativeContextPhase(351)&&retry.installs==1&&retry.stops==1&&retry.scopeCalls==2);
        check(retry.PamelaNativeContextPhase(352)&&retry.scopeCalls==3); // successful proof never cached
    }
    {NativeContextHost retry;retry.bundleKill.policy.PamelaUsed(retry.identity,4,100,true,100,100);
        retry.r_.hint=10;retry.observedDeath={10,11,12,false};retry.installOK=false;
        check(retry.PamelaNativeContextPhase(101)&&retry.scopeCalls==1);
        check(!retry.PamelaNativeContextPhase(350)&&retry.scopeCalls==1);
        retry.installOK=true;check(retry.PamelaNativeContextPhase(351)&&retry.installs==1&&retry.stops==1);
    }
    {NativeContextHost concurrent;concurrent.bundleKill.policy.PamelaUsed(concurrent.identity,4,100,true,100,100);
        concurrent.r_.hint=10;concurrent.observedDeath={10,11,12,false};
        for(unsigned frame=0;frame<32;++frame){
            if(cfgpamela::ContextTurn(true,true,true,true,true))concurrent.PamelaNativeContextPhase(101+frame);
            check(concurrent.scopeCalls==frame+1); // RB/spectator/independent never veto validation
        }
    }
    for(unsigned mask=0;mask<32;++mask){const bool opportunity=(mask&1)!=0,camera=(mask&2)!=0,
        independent=(mask&4)!=0,mirror=(mask&8)!=0,input=(mask&16)!=0;
        check(cfgpamela::ContextTurn(opportunity,camera,independent,mirror,input)==opportunity);}
    std::printf("KP0 actual native-context member + female universal scoped eligibility + J1a bounded retries/turn sharing: %u checks PASS; native boundaries supplied.\n",checks);
}
