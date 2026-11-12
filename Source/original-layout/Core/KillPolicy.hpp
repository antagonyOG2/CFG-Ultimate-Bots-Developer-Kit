#pragma once
#include <cmath>
#include <cstddef>
#include <cstdint>
namespace cfgkill {
// Short inactive opportunities; a REAL dispatched/active native cinematic
// retains the original60s completion protection, never a rolling6s reset.
constexpr std::uint64_t HitWindowMs=10000,ContextWaitMs=6000,NativeWindowMs=60000;
constexpr std::uint64_t PamelaWindowMs=30000;
enum class Stage : unsigned { Idle, AwaitHit, AwaitContext, NativeContext, NativeActive, Expired, Uncertain };
enum class Weapon : unsigned { Other, Axe, Machete };
struct Identity {
    std::uintptr_t world=0,jason=0;std::int32_t serial=0;
    bool Valid()const{return world&&jason&&serial>0;}
    bool operator==(const Identity& x)const{return world==x.world&&jason==x.jason&&serial==x.serial;}
};
struct DamagePacket {
    float damage=0;std::uint32_t padding=0;
    std::uintptr_t damageType=0,instigatedBy=0,damageCauser=0;
};
static_assert(sizeof(DamagePacket)==32&&offsetof(DamagePacket,damageType)==8&&
    offsetof(DamagePacket,instigatedBy)==16&&offsetof(DamagePacket,damageCauser)==24);
struct Hit {
    Identity identity{};std::uintptr_t counselor=0,controller=0,weapon=0;
    Weapon kind=Weapon::Other;float damage=0,distanceSquared=0;
    bool nativePostNotification=false,trackedLivingCounselor=false,maskOff=false,causerIsCounselor=false;
};
struct Policy {
    Identity identity{};Stage stage=Stage::Idle;
    std::uint64_t usedAt=0,until=0;std::uintptr_t wearer=0,finisher=0,weapon=0,context=0;
    std::uint64_t acceptedAt=0;bool finishingStrike=false;
    bool stanceAttempted=false,interactionAttempted=false,nativeActiveObserved=false,recoveryAttempted=false;
    bool recoveryReturned=false;
    bool meleeArmed=false;
    bool pamela=false,healthBaselineValid=false,pamelaNativeContextOnly=false;
    bool hostedNativeContextRecovery=false;float healthBaseline=0;
    std::uint64_t hostedNativeContextUsedAt=0;
    bool pamelaDispatchPending=false,pamelaDispatchReturned=false;
    std::uint64_t pamelaNextDispatch=0;
    void Clear(){*this={};}
    bool SweaterUsed(const Identity& id,std::uintptr_t counselor,std::uint64_t now,bool maskOff){
        if(!id.Valid()||!counselor||!maskOff||now>UINT64_MAX-NativeWindowMs)return false;
        // Called only for a newly observed native use edge, never for ownership
        // or recharge. A later real use refreshes an unconsumed hit window.
        // Once a hit/dispatch is accepted, further activations cannot replace it.
        if(identity==id&&(stage==Stage::AwaitContext||stage==Stage::NativeContext||stage==Stage::NativeActive||stage==Stage::Uncertain||
            (stage==Stage::AwaitHit&&now<=usedAt)))return false;
        Clear();identity=id;wearer=counselor;usedAt=now;until=now+HitWindowMs;stage=Stage::AwaitHit;return true;
    }
    bool Accept(const Hit& h,std::uint64_t now){
        if(stage!=Stage::AwaitHit||!(identity==h.identity)||now<usedAt||now>=until||
           !h.counselor||!h.controller||!h.weapon||!h.nativePostNotification||!h.trackedLivingCounselor||
           !h.maskOff||!h.causerIsCounselor||(h.kind!=Weapon::Axe&&h.kind!=Weapon::Machete)||
           !std::isfinite(h.damage)||h.damage<=0||!std::isfinite(h.distanceSquared)||h.distanceSquared<0||
           h.distanceSquared>200.f*200.f)return false;
        finisher=h.counselor;weapon=h.weapon;acceptedAt=now;stage=Stage::AwaitContext;until=now+ContextWaitMs;return true;
    }
    bool PamelaUsed(const Identity& id,std::uintptr_t counselor,std::uint64_t now,bool maskOff,float squared,float health){
        if(!std::isfinite(squared)||squared<0||squared>250.f*250.f||!std::isfinite(health)||
           now>UINT64_MAX-NativeWindowMs||!SweaterUsed(id,counselor,now,maskOff))return false;
        pamela=true;healthBaseline=health;healthBaselineValid=true;until=now+PamelaWindowMs;return true;
    }
    bool PamelaAccept(const Identity& id,std::uint64_t now){
        if(!pamela||stage!=Stage::AwaitHit||!(identity==id)||now<usedAt||now>=until||now>UINT64_MAX-NativeWindowMs)return false;
        acceptedAt=now;stage=Stage::AwaitContext;until=now+NativeWindowMs;
        finishingStrike=true;healthBaselineValid=false;return true;
    }
    bool PamelaHealth(const Identity& id,float health,std::uint64_t now){
        return healthBaselineValid&&std::isfinite(health)&&health+.05f<healthBaseline&&PamelaAccept(id,now);
    }
    bool PamelaAccepted()const{return pamela&&((finishingStrike&&acceptedAt)||pamelaNativeContextOnly);}
    // FJB TryCompleteSimplifiedPamelaTranceKill 12497: a real native
    // JasonDeath context wins independently of the mod's damage observation.
    // This route enables the player's ordinary interaction, never invents a hit.
    bool ObservePamelaNativeContext(const Identity& id,std::uintptr_t actor,std::uint64_t now,bool active){
        if(!pamela||!(id==identity)||!actor||now<usedAt||now>=until||now>UINT64_MAX-NativeWindowMs||
           (stage!=Stage::AwaitHit&&stage!=Stage::AwaitContext&&stage!=Stage::NativeContext&&stage!=Stage::NativeActive))return false;
        if(stage==Stage::AwaitHit){pamelaNativeContextOnly=true;stage=Stage::AwaitContext;}
        if(until<now+3000)until=now+PamelaWindowMs;
        if(!ObserveContext(actor,now,active))return false;
        if(active){interactionAttempted=true;pamelaDispatchPending=false;} // actual stock interaction, not synthetic dispatch
        return true;
    }
    bool AcceptFinishingStrike(const Hit& h,std::uint64_t now){
        if(stage!=Stage::NativeContext||!context||interactionAttempted||finishingStrike||
           now<=acceptedAt||now>=until||!(identity==h.identity)||!h.counselor||!h.controller||!h.weapon||
           !h.nativePostNotification||!h.trackedLivingCounselor||!h.maskOff||!h.causerIsCounselor||
           (h.kind!=Weapon::Axe&&h.kind!=Weapon::Machete)||!std::isfinite(h.damage)||h.damage<=0||
           !std::isfinite(h.distanceSquared)||h.distanceSquared<0||h.distanceSquared>200.f*200.f)return false;
        finisher=h.counselor;weapon=h.weapon;finishingStrike=true;return true;
    }
    bool RequestStance(std::uint64_t now){
        if(stage!=Stage::AwaitContext||stanceAttempted||now>=until)return false;
        stanceAttempted=true;return true; // mark before a potentially side-effecting native dispatch
    }
    bool ObserveContext(std::uintptr_t actor,std::uint64_t now,bool active){
        if(!actor||now>UINT64_MAX-NativeWindowMs||
           (stage!=Stage::AwaitContext&&stage!=Stage::NativeContext&&stage!=Stage::NativeActive))return false;
        if(context&&context!=actor){stage=Stage::Uncertain;return false;}
        if(!context){context=actor;const auto next=now+(active?NativeWindowMs:ContextWaitMs);
            if(!PamelaAccepted()||next>until)until=next;}
        else if(active&&!nativeActiveObserved)until=now+NativeWindowMs;
        nativeActiveObserved=nativeActiveObserved||active;
        stage=active?Stage::NativeActive:Stage::NativeContext;return true;
    }
    bool RequestInteraction(std::uint64_t now){
        if(stage!=Stage::NativeContext||interactionAttempted||now>=until)return false;
        interactionAttempted=true;until=now+NativeWindowMs;return true;
    }
    bool RequestPamelaInteraction(std::uint64_t now){
        if(!PamelaAccepted()||stage!=Stage::NativeContext||!context||!finisher||!weapon||
           interactionAttempted||pamelaDispatchPending||now<acceptedAt||now>=until||now<pamelaNextDispatch)return false;
        pamelaDispatchPending=true;pamelaDispatchReturned=false;
        pamelaNextDispatch=now>UINT64_MAX-1000?UINT64_MAX:now+1000;
        return true; // invocation only; native active observation consumes it
    }
    void Expire(std::uint64_t now){
        if(stage!=Stage::Idle&&stage!=Stage::Expired&&stage!=Stage::Uncertain&&now>=until)stage=Stage::Expired;
    }
    bool Exclusive(std::uint64_t now)const{
        return now<until&&(stage==Stage::AwaitContext||stage==Stage::NativeContext||stage==Stage::NativeActive);
    }
    bool InputExclusive(std::uint64_t now)const{
        // An unconsumed inactive kneeling context reserves Jason's AI, not
        // the recovered human's wheel for the entire60s finishing window.
        // Native activity is freshly checked by the wheel/mirror consumer.
        return Exclusive(now)&&(stage!=Stage::NativeContext||interactionAttempted||nativeActiveObserved||
            now<usedAt||now-usedAt<HitWindowMs);
    }
    bool RecoveryPending(std::uint64_t now)const{
        // A hosted Pamela-only native context can become active without our
        // dispatching the stock interaction. After its fixed timeout, let the
        // runtime's fresh death/context/stun proof decide whether it is safe
        // to recover; an active or dying native context still refuses there.
        const bool hostedObservedOnly=hostedNativeContextRecovery&&pamelaNativeContextOnly&&
            nativeActiveObserved&&!finishingStrike&&!pamelaDispatchPending&&!pamelaDispatchReturned&&
            hostedNativeContextUsedAt==usedAt&&stage==Stage::Expired;
        return (stanceAttempted||context||pamela)&&(!interactionAttempted||hostedObservedOnly)&&
            (!nativeActiveObserved||hostedObservedOnly)&&
            !(pamelaDispatchPending&&!pamelaDispatchReturned)&&(!recoveryAttempted||recoveryReturned)&&
            (stage==Stage::Expired||stage==Stage::Uncertain)&&now>=until&&now-until<=5000;
    }
    bool RequestRecovery(std::uint64_t now){
        if(recoveryAttempted||!RecoveryPending(now))return false;
        recoveryAttempted=true;return true; // no replay after a native dispatch ambiguity
    }
    bool ConfirmRecovery(std::uint64_t now,bool nativeInactive,bool unstunned){
        if(!RecoveryPending(now)||!recoveryAttempted||!nativeInactive||!unstunned)return false;
        Clear();return true; // a separate CURRENT native proof, never an ambiguous dispatch replay
    }
    bool HostedNativeContextRecoveryProof(std::uint64_t now,const Identity& observedIdentity,
        bool hostedOwner,bool dying,std::uintptr_t observedContext,bool contextActive,
        std::uintptr_t stoppedContext,std::uint64_t stoppedUseAt)const{
        return hostedNativeContextRecovery&&hostedOwner&&RecoveryPending(now)&&
            pamelaNativeContextOnly&&nativeActiveObserved&&!finishingStrike&&
            !pamelaDispatchPending&&!pamelaDispatchReturned&&identity==observedIdentity&&
            hostedNativeContextUsedAt==usedAt&&context&&context==observedContext&&!dying&&!contextActive&&
            stoppedContext==context&&stoppedUseAt==usedAt;
    }
    bool RecoveryRuntimeProof(std::uint64_t now,const Identity& observedIdentity,bool hostedOwner,
        bool dying,std::uintptr_t observedContext,bool contextActive,
        std::uintptr_t stoppedContext,std::uint64_t stoppedUseAt)const{
        if(hostedNativeContextRecovery&&nativeActiveObserved)
            return HostedNativeContextRecoveryProof(now,observedIdentity,hostedOwner,dying,
                observedContext,contextActive,stoppedContext,stoppedUseAt);
        return RecoveryPending(now)&&identity==observedIdentity&&!dying&&!contextActive&&
            (!observedContext||observedContext==context);
    }
};
}
