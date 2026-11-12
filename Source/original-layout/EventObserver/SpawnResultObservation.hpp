#pragma once
#include "OfflineContextObservation.hpp"
#include "../RuntimeProbe/AiResourceSnapshot.hpp"
namespace cfgspawn {
// Native spawning may extend/reallocate GUObjectArray. Refresh its bounded
// header only; preserve/revalidate every original human/context identity.
template<class Reader,class Scope> bool ObserveCounselorSpawnResultScoped(Reader& r,const cfgobserver::Plan& before,
    std::uintptr_t gameState,std::uintptr_t pawnClass,std::uintptr_t counselorClass,
    std::uintptr_t aiClass,std::uintptr_t brainClass,std::uintptr_t pawn,std::uintptr_t& outputController,Scope&& scope) {
    outputController=0;
    cfgprobe::lifetime::ArrayHeader after;
    if (!cfgprobe::lifetime::ReadArrayHeader(r,before.module+cfgprobe::lifetime::ObjectArrayRva,after)) return false;
    auto freshPlan=before; freshPlan.objectArray=after;
    cfgprobe::lifetime::SlotObservation pawnSlot,controllerSlot,fresh;
    std::uintptr_t controller=0,actualClass=0,back=0,brain=0;
    if (!scope(freshPlan) ||
        !cfgobserver::Same(r,before.world+cfgprobe::WorldGameState,gameState) ||
        !ObjectSlot(r,after,pawn,pawnSlot) || !pawnSlot.HasAssignedSerial() ||
        !r.Read(pawn+cfgprobe::ObjectClass,actualClass) || actualClass!=pawnClass ||
        !r.Read(pawn+0x3A0,controller) || controller==before.controller ||
        !ObjectSlot(r,after,controller,controllerSlot) || !controllerSlot.HasAssignedSerial() ||
        !r.Read(controller+0x370,back) || back!=pawn || !r.Read(controller+0x418,brain) ||
        !ObjectIs(r,pawn,counselorClass) || !ObjectIs(r,controller,aiClass) || !ObjectIs(r,brain,brainClass) ||
        !ObjectSlot(r,after,pawn,fresh) || !(fresh==pawnSlot) ||
        !ObjectSlot(r,after,controller,fresh) || !(fresh==controllerSlot) ||
        !cfgobserver::Same(r,pawn+0x3A0,controller) || !cfgobserver::Same(r,controller+0x370,pawn) ||
        !cfgobserver::Same(r,controller+0x418,brain) ||
        !scope(freshPlan)) return false;
    outputController=controller; return true;
}
template<class Reader> bool ObserveCounselorSpawnResult(Reader& r,const cfgobserver::Plan& before,
    std::uintptr_t gameState,std::uintptr_t pawnClass,std::uintptr_t counselorClass,
    std::uintptr_t aiClass,std::uintptr_t brainClass,std::uintptr_t pawn,std::uintptr_t& outputController){
    return ObserveCounselorSpawnResultScoped(r,before,gameState,pawnClass,counselorClass,aiClass,brainClass,pawn,outputController,
        [&](const cfgobserver::Plan& fresh){return cfgobserver::ContextUnchanged(r,fresh);});
}
} // namespace cfgspawn
