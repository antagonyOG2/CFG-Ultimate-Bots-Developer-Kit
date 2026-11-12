#pragma once
#include "../RuntimeProbe/AiResourceSnapshot.hpp"
#include "../RuntimeProbe/TrapPriorityBindings.generated.hpp"
#include "../Core/TrapPriorityPolicy.hpp"
namespace cfgtrappriority {
using Slot=cfgprobe::lifetime::SlotObservation;
// Called AFTER the original native callback. No scan, movement, damage or
// teleport inside the trap overlap; queue only the positively owned victim.
template<class R> bool Capture(R& r,const cfgprobe::lifetime::ArrayHeader& array,std::uintptr_t module,
    std::uintptr_t killer,std::uintptr_t pawn,std::uintptr_t trap,Slot& victimSlot,Slot& trapSlot){
    if(!disk::RuntimeVerified)return false;
    std::uintptr_t type=0,constructor=0,armer=0,linked=0,reflectionAfter=0,constructorAfter=0;
    unsigned char role=0;Slot freshVictim{},freshTrap{},classSlot{},freshClass{};
    return killer&&pawn&&trap&&pawn!=killer&&
        r.Read(module+disk::TrapReflection,type)&&r.Read(module+disk::TrapConstructor,constructor)&&constructor&&
        (!type||type==constructor)&&cfgspawn::ObjectSlot(r,array,constructor,classSlot)&&
        cfgspawn::ObjectSlot(r,array,pawn,victimSlot)&&victimSlot.HasAssignedSerial()&&
        cfgspawn::ObjectSlot(r,array,trap,trapSlot)&&cfgspawn::ObjectIs(r,trap,constructor)&&
        r.Read(trap+0x110,role)&&role==3&&r.Read(trap+disk::TrapArmer,armer)&&armer==killer&&
        r.Read(pawn+disk::TriggeredTrap,linked)&&linked==trap&&
        cfgspawn::ObjectSlot(r,array,pawn,freshVictim)&&freshVictim==victimSlot&&
        cfgspawn::ObjectSlot(r,array,trap,freshTrap)&&freshTrap==trapSlot&&
        r.Read(trap+disk::TrapArmer,armer)&&armer==killer&&r.Read(pawn+disk::TriggeredTrap,linked)&&linked==trap&&
        cfgspawn::ObjectSlot(r,array,constructor,freshClass)&&freshClass==classSlot&&
        r.Read(module+disk::TrapConstructor,constructorAfter)&&constructorAfter==constructor&&
        r.Read(module+disk::TrapReflection,reflectionAfter)&&reflectionAfter==type;
}
}
