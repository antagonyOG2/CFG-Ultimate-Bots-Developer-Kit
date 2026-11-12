#pragma once
#include "../Core/KillPolicy.hpp"
#include "../Core/KillHitDiagnostic.hpp"
#include "../Core/PamelaFinisherPolicy.hpp"
#include "../RuntimeProbe/KillBindings.generated.hpp"
#include "KillNativeResource.hpp"
#include "HeldWeaponObservation.hpp"
#include "MeleeContactObservation.hpp"
namespace cfgkill {
struct Context {std::uintptr_t actor=0,interactable=0,killComponent=0;bool active=false;};
enum class Step : unsigned { Idle, Waiting, Unavailable, StanceDispatched, ContextReady, InteractionDispatched, NativeActive, Uncertain };
enum class Preparation : unsigned { Pending, Ready, Unavailable };
enum class Failure : unsigned { None, StancePreparation, InteractionPreparation, Scope,
    Context, ContextChanged, FinisherWeapon, InteractionDispatch, InteractionMarker, StanceDispatch };
namespace detail {
template<class H>auto PreparePamelaContext(H& h,const Context& c,int)->decltype(h.PreparePamelaContext(c)){return h.PreparePamelaContext(c);}
template<class H>Preparation PreparePamelaContext(H&,const Context&,long){return Preparation::Ready;}
template<class H>auto PamelaFinisher(H& h,std::uintptr_t& pawn,std::uintptr_t& weapon,Weapon& kind,int)
    ->decltype(h.PamelaFinisher(pawn,weapon,kind)){return h.PamelaFinisher(pawn,weapon,kind);}
template<class H>bool PamelaFinisher(H&,std::uintptr_t&,std::uintptr_t&,Weapon&,long){return false;}
template<class Host>auto ArmFinishingStrike(Host& h,const Context& c,std::uintptr_t pawn,int)
    ->decltype(h.ArmFinishingStrike(c,pawn)){return h.ArmFinishingStrike(c,pawn);}
template<class Host>bool ArmFinishingStrike(Host&,const Context&,std::uintptr_t,long){return true;}
template<class Host>auto RequiresFinishingStrike(Host& h,int)->decltype(h.RequiresFinishingStrike()){return h.RequiresFinishingStrike();}
template<class Host>bool RequiresFinishingStrike(Host&,long){return false;}
template<class Host> auto PrepareStance(Host& h,int) -> decltype(h.PrepareStance()){return h.PrepareStance();}
template<class Host> Preparation PrepareStance(Host&,long){return Preparation::Ready;}
template<class Host> auto PrepareInteraction(Host& h,std::uintptr_t actor,std::uintptr_t pawn,int)
    -> decltype(h.PrepareInteraction(actor,pawn)){return h.PrepareInteraction(actor,pawn);}
template<class Host> Preparation PrepareInteraction(Host&,std::uintptr_t,std::uintptr_t,long){return Preparation::Ready;}
template<class Host,class Mark> auto DispatchStance(Host& h,Mark mark,int)
    -> decltype(h.DispatchStance(mark)){return h.DispatchStance(mark);}
template<class Host,class Mark> bool DispatchStance(Host& h,Mark mark,long){
    bool stunned=false;return h.InStun(stunned)&&(stunned||h.BridgeStun())&&mark()&&h.EnterStance();
}
template<class Host,class Mark> auto FinalInteraction(Host& h,const Context& c,std::uintptr_t pawn,Mark mark,int)
    -> decltype(h.FinalInteraction(c,pawn,mark)){return h.FinalInteraction(c,pawn,mark);}
template<class Host,class Mark> bool FinalInteraction(Host& h,const Context& c,std::uintptr_t pawn,Mark mark,long){
    return mark()&&h.FinalInteraction(c,pawn);
}
}
// Host contract (all methods on captured game thread, within shared read/call budgets):
// Scope(Identity&): retained positive offline world + exact primary Jason serial;
// KnownLiving(pawn), DamageCounselor(controller,pawn&): bounded tracked roster;
// MaskOff(bool&), InStun(bool&): strict 0/1 typed CFG properties;
// CurrentWeapon(pawn,weapon&,Weapon&): serial/class/owner validated held item,
//   exact authored CounselorTwoHandedAxe_C or CounselorMachete_C class ancestry;
// DamageTypeMatches(weapon,damageType): event damage type object's exact class
//   equals a validated nonnull class at native weapon+5D8 or+5E0, with class/CDO
//   and object registry validation. These are fields of native WeaponStats.
// DistanceSquared(pawn,float&): finite live native positions;
// DamageEvent(function): exact Actor ReceiveAnyDamage UFunction and 32-byte frame
//   (or a separately verified override with the same parent/frame); no name-only gate;
// ReadDamage(parameters,DamagePacket&): bounded reader, never retain event storage;
// DeathContext(Context&): true with zero actor means verified absent. Nonzero
//   requires primary.JasonKillObject, its exact JasonKillClass, SCContextKillActor,
//   registry serials, same world, owned typed siblings and Victim null or primary.
// BridgeStun(): revalidate scope + mask + accepted finisher/weapon; write exact
//   bInStun native byte only, full setter proof and readback; never write health/death.
// EnterStance(): FeatureFunction(EnterKillStance), owner/frame, executor pointer,
//   COMPLETE ExecutorCode + ImplementationCode; revalidate scope; Invoke null packet.
// Optional PrepareStance(): Preparation. One cold bounded phase per call, no
//   gameplay side effects. Called before other Tick reads while a stance is pending.
//   Pending yields this tick; Unavailable closes this accepted-hit attempt. A host
//   without the method must fit its entire validated dispatch in its read budget.
// Optional DispatchStance(mark): fresh scope/owner/frame/slots/code and budget
//   proof BEFORE the stun bridge; mark immediately before native Invoke. False
//   is terminal uncertainty, including an ambiguous bridge write. No hot retry.
// Optional PrepareInteraction(actor,pawn): one bounded phase, Pending yields.
// Optional FinalInteraction(Context,pawn,mark): all fresh eligibility checks
//   first, then mark immediately before Invoke. A false BEFORE mark is a
//   side-effect-free preflight deferral; after mark it is terminal uncertainty.
// FinalInteraction(Context,pawn): native interaction of the public InteractComponent
//   with original weapon/range/living eligibility. No unconditional kill flags.
//   Return false when unsupported. Universal Hunter override is NOT provided here.
// Stance/interaction returns mean dispatch only. Native active context is not death.
class Runtime {
public:
    Policy policy{};
    unsigned accepted=0,stanceDispatches=0,interactionDispatches=0,unavailable=0,interactionDeferrals=0;
    Failure failure=Failure::None;
    std::uint64_t pamelaHealthAt=0;unsigned pamelaFinisherCursor=0;
    PamelaStockApproval pamelaStock;
    template<class H>bool ObservePamelaEvent(H& h,std::uintptr_t receiver,std::uintptr_t function,std::uintptr_t parameters,std::uint64_t now,bool post){
        if(!post||!policy.pamela||policy.stage!=Stage::AwaitHit||receiver!=policy.identity.jason)return false;
        cfgkilldiag::Hit observed;observed.function=function;
        Identity id{};bool acceptedHit=false;
        if(!h.Scope(id)||!(id==policy.identity))observed.clause=1;
        else if(!(observed.event=h.PamelaDamageEvent(function)))observed.clause=2;
        else {acceptedHit=policy.PamelaAccept(id,now);observed.clause=acceptedHit?0u:13u;
            // Diagnostic packet decode only for the separately proved AnyDamage
            // layout. Point/Radial acceptance never casts their packet as Any.
            if(h.DamageEvent(function)){DamagePacket packet{};if(h.ReadDamage(parameters,packet)){
                observed.packet=true;observed.instigator=packet.instigatedBy;observed.causer=packet.damageCauser;observed.damageType=packet.damageType;}}}
        if(acceptedHit)++accepted;
        if(observed.event)cfgkilldiag::Observe(h,observed,0);
        return acceptedHit;
    }
    template<class H>bool ObservePamelaHealth(H& h,std::uint64_t now){
        if(!policy.pamela||policy.stage!=Stage::AwaitHit)return false;
        Identity id{};float health=0;
        if(!h.Scope(id)||!(id==policy.identity)||!h.PamelaHealthValue(health)||!policy.PamelaHealth(id,health,now))return false;
        ++accepted;cfgkilldiag::Hit hit;hit.clause=14;cfgkilldiag::Observe(h,hit,0);return true;
    }
    template<class Host> bool SweaterUsed(Host& h,std::uintptr_t wearer,std::uint64_t now){
        Identity id{};bool maskOff=false;
        const bool armed=h.Scope(id)&&h.KnownLiving(wearer)&&h.MaskOff(maskOff)&&policy.SweaterUsed(id,wearer,now,maskOff);
        if(armed)failure=Failure::None;return armed;
    }
    template<class Host> bool ObserveDamage(Host& h,std::uintptr_t receiver,std::uintptr_t function,
                                          std::uintptr_t parameters,std::uint64_t now,bool postOriginal){
        const bool finishing=policy.stage==Stage::NativeContext&&detail::RequiresFinishingStrike(h,0);
        if(!postOriginal||(policy.stage!=Stage::AwaitHit&&!finishing)||receiver!=policy.identity.jason)return false;
        cfgkilldiag::Hit observed;observed.function=function;
        const auto result=[&](unsigned clause,bool accepted=false){observed.clause=clause;cfgkilldiag::Observe(h,observed,0);return accepted;};
        Identity id{};DamagePacket packet{};Hit hit{};
        if(!h.Scope(id)||!(id==policy.identity))return result(1);
        observed.event=h.DamageEvent(function);if(!observed.event)return result(2);
        if(!h.ReadDamage(parameters,packet))return result(3);
        observed.instigator=packet.instigatedBy;observed.causer=packet.damageCauser;observed.damageType=packet.damageType;
        observed.packet=std::isfinite(packet.damage)&&packet.damage>0&&packet.instigatedBy&&packet.damageCauser;
        if(!observed.packet)return result(4);
        if(!h.DamageCounselor(packet.instigatedBy,hit.counselor))return result(5);
        if(packet.damageCauser!=hit.counselor)return result(6);
        if(!h.KnownLiving(hit.counselor))return result(7);
        const bool weapon=h.CurrentWeapon(hit.counselor,hit.weapon,hit.kind);
        observed.weapon=hit.weapon;observed.kind=hit.kind;if(!weapon)return result(8);
        if(!h.DamageTypeMatches(hit.weapon,packet.damageType))return result(9);
        if(!h.MaskOff(hit.maskOff))return result(10);observed.mask=hit.maskOff;
        if(!h.DistanceSquared(hit.counselor,hit.distanceSquared))return result(11);
        observed.squared=hit.distanceSquared;observed.range=std::isfinite(hit.distanceSquared)&&hit.distanceSquared>=0;
        hit.identity=id;hit.controller=packet.instigatedBy;hit.damage=packet.damage;
        // CFG native counselor hit passes this counselor as DamageCauser,
        // not the weapon actor (proved AttackTrace -> virtual E00 -> 3F406B).
        hit.nativePostNotification=hit.trackedLivingCounselor=hit.causerIsCounselor=true;
        if(finishing){Context c{};
            if(!h.DeathContext(c)||c.actor!=policy.context||c.active||!policy.AcceptFinishingStrike(hit,now))return result(12);
        }else if(!policy.Accept(hit,now))return result(13);
        ++accepted;return result(0,true);
    }
#include "PamelaRuntime.hpp"
    template<class Host> Step Tick(Host& h,std::uint64_t now){
        policy.Expire(now);
        if(!policy.Exclusive(now))return Step::Idle;
        if(policy.PamelaAccepted())return TickPamela(h,now);
        if(policy.stage==Stage::AwaitContext&&!policy.stanceAttempted){
            const auto prepared=detail::PrepareStance(h,0);
            if(prepared==Preparation::Pending)return Step::Waiting;
            if(prepared!=Preparation::Ready){failure=Failure::StancePreparation;++unavailable;policy.stage=Stage::Uncertain;return Step::Unavailable;}
        }
        bool interactionPrepared=false;
        if(policy.stage==Stage::NativeContext&&!policy.interactionAttempted&&
           (!detail::RequiresFinishingStrike(h,0)||policy.finishingStrike)){
            const auto prepared=detail::PrepareInteraction(h,policy.context,policy.finisher,0);
            if(prepared==Preparation::Pending)return Step::Waiting;
            if(prepared!=Preparation::Ready){failure=Failure::InteractionPreparation;++unavailable;policy.stage=Stage::Uncertain;return Step::Unavailable;}
            interactionPrepared=true;
        }
        Identity id{};bool maskOff=false;std::uintptr_t weapon=0;Weapon kind=Weapon::Other;
        if(!h.Scope(id)||!(id==policy.identity)){failure=Failure::Scope;policy.stage=Stage::Uncertain;return Step::Uncertain;}
        Context context{};
        if(!h.DeathContext(context)){failure=Failure::Context;++unavailable;policy.stage=Stage::Uncertain;return Step::Unavailable;}
        if(context.actor){
            if(!policy.ObserveContext(context.actor,now,context.active)){failure=Failure::ContextChanged;return Step::Uncertain;}
            if(context.active)return Step::NativeActive;
            if(policy.interactionAttempted)return Step::ContextReady;
            // Proximity alone is not a second attack. Require a NEW native
            // melee notification after the kneeling context has been observed.
            if(detail::RequiresFinishingStrike(h,0)&&!policy.finishingStrike){
                if(!policy.meleeArmed){
                    policy.meleeArmed=true; // mark before even a partial native-field mutation; never replay ambiguity
                    if(!detail::ArmFinishingStrike(h,context,policy.finisher,0)){
                        failure=Failure::InteractionPreparation;++unavailable;policy.stage=Stage::Uncertain;return Step::Unavailable;
                    }
                }
                return Step::ContextReady; // arm ordinary melee, not a synthetic A/proximity kill
            }
            if(!interactionPrepared){
                const auto prepared=detail::PrepareInteraction(h,context.actor,policy.finisher,0);
                if(prepared==Preparation::Pending)return Step::Waiting;
                if(prepared!=Preparation::Ready){failure=Failure::InteractionPreparation;++unavailable;policy.stage=Stage::Uncertain;return Step::Unavailable;}
            }
            if(!h.KnownLiving(policy.finisher)||!h.CurrentWeapon(policy.finisher,weapon,kind)||weapon!=policy.weapon||
               (!policy.PamelaAccepted()&&kind!=Weapon::Axe&&kind!=Weapon::Machete)){failure=Failure::FinisherWeapon;++unavailable;policy.stage=Stage::Uncertain;return Step::Unavailable;}
            // A staged host marks only at its actual native call boundary.
            const auto mark=[&](){return policy.RequestInteraction(now);};
            if(!detail::FinalInteraction(h,context,policy.finisher,mark,0)){
                failure=Failure::InteractionDispatch;++unavailable;
                if(!policy.interactionAttempted){++interactionDeferrals;return Step::Waiting;}
                policy.stage=Stage::Uncertain;return Step::Unavailable;
            }
            if(!policy.interactionAttempted){failure=Failure::InteractionMarker;policy.stage=Stage::Uncertain;return Step::Uncertain;}
            failure=Failure::None;++interactionDispatches;return Step::InteractionDispatched;
        }
        if(policy.stage!=Stage::AwaitContext||policy.stanceAttempted)return Step::Waiting;
        if(!h.MaskOff(maskOff)||!maskOff||!h.KnownLiving(policy.finisher)||
           !h.CurrentWeapon(policy.finisher,weapon,kind)||weapon!=policy.weapon||
           (!policy.PamelaAccepted()&&kind!=Weapon::Axe&&kind!=Weapon::Machete)){failure=Failure::FinisherWeapon;++unavailable;policy.stage=Stage::Uncertain;return Step::Unavailable;}
        // Same base-mod bridge, only after a native accepted melee notification.
        // Never bridge on activation, numeric HP change, generic radial damage,
        // proximity, animation or a weapon merely being carried nearby.
        const auto mark=[&](){return policy.RequestStance(now);};
        if(!detail::DispatchStance(h,mark,0)||!policy.stanceAttempted){failure=Failure::StanceDispatch;policy.stage=Stage::Uncertain;return Step::Uncertain;}
        ++stanceDispatches;return Step::StanceDispatched;
    }
    template<class Host>bool ObserveMeleeContact(Host& h,const cfgkillcontact::Proof& proof,std::uint64_t now){
        // Native blocking contact is distinct from damage: a kneeling Jason
        // may reject damage. Never turn a swing/proximity/old trace into a hit.
        if(!proof.armed||policy.stage!=Stage::NativeContext||policy.interactionAttempted||policy.finishingStrike||
           policy.nativeActiveObserved||now<=policy.acceptedAt||now>=policy.until||
           !(proof.identity==policy.identity)||proof.context!=policy.context||proof.usedAt!=policy.usedAt||
           proof.acceptedAt!=policy.acceptedAt||proof.source.object!=policy.finisher||proof.weapon!=policy.weapon||
           !cfgkillcontact::AddedTarget(proof.before,proof.after,policy.identity.jason,policy.context))return false;
        Identity id{};Context c{};bool off=false;std::uintptr_t weapon=0;Weapon kind=Weapon::Other;float squared=0;
        if(!h.Scope(id)||!(id==policy.identity)||!h.KnownLiving(policy.finisher)||
           !h.CurrentWeapon(policy.finisher,weapon,kind)||weapon!=policy.weapon||
           (kind!=Weapon::Axe&&kind!=Weapon::Machete)||!h.MaskOff(off)||!off||
           !h.DistanceSquared(policy.finisher,squared)||!std::isfinite(squared)||squared<0||squared>200.f*200.f||
           !h.DeathContext(c)||c.actor!=policy.context||c.active||!h.MeleeContactCurrent(proof))return false;
        policy.finishingStrike=true;++accepted;return true;
        // Existing staged FinalInteraction owns the next callback. No native
        // call, synthetic damage, health/death write or timing change here.
    }
};
}
