#pragma once
#include <cstdint>
#include "J24FeatureFlags.hpp"
namespace cfgsweater {
enum class KillArmResult {Accepted,Rejected,Deferred};
namespace detail {
template<class H,class K,class I>auto ArmKill(H& h,K& k,const I& id,std::uintptr_t wearer,std::uint64_t used,int)
    ->decltype(h.ArmSweaterKill(k,id,wearer,used)){return h.ArmSweaterKill(k,id,wearer,used);}
template<class H,class K,class I>KillArmResult ArmKill(H&,K& k,const I& id,std::uintptr_t wearer,std::uint64_t used,long){
    return k.policy.SweaterUsed(id,wearer,used,true)?KillArmResult::Accepted:KillArmResult::Rejected;}
}
// Donor activation: current wearer's HasSweater1->0; HasBeenUsed0->1
// is a secondary witness. Re-publish after the native10s montage +1s,
// independently of the30s assisted-hit opportunity.
// Never replay input or its montage. Keep the native cinematic lease unchanged.
// One positively identified offline counselor/ability. No emote or input flags.
enum class Stage:unsigned { Dormant,Discovering,Owned,Waiting,Refused,HudPending };
enum class Action:unsigned { None,Discover,Publish,Observe,Rearm,PublishHud };
enum class ArmFailure:unsigned { None,Scope,Living,MaskRead,Masked,Policy,Expired };
constexpr unsigned PublicationReadReserve=512;
inline bool PublicationBudget(unsigned reads,unsigned cap,unsigned calls,unsigned callCap){
    return cap>=PublicationReadReserve&&reads<=cap-PublicationReadReserve&&calls<callCap;
}
struct Policy {
    Stage stage=Stage::Dormant;
    std::uint64_t next=0,deadline=0,rearmAt=0,lastUse=0;
    bool clearObserved=false,publicationStarted=false;
    bool ownershipObserved=false,ownershipHas=false,nativeOwnershipEarned=false;
    bool hostedOwnerHandoffUsed=false;
    bool healthOpportunityOwed=false;
    bool armPending=false;
    std::uint64_t armUntil=0,armNext=0;
    unsigned readDeferrals=0,armAttempts=0,arms=0,armDeferrals=0,armRejections=0;
    ArmFailure armFailure=ArmFailure::None;
    unsigned lastFailure=0,hudAttempts=0,hudReturns=0;
    bool pendingRearm=false,searchExpired=false;
    bool repairingOwned=false;unsigned ownedRepairs=0,rebinds=0;
    std::uint64_t hudUntil=0,hudQueuedAt=0,hudFirstServiceAt=0;
    unsigned hudProofFailure=0;
    bool CanPoll()const{return stage!=Stage::Refused||searchExpired;}
    bool Due(std::uint64_t now){if(!CanPoll()||now<next)return false;next=now+500;return true;}
    bool AdoptNativeOwned(){
        if(publicationStarted||(stage==Stage::Refused&&!searchExpired))return false;
        if(stage!=Stage::Dormant&&stage!=Stage::Discovering&&!searchExpired)return false;
        stage=Stage::Dormant;searchExpired=false;nativeOwnershipEarned=true;return true;
    }
    Action Decide(std::uint64_t now,bool trigger,bool nativeOwned,bool bound){
        if(stage==Stage::Refused)return Action::None;
        if(stage==Stage::HudPending){if(!hudFirstServiceAt)hudFirstServiceAt=now;
            if(now>=hudUntil){lastFailure=30;Refuse();return Action::None;}return Action::PublishHud;}
        if(stage==Stage::Dormant){
            if(nativeOwned&&bound){stage=Stage::Discovering;deadline=now+60000;return Action::Publish;}
            if(!trigger)return Action::None;
            stage=Stage::Discovering;deadline=now+60000;return Action::Discover;
        }
        if(stage==Stage::Discovering){if(bound)return Action::Publish;
            if(now>=deadline){Refuse();searchExpired=true;return Action::None;}return Action::Discover;}
        if(stage==Stage::Waiting)return now>=rearmAt?Action::Rearm:Action::None;
        return bound?Action::Observe:Action::None;
    }
    bool CommitUse(std::uint64_t now){
        const bool earnedPending=nativeOwnershipEarned&&(stage==Stage::Discovering||stage==Stage::HudPending);
        if((stage!=Stage::Owned&&!earnedPending)||publicationStarted)return false;
        if(now>UINT64_MAX-30000)return false;
        lastUse=now;rearmAt=now+11000;stage=Stage::Waiting;clearObserved=false;healthOpportunityOwed=false;
        armPending=true;armUntil=now+5000;armNext=now+1;armFailure=ArmFailure::None;return true;
    }
    bool ObserveOwnership(std::uint64_t now,unsigned has){
        if(has>1||publicationStarted)return false;
        const bool falling=ownershipObserved&&ownershipHas&&!has;
        ownershipObserved=true;ownershipHas=has!=0;
        return falling&&CommitUse(now);
    }
    bool ObserveUsed(std::uint64_t now,unsigned used){
        if(stage!=Stage::Owned||used>1)return false;
        if(!used){clearObserved=true;return false;}
        return clearObserved&&CommitUse(now); // A late1 alone is not a use.
    }
    // Use observation has already spent a phase's read/native-call budget.
    // Admission gets a later phase and retains the original use timestamp.
    // Missing observations may retry for five seconds; a proved mask-on or a
    // consumed/uncertain kill transaction is a rejection, never a bypass.
    template<class Host,class Kill> bool ServiceArm(Host& host,Kill& kill,std::uintptr_t wearer,std::uint64_t now){
        if(!armPending)return false;
        if(now<lastUse||now>=armUntil){armPending=false;armFailure=ArmFailure::Expired;++armRejections;return true;}
        if(now<armNext)return false;
        armNext=now+250;
        ++armAttempts;auto identity=kill.policy.identity;identity={};bool maskOff=false;
        if(!host.Scope(identity))armFailure=ArmFailure::Scope;
        else if(!host.KnownLiving(wearer))armFailure=ArmFailure::Living;
        else if(!host.MaskOff(maskOff))armFailure=ArmFailure::MaskRead;
        else if(!maskOff){armFailure=ArmFailure::Masked;armPending=false;++armRejections;return true;}
        else {
            armPending=false;
            const auto armed=detail::ArmKill(host,kill,identity,wearer,lastUse,0);
            if(armed==KillArmResult::Deferred){armPending=true;armFailure=ArmFailure::Scope;++armDeferrals;return true;}
            if(armed==KillArmResult::Accepted){++arms;armFailure=ArmFailure::None;kill.failure={};}
            else {++armRejections;armFailure=ArmFailure::Policy;}
            return true;
        }
        ++armDeferrals;return true;
    }
    // An unavailable observation/preflight performs no mutation. Keep the
    // captured source and repeat its complete ownership proof on a later phase.
    // Once the first write is attempted, failure remains terminal uncertainty.
    void Unavailable(){if(publicationStarted)Refuse();else ++readDeferrals;}
    void BeginPublish(){publicationStarted=true;}
    void QueueHud(std::uint64_t now,bool rearm){if(now>UINT64_MAX-5000){lastFailure=30;Refuse();return;}stage=Stage::HudPending;nativeOwnershipEarned=true;publicationStarted=false;pendingRearm=rearm;hudUntil=now+5000;hudQueuedAt=now;hudFirstServiceAt=0;hudProofFailure=0;}
    void ConfirmPublish(){stage=Stage::Owned;clearObserved=true;publicationStarted=false;repairingOwned=false;
        ownershipObserved=ownershipHas=nativeOwnershipEarned=true;healthOpportunityOwed=false;} // publication proved HasSweater=1
    bool QueueRetiredOwned(std::uint64_t now){
        if(stage!=Stage::Owned||publicationStarted||ownedRepairs>=3||now>UINT64_MAX-60000)return false;
        ++ownedRepairs;stage=Stage::Waiting;repairingOwned=true;rearmAt=now;deadline=now+60000;
        return true; // NOT a native use edge and never arms a kill transaction
    }
    bool RebindNative(std::uint64_t now,unsigned used){
        if(used>1||stage==Stage::Refused||publicationStarted)return false;
        if(stage==Stage::Waiting){
            // A new positive native owner does not reset the 11-second clock,
            // create a second use edge or replay an already entered constructor.
            // The newly current native ability supersedes the planned repair.
            repairingOwned=false;clearObserved=used==0;++rebinds;return true;
        }
        if(!ReplaceOwned(now,used))return false;
        ++rebinds;return true;
    }
    bool PreserveHostedOwnerRecharge(const Policy& previous,std::uint64_t now){
        if(hostedOwnerHandoffUsed||previous.hostedOwnerHandoffUsed||!previous.nativeOwnershipEarned||
           !previous.lastUse||previous.stage!=Stage::Waiting||!previous.rearmAt||
           previous.rearmAt<=previous.lastUse||now<previous.lastUse)return false;
        stage=Stage::Waiting;lastUse=previous.lastUse;rearmAt=previous.rearmAt;
        nativeOwnershipEarned=true;hostedOwnerHandoffUsed=true;return true;
    }
    void Refuse(){stage=Stage::Refused;publicationStarted=false;armPending=false;searchExpired=false;}
    bool KillWindow(std::uint64_t now)const{return lastUse&&now>=lastUse&&now-lastUse<=10000;}
    bool ReplaceOwned(std::uint64_t now,unsigned used){
        if(stage==Stage::Refused||publicationStarted||used>1)return false;
        const bool waiting=stage==Stage::Waiting;
        // A real pickup can replace the mask-borrowed native ability. Never
        // reset the witnessed use/kill window or revive the destroyed source.
        if(!used){QueueHud(now,waiting);return stage==Stage::HudPending;}
        if(waiting&&lastUse){clearObserved=false;return true;}
        stage=Stage::Owned;clearObserved=false;return true; // no invented 0->1 use
    }
};
enum class Continuation {None,Hud,Fresh,Class};
// This chooser is used by the production callback BEFORE optional class/helper
// work. A successful publication owes its UI handoff, not another class survey.
inline Continuation ChooseContinuation(const Policy& policy,std::uint64_t now,
    bool convergence,bool fresh,bool capture){
    if(policy.stage==Stage::HudPending&&now>=policy.next)return Continuation::Hud;
    if(!convergence)return Continuation::None;
    return fresh?Continuation::Fresh:capture?Continuation::Class:Continuation::None;
}
// Positive weak identity is retained from the earned native ability; native
// removal clears both owning pointers. Never accept a borrowed/non-null owner
// or resurrect an unassigned serial after its strong source link disappeared.
inline bool AcceptOwnedDuringDiscovery(Stage stage,bool searchExpired=false){return stage==Stage::Dormant||stage==Stage::Discovering||(stage==Stage::Refused&&searchExpired);}
inline bool DetachedEarnedAbility(bool positiveSerial,bool observedUse,std::uintptr_t pawnSlot,std::uintptr_t abilityOwner){
    return positiveSerial&&observedUse&&!pawnSlot&&!abilityOwner;
}
inline bool AssignedOwnedSlot(std::uintptr_t oldObject,int oldIndex,int oldSerial,
    std::uintptr_t freshObject,int freshIndex,int freshSerial,bool reciprocalPositiveOwner){
    return reciprocalPositiveOwner&&oldObject&&oldObject==freshObject&&oldIndex>=0&&oldIndex==freshIndex&&oldSerial==0&&freshSerial>0;
}
}
