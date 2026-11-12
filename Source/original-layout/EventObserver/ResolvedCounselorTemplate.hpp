#pragma once
#include "OwnedCounselorTemplate.hpp"
namespace cfgbundle {
// The donor's counselor CDO can contain a behavior tree. The killer-only
// template observer requires a null tree and must not be reused here.
template<class R> bool ResolvedCounselorTemplate(R& r,const cfgprobe::lifetime::ArrayHeader& array,
    std::uintptr_t type,std::uintptr_t counselor,std::uintptr_t ai,cfgspawn::TemplateObservation& out){
    out={};cfgspawn::TemplateObservation value{};value.pawnClass=type;
    if(!cfgspawn::ClassContains(r,type,counselor)||!cfgspawn::ObjectSlot(r,array,type,value.classSlot)||
        !r.Read(type+cfgspawn::ClassDefaultObject,value.defaults)||
        !cfgspawn::ObjectSlot(r,array,value.defaults,value.defaultSlot))return false;
    std::uintptr_t actual=0;
    if(!r.Read(value.defaults+cfgprobe::ObjectClass,actual)||actual!=type||
        !r.Read(value.defaults+cfgspawn::PawnAIClass,value.defaultAI)||!cfgspawn::ClassContains(r,value.defaultAI,ai))return false;
    cfgprobe::lifetime::SlotObservation c{},d{};
    if(!cfgspawn::ObjectSlot(r,array,type,c)||!(c==value.classSlot)||
        !r.Read(type+cfgspawn::ClassDefaultObject,actual)||actual!=value.defaults||
        !cfgspawn::ObjectSlot(r,array,value.defaults,d)||!(d==value.defaultSlot)||
        !r.Read(value.defaults+cfgprobe::ObjectClass,actual)||actual!=type||
        !r.Read(value.defaults+cfgspawn::PawnAIClass,actual)||actual!=value.defaultAI||!cfgspawn::ClassContains(r,actual,ai)||
        !cfgspawn::ClassContains(r,type,counselor))return false;
    value.kind=1;value.aiKind=2;out=value;return true;
}
}
