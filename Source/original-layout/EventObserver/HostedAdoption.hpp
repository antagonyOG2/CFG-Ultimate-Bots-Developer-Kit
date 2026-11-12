#pragma once
#include "JasonInstanceObservation.hpp"
#include "HostedRetainedWorld.hpp"

// H1 adoption of a human-owned (remote-selected) Jason pawn by a mod-spawned
// SCKillerAIController (mailbox #349 item 4). The donor never rewrites that
// pawn's AIControllerClass, so cfgjason::LinkedKiller's PawnAIClass==KillerAI
// check cannot hold for it. An exact, CURRENT AdoptionReceipt replaces ONLY that
// one check, for ONLY that pawn; every other LinkedKiller proof is repeated.
// A receipt is observation evidence and never call authority; GC retention,
// world admission (RetainedWorld) and the native tick witness remain required.
namespace cfghosted::adoption {
struct Receipt {
    bool valid=false;
    cfgprobe::lifetime::SlotObservation pawn{};        // Hunt CurrentKiller at takeover (the stock Jason pawn)
    cfgprobe::lifetime::SlotObservation controller{};  // mod-spawned SCKillerAIController
    std::uintptr_t controllerState=0;                  // the AI's PlayerState (strongly owned by the controller)
    std::uintptr_t displacedState=0;                   // the remote human's PlayerState (returned as counselor)
    std::uintptr_t world=0,mode=0;
    bool possessReturned=false;
    std::int32_t botsBefore=0,botsAfter=0,rosterBefore=0,rosterAfter=0;
    // The postconditions the donor transaction verified. Stored, not re-derived.
    bool Complete() const noexcept {
        return valid&&possessReturned&&botsAfter==botsBefore+1&&rosterAfter==rosterBefore+1&&
            pawn.HasAssignedSerial()&&controller.HasAssignedSerial()&&pawn.object!=controller.object&&
            cfgprobe::ObjectPointer(controllerState)&&cfgprobe::ObjectPointer(displacedState)&&
            controllerState!=displacedState&&cfgprobe::ObjectPointer(world)&&cfgprobe::ObjectPointer(mode);
    }
};
constexpr unsigned ControllerPawn=0x370,PawnController=0x3A0,ControllerState=0x388,PawnState=0x388,StateOwner=0x88;
// Current ownership linkage of the adoption (repeats on every native tick admission).
template<class R> bool Current(R& r,const cfgprobe::lifetime::ArrayHeader& array,const Receipt& e,const cfgobserver::Plan& plan){
    if(!e.Complete()||e.world!=plan.world||e.mode!=plan.mode||e.pawn.object==plan.pawn||e.controller.object==plan.controller)return false;
    return cfgjason::SameSlot(r,array,e.pawn)&&cfgjason::SameSlot(r,array,e.controller)&&
        cfgobserver::Same(r,e.pawn.object+PawnController,e.controller.object)&&
        cfgobserver::Same(r,e.controller.object+ControllerPawn,e.pawn.object)&&
        cfgobserver::Same(r,e.controller.object+ControllerState,e.controllerState)&&
        cfgobserver::Same(r,e.pawn.object+PawnState,e.controllerState)&&
        cfgobserver::Same(r,e.controllerState+StateOwner,e.controller.object);
}
// H2.48b diagnostics only: the first failing clause of Current (same reads, same order).
// 1 receipt incomplete, 2 world, 3 mode, 4 pawn==local, 5 controller==local, 6 pawn slot,
// 7 controller slot, 8 pawn->controller, 9 controller->pawn, 10 controller->state,
// 11 pawn->state, 12 state->owner; 0 = passes.
template<class R> unsigned CurrentClause(R& r,const cfgprobe::lifetime::ArrayHeader& array,const Receipt& e,const cfgobserver::Plan& plan){
    if(!e.Complete())return 1;if(e.world!=plan.world)return 2;if(e.mode!=plan.mode)return 3;
    if(e.pawn.object==plan.pawn)return 4;if(e.controller.object==plan.controller)return 5;
    if(!cfgjason::SameSlot(r,array,e.pawn))return 6;if(!cfgjason::SameSlot(r,array,e.controller))return 7;
    if(!cfgobserver::Same(r,e.pawn.object+PawnController,e.controller.object))return 8;
    if(!cfgobserver::Same(r,e.controller.object+ControllerPawn,e.pawn.object))return 9;
    if(!cfgobserver::Same(r,e.controller.object+ControllerState,e.controllerState))return 10;
    if(!cfgobserver::Same(r,e.pawn.object+PawnState,e.controllerState))return 11;
    if(!cfgobserver::Same(r,e.controllerState+StateOwner,e.controller.object))return 12;
    return 0;
}
}
namespace cfgjason {
// Sibling of LinkedKiller. Identical except the PawnAIClass==killerAI line is
// replaced by an exact current AdoptionReceipt for THIS pawn/controller pair.
struct AdoptedKillerProof {
    unsigned firstFailure=0,reads=0;
    explicit operator bool()const noexcept{return firstFailure==0;}
};
template<class Reader> AdoptedKillerProof LinkedAdoptedKillerProof(Reader& r,const cfgprobe::lifetime::ArrayHeader& array,
    const NewActorEvidence& e,const cfghosted::adoption::Receipt& receipt,const cfgobserver::Plan& human,std::uintptr_t target,
    std::uintptr_t pawnClass,std::uintptr_t defaults,std::uintptr_t killerBase,std::uintptr_t killerAI,std::uintptr_t pathClass,
    std::uintptr_t targetController);
template<class Reader> bool LinkedAdoptedKiller(Reader& r,const cfgprobe::lifetime::ArrayHeader& array,
    const NewActorEvidence& e,const cfghosted::adoption::Receipt& receipt,const cfgobserver::Plan& human,std::uintptr_t target,
    std::uintptr_t pawnClass,std::uintptr_t defaults,std::uintptr_t killerBase,std::uintptr_t killerAI,std::uintptr_t pathClass,
    std::uintptr_t targetController) {
    return LinkedAdoptedKillerProof(r,array,e,receipt,human,target,pawnClass,defaults,killerBase,killerAI,pathClass,targetController).firstFailure==0;
}
// Same ordered proof as LinkedAdoptedKiller, with an exact first-failing
// clause and bounded-read delta for diagnostics. It never relaxes a clause.
template<class Reader> AdoptedKillerProof LinkedAdoptedKillerProof(Reader& r,const cfgprobe::lifetime::ArrayHeader& array,
    const NewActorEvidence& e,const cfghosted::adoption::Receipt& receipt,const cfgobserver::Plan& human,std::uintptr_t target,
    std::uintptr_t pawnClass,std::uintptr_t defaults,std::uintptr_t killerBase,std::uintptr_t killerAI,std::uintptr_t pathClass,
    std::uintptr_t targetController) {
    const unsigned before=r.reads;unsigned clause=0;
    const auto step=[&](bool okay,unsigned id){if(!okay&&clause==0)clause=id;return okay;};
    if(!step(receipt.Complete()&&receipt.pawn==e.pawn&&receipt.controller==e.killerController&&
        e.killerController.object!=human.controller&&e.killerController.object!=targetController&&
        e.killerController.object!=e.initialController.object,1)||
       !step(NewPawn(r,array,e,human,target,pawnClass,defaults,killerBase),2)||
       !step(SameSlot(r,array,e.killerController),3)||
       !step(OwnedComponentSlot(r,array,e.path),4)||
       !step(cfgspawn::ObjectIs(r,e.killerController.object,killerAI),5)||
       !step(cfgspawn::ObjectIs(r,e.path.object,pathClass),6)||
       !step(cfghosted::adoption::Current(r,array,receipt,human),7)||
       !step(cfgobserver::Same(r,e.pawn.object+0x3A0,e.killerController.object),8)||
       !step(cfgobserver::Same(r,e.killerController.object+0x370,e.pawn.object),9)||
       !step(cfgobserver::Same(r,e.killerController.object+0x410,e.path.object),10)||
       !step(cfgobserver::Same(r,e.path.object+0x20,e.killerController.object),11)||
       !step(cfgobserver::Same(r,e.path.object+disk::ComponentOwner,e.killerController.object),12)||
       !step(SameSlot(r,array,e.pawn),13)||!step(SameSlot(r,array,e.killerController),14)||
       !step(OwnedComponentSlot(r,array,e.path),15)||
       !step(cfgobserver::Same(r,e.killerController.object+0x410,e.path.object),16)||
       !step(cfgobserver::Same(r,e.path.object+0x20,e.killerController.object),17)||
       !step(cfgobserver::Same(r,e.path.object+disk::ComponentOwner,e.killerController.object),18)){}
    return {clause,r.reads>=before?r.reads-before:0};
}
}
