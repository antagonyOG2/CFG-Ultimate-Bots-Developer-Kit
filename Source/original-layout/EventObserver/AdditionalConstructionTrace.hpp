#pragma once
#include "../RuntimeProbe/AdditionalConstructionBindings.hpp"
#include "OfflineDiagnosticTrace.hpp"
namespace cfgadditionalconstruction {
struct Scope {
    std::uintptr_t module=0,world=0,mode=0,gameState=0,type=0,primary=0,human=0;
    unsigned thread=0;bool active=false;
    std::uintptr_t actor=0;unsigned observations=0,events=0;
    std::uintptr_t beginActor=0,primaryState=0,primaryController=0;
    cfgprobe::lifetime::SlotObservation primarySlot{},actorSlot{},primaryControllerSlot{};
    unsigned primarySkipped=0;
    bool hosted=false; // H2.59c: a listen server has a NetDriver; every other proof below is unchanged
};
struct Sample {
    cfgprobe::lifetime::SlotObservation slot{};
    std::uintptr_t pawn=0,state=0,controller=0;std::uint32_t flags=0;
};
template<class R> bool Observe(R& r,const Scope& scope,std::uintptr_t actor,unsigned thread,Sample& out){
    if(!scope.active||thread!=scope.thread||actor==scope.primary||actor==scope.human||!actor)return false;
    cfgprobe::lifetime::ArrayHeader array{};std::uintptr_t type=0;
    if(!cfgprimary::LookupSafe(r,scope.module)||
       (!scope.hosted&&!cfgobserver::Same(r,scope.world+cfgprobe::WorldNetDriver,std::uintptr_t{}))||
       !cfgobserver::Same(r,scope.world+cfgprobe::WorldGameMode,scope.mode)||
       !cfgobserver::Same(r,scope.world+cfgprobe::WorldGameState,scope.gameState)||
       !r.Read(actor+cfgprobe::ObjectClass,type)||type!=scope.type||
       !cfgprobe::lifetime::ReadArrayHeader(r,scope.module+cfgprobe::lifetime::ObjectArrayRva,array)||
       !cfgprobe::lifetime::ReadSlot(r,array,actor,out.slot)||
       !r.Read(array.data+24ULL*static_cast<unsigned>(out.slot.index)+8,out.flags)||
       !r.Read(scope.gameState+cfgprimary::CurrentKiller,out.pawn)||
       !r.Read(scope.gameState+cfgprimary::CurrentKillerState,out.state)||!r.Read(actor+0x3A0,out.controller))return false;
    // Serial0 is logged as unassigned, never promoted to mutation authority.
    return true;
}
template<class Original,class Witness> void Forward(Original original,Witness witness,void* actor,const void* transform,
    void* owner,void* instigator,unsigned char remote,unsigned char noFail,unsigned char defer){
    witness(false);original(actor,transform,owner,instigator,remote,noFail,defer);witness(true);
}
// Only the two enumerator calls in the exact extra's native BeginPlay may
// advance past the already-owned PRIMARY. Do not hide foreign killers, touch
// actor lists/flags/possession, or bypass any other native duplicate check.
template<class R> bool RegisteredExtra(R& r,const Scope& s,unsigned thread){
    if(!s.actorSlot.HasAssignedSerial()||s.actorSlot.object!=s.actor||
       !s.primarySlot.HasAssignedSerial()||s.primarySlot.object!=s.primary)return false;
    Sample sample{};cfgprobe::lifetime::ArrayHeader array{};cfgprobe::lifetime::SlotObservation primary{},controller{};
    // PostInitializeComponents publishes EXTRA before BeginPlay. Neither a
    // null/foreign singleton nor an unassigned/reused actor grants permission.
    return Observe(r,s,s.actor,thread,sample)&&!sample.flags&&sample.slot==s.actorSlot&&
       sample.pawn==s.actor&&sample.state==s.primaryState&&
       cfgobserver::Same(r,s.primary+0x3A0,s.primaryController)&&
       cfgobserver::Same(r,s.primaryController+0x370,s.primary)&&
       cfgprobe::lifetime::ReadArrayHeader(r,s.module+cfgprobe::lifetime::ObjectArrayRva,array)&&
       cfgspawn::ObjectSlot(r,array,s.primary,primary)&&primary==s.primarySlot&&
       s.primaryControllerSlot.HasAssignedSerial()&&s.primaryControllerSlot.object==s.primaryController&&
       cfgspawn::ObjectSlot(r,array,s.primaryController,controller)&&controller==s.primaryControllerSlot;
}
template<class R> bool BeginAllowed(R& r,const Scope& s,std::uintptr_t actor,unsigned thread){
    return s.active&&thread==s.thread&&!s.beginActor&&actor==s.actor&&RegisteredExtra(r,s,thread);
}

// A native fault can occur after construction but before SpawnActor assigns
// its return value. This exact scoped witness grants cleanup only, never a
// successful birth or adoption of an actor found by scanning the world.
template<class R> bool CleanupWitness(R& r,const Scope& s,unsigned thread,
    cfgprobe::lifetime::SlotObservation& out){
    if(!s.active||thread!=s.thread||s.actor==s.primary||s.actor==s.human||
       !RegisteredExtra(r,s,thread))return false;
    out=s.actorSlot;return true;
}
template<class R> bool SkipPrimary(R& r,const Scope& s,std::uintptr_t iterator,std::uintptr_t caller,unsigned thread){
    if(!s.active||thread!=s.thread||!s.actor||s.beginActor!=s.actor||s.actor==s.primary||s.actor==s.human||
       s.primarySkipped||!s.primarySlot.HasAssignedSerial()||s.primarySlot.object!=s.primary||
       (caller!=s.module+disk::FirstIteratorReturn&&caller!=s.module+disk::NextIteratorReturn))return false;
    std::uintptr_t world=0,current=0;std::uint8_t ended=1;
    if(!RegisteredExtra(r,s,thread)||
       !r.Read(iterator+8,world)||world!=s.world||!r.Read(iterator+0x24,ended)||ended||
       !r.Read(iterator+0x30,current)||current!=s.primary||
       !cfgobserver::Same(r,s.primary+0x3A0,s.primaryController)||
       !cfgobserver::Same(r,s.primaryController+0x370,s.primary))return false;
    cfgprobe::lifetime::ArrayHeader array{};cfgprobe::lifetime::SlotObservation slot{};
    return cfgprobe::lifetime::ReadArrayHeader(r,s.module+cfgprobe::lifetime::ObjectArrayRva,array)&&
       cfgspawn::ObjectSlot(r,array,s.primary,slot)&&slot==s.primarySlot&&
       cfgobserver::Same(r,iterator+0x30,s.primary)&&cfgobserver::Same(r,iterator+8,s.world);
}
template<class Original,class MaySkip,class Witness> void Advance(Original original,MaySkip maySkip,Witness witness,void* iterator){
    original(iterator);
    if(maySkip()){witness();original(iterator);} // exactly one extra native advance
}
}
