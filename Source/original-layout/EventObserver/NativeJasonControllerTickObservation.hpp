#pragma once
#include "JasonInstanceObservation.hpp"
#include "PrimaryContextObservation.hpp"
#include "KillNativeResource.hpp"
#include "HostedAdoption.hpp"
#include "../RuntimeProbe/Compatibility/CfgJasonHunting35.generated.hpp"
namespace cfgjasontick {
// Values copied from the already-owned primary or F1 birth, never adopted from
// an incoming tick's raw pointers. This snapshot is an observation, NOT a root.
struct Owner {
    cfgobserver::Plan source{};
    cfgjason::NewActorEvidence actor{};
    std::uintptr_t pawnClass=0,defaults=0,killerBase=0,killerAI=0,pathClass=0;
    cfghosted::adoption::Receipt adoption{}; // H1: exact receipt for a mod-adopted human Jason; invalid for every offline owner
};
template<class R>bool ObserveOwner(R& r,const Owner& owner,std::uint32_t thread){
    const auto& p=owner.source;
    if(!p.module||!thread||thread!=p.threadId||!owner.actor.pawn.HasAssignedSerial()||
       !owner.actor.killerController.HasAssignedSerial()||!owner.pawnClass||!owner.defaults||
       !owner.killerBase||!owner.killerAI||!owner.pathClass)return false;
    cfgobserver::Plan fresh{};
    if(!cfgprimary::LookupVetoClearForObservation(r,p.module)||!cfgprimary::RetainedWorld(r,p,fresh))return false;
    // RetainedOfflineWorld checks stable identity. Native entry additionally
    // needs the existing dispatcher-invalid slot-flag veto on both owners.
    cfgprobe::lifetime::SlotObservation world{},mode{};
    if(!cfgspawn::ObjectSlot(r,fresh.objectArray,p.world,world)||!(world==p.objects[0])||
       !cfgspawn::ObjectSlot(r,fresh.objectArray,p.mode,mode)||!(mode==p.objects[1]))return false;
    cfgprobe::lifetime::SlotObservation pawnLevel{},controllerLevel{};std::uintptr_t pawnLevelType=0,controllerLevelType=0;
    if(!cfgkill::ActorWorldCurrent(r,fresh.objectArray,p.module,owner.actor.pawn.object,p.world,pawnLevel,pawnLevelType)||
       !cfgkill::ActorWorldCurrent(r,fresh.objectArray,p.module,owner.actor.killerController.object,p.world,controllerLevel,controllerLevelType))return false;
    const auto& binding=cfgcompat::cfg35::ai::Classes[3]; // named SCKillerAIController
    return cfgobserver::Same(r,p.module+binding.reflection,owner.killerAI)&&
        cfgobserver::Same(r,p.module+binding.constructor,owner.killerAI)&&
        (owner.adoption.valid?cfgjason::LinkedAdoptedKiller(r,fresh.objectArray,owner.actor,owner.adoption,p,p.pawn,owner.pawnClass,owner.defaults,
            owner.killerBase,owner.killerAI,owner.pathClass,p.controller):
         cfgjason::LinkedKiller(r,fresh.objectArray,owner.actor,p,p.pawn,owner.pawnClass,owner.defaults,
            owner.killerBase,owner.killerAI,owner.pathClass,p.controller))&&
        cfgprimary::LookupVetoClearForObservation(r,p.module);
}
}
