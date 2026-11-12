#pragma once
#include "OfflineContextObservation.hpp"
#include "SpawnAssetObservation.hpp"
namespace cfgbundle {
inline bool OwnedClassSlotMatches(const cfgprobe::lifetime::SlotObservation& saved,
    const cfgprobe::lifetime::SlotObservation& current){
    return saved.object==current.object&&saved.index==current.index&&saved.serial>=0&&current.serial>=0&&
        (saved.serial==0||saved.serial==current.serial);
}
// Class/CDO resources may have no assigned weak serial. Their exact objects,
// indices AND the acknowledged human -> class -> CDO -> native AI ownership
// chain must stay unchanged. Native assignment of a previously absent serial
// is allowed only if both current samples agree. An originally positive serial
// may never change. No serial assignment, class adoption or CDO write.
template<class R> bool OwnedCounselorTemplate(R& r,const cfgprobe::lifetime::ArrayHeader& array,
    std::uintptr_t human,const cfgspawn::TemplateObservation& t,std::uintptr_t counselorBase,
    std::uintptr_t counselorAI) {
    if(t.pawn!=human||t.kind!=1||t.defaultAI!=counselorAI||
       t.classSlot.object!=t.pawnClass||t.defaultSlot.object!=t.defaults)return false;
    cfgprobe::lifetime::SlotObservation previousClass{},previousDefault{};
    for(unsigned sample=0;sample<2;++sample){
        cfgprobe::lifetime::SlotObservation c{},d{};
        if(!cfgobserver::Same(r,human+cfgprobe::ObjectClass,t.pawnClass)||
           !cfgspawn::ObjectSlot(r,array,t.pawnClass,c)||!OwnedClassSlotMatches(t.classSlot,c)||
           !cfgobserver::Same(r,t.pawnClass+cfgspawn::ClassDefaultObject,t.defaults)||
           !cfgspawn::ObjectSlot(r,array,t.defaults,d)||!OwnedClassSlotMatches(t.defaultSlot,d)||
           !cfgobserver::Same(r,t.defaults+cfgprobe::ObjectClass,t.pawnClass)||
           !cfgobserver::Same(r,t.defaults+cfgspawn::PawnAIClass,counselorAI)||
           !cfgspawn::ObjectIs(r,human,counselorBase))return false;
        if(sample&&(!(c==previousClass)||!(d==previousDefault)))return false;
        previousClass=c;previousDefault=d;
    }
    return true;
}
}
