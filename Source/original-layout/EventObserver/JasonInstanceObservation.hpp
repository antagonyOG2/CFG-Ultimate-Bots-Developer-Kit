#pragma once
#include "SpawnResultObservation.hpp"
#include "../RuntimeProbe/JasonApproachBindings.generated.hpp"
namespace cfgjason {
struct NewActorEvidence {
    cfgprobe::lifetime::SlotObservation pawn{},initialController{},killerController{},path{};
};
template<class Reader> bool SameSlot(Reader& r,const cfgprobe::lifetime::ArrayHeader& array,
    const cfgprobe::lifetime::SlotObservation& expected) {
    cfgprobe::lifetime::SlotObservation fresh;
    return expected.HasAssignedSerial() && cfgspawn::ObjectSlot(r,array,expected.object,fresh) && fresh==expected;
}
template<class Reader> bool OwnedComponentSlot(Reader& r,const cfgprobe::lifetime::ArrayHeader& array,
    const cfgprobe::lifetime::SlotObservation& expected) {
    // A strong controller-owned subobject can have no assigned weak serial.
    // This is NOT an identity/GC pin. LinkedKiller must corroborate its owner,
    // native type and exact controller reference on both sides of observation.
    cfgprobe::lifetime::SlotObservation fresh;
    return cfgspawn::ObjectSlot(r,array,expected.object,fresh) && fresh==expected;
}
// All state writes belong to a positively identified, newly spawned instance.
// Never accept a supplied human/target/controller/CDO as that new actor.
template<class Reader> bool NewPawn(Reader& r,const cfgprobe::lifetime::ArrayHeader& array,
    const NewActorEvidence& e,const cfgobserver::Plan& human,std::uintptr_t target,
    std::uintptr_t pawnClass,std::uintptr_t defaults,std::uintptr_t killerBase) {
    return e.pawn.object!=human.pawn && e.pawn.object!=target && e.pawn.object!=defaults &&
        SameSlot(r,array,e.pawn) && cfgobserver::Same(r,e.pawn.object+cfgprobe::ObjectClass,pawnClass) &&
        cfgspawn::ObjectIs(r,e.pawn.object,killerBase);
}
template<class Reader> bool Detached(Reader& r,const cfgprobe::lifetime::ArrayHeader& array,
    const NewActorEvidence& e,const cfgobserver::Plan& human,std::uintptr_t target,std::uintptr_t pawnClass,
    std::uintptr_t defaults,std::uintptr_t killerBase,std::uintptr_t defaultAI,std::uintptr_t targetController) {
    return e.initialController.object!=human.controller && e.initialController.object!=targetController &&
        NewPawn(r,array,e,human,target,pawnClass,defaults,killerBase) && SameSlot(r,array,e.initialController) &&
        cfgobserver::Same(r,e.initialController.object+cfgprobe::ObjectClass,defaultAI) &&
        cfgobserver::Same(r,e.initialController.object+0x370,std::uintptr_t{0}) &&
        cfgobserver::Same(r,e.pawn.object+0x3A0,std::uintptr_t{0}) &&
        cfgobserver::Same(r,e.pawn.object+cfgspawn::PawnAIClass,defaultAI) &&
        SameSlot(r,array,e.pawn) && SameSlot(r,array,e.initialController);
}
template<class Reader> bool LinkedKiller(Reader& r,const cfgprobe::lifetime::ArrayHeader& array,
    const NewActorEvidence& e,const cfgobserver::Plan& human,std::uintptr_t target,std::uintptr_t pawnClass,
    std::uintptr_t defaults,std::uintptr_t killerBase,std::uintptr_t killerAI,std::uintptr_t pathClass,std::uintptr_t targetController) {
    return e.killerController.object!=human.controller && e.killerController.object!=targetController &&
        e.killerController.object!=e.initialController.object &&
        NewPawn(r,array,e,human,target,pawnClass,defaults,killerBase) && SameSlot(r,array,e.killerController) &&
        OwnedComponentSlot(r,array,e.path) && cfgspawn::ObjectIs(r,e.killerController.object,killerAI) &&
        cfgspawn::ObjectIs(r,e.path.object,pathClass) &&
        cfgobserver::Same(r,e.pawn.object+cfgspawn::PawnAIClass,killerAI) &&
        cfgobserver::Same(r,e.pawn.object+0x3A0,e.killerController.object) &&
        cfgobserver::Same(r,e.killerController.object+0x370,e.pawn.object) &&
        cfgobserver::Same(r,e.killerController.object+0x410,e.path.object) &&
        cfgobserver::Same(r,e.path.object+0x20,e.killerController.object) &&
        cfgobserver::Same(r,e.path.object+disk::ComponentOwner,e.killerController.object) &&
        SameSlot(r,array,e.pawn) && SameSlot(r,array,e.killerController) && OwnedComponentSlot(r,array,e.path) &&
        cfgobserver::Same(r,e.killerController.object+0x410,e.path.object) &&
        cfgobserver::Same(r,e.path.object+0x20,e.killerController.object) &&
        cfgobserver::Same(r,e.path.object+disk::ComponentOwner,e.killerController.object);
}
} // namespace cfgjason
