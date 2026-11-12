#pragma once
#include "OfflineContextObservation.hpp"
#include "../RuntimeProbe/AiResourceSnapshot.hpp"
#include "../RuntimeProbe/HunterBindings.generated.hpp"
#include "../RuntimeProbe/OwnedGenericDispatcher.hpp"
#include <cmath>
namespace cfgbundle {
template<class R,class A> bool Code(R& r,std::uintptr_t module,std::uintptr_t rva,const A& expected){
    if(rva==cfgspawn::GenericDispatcherRva&&expected.size()==cfgspawn::GenericDispatcherPrefix.size())
        return std::memcmp(expected.data(),cfgspawn::GenericDispatcherPrefix.data(),cfgspawn::GenericDispatcherPrefix.size())==0&&cfgdispatch::Proof(r,module);
    A fresh{};return r.Read(module+rva,fresh)&&fresh==expected;
}
template<class R> bool ReleaseBinding(R& r,std::uintptr_t module,std::uintptr_t pawn){
    std::uintptr_t table=0;
    return r.Read(pawn,table)&&cfgprobe::ObjectPointer(table)&&
        cfgobserver::Same(r,table+cfghunter::disk::ReleaseSlot,module+cfghunter::disk::Release)&&
        Code(r,module,cfghunter::disk::Release,cfghunter::disk::ReleaseBytes);
}
template<class R> bool PressBinding(R& r,std::uintptr_t module,std::uintptr_t pawn){
    std::uintptr_t table=0;
    return r.Read(pawn,table)&&cfgprobe::ObjectPointer(table)&&
        cfgobserver::Same(r,table+cfghunter::disk::PressSlot,module+cfghunter::disk::Press)&&
        Code(r,module,cfghunter::disk::Press,cfghunter::disk::PressBytes);
}
template<class R> bool NativeBindings(R& r,std::uintptr_t module,std::uintptr_t pawn){
    std::uintptr_t table=0;
    return r.Read(pawn,table)&&cfgprobe::ObjectPointer(table)&&
        cfgobserver::Same(r,table+cfghunter::disk::PressSlot,module+cfghunter::disk::Press)&&
        cfgobserver::Same(r,table+cfghunter::disk::ReleaseSlot,module+cfghunter::disk::Release)&&
        cfgobserver::Same(r,table+cfghunter::disk::CanAttackSlot,module+cfghunter::disk::CanAttack)&&
        Code(r,module,cfghunter::disk::Press,cfghunter::disk::PressBytes)&&
        Code(r,module,cfghunter::disk::Release,cfghunter::disk::ReleaseBytes)&&
        Code(r,module,cfghunter::disk::CanAttack,cfghunter::disk::CanAttackBytes)&&
        Code(r,module,cfghunter::disk::Stunned,cfghunter::disk::StunnedBytes)&&
        Code(r,module,cfghunter::disk::Attacking,cfghunter::disk::AttackingBytes);
}
template<class R> bool Manager(R& r,std::uintptr_t module,const cfgprobe::lifetime::ArrayHeader& objects,
    std::uintptr_t pawn,std::uintptr_t native,const cfgprobe::lifetime::SlotObservation& nativeSlot,unsigned* failedClause=nullptr,
    cfgprobe::lifetime::SlotObservation* hostedMetadata=nullptr){
    if(failedClause)*failedClause=0;
    const auto refuse=[&](unsigned clause){if(failedClause)*failedClause=clause;return false;};
    if(nativeSlot.object!=native)return refuse(1);
    std::uintptr_t component=0;cfgprobe::lifetime::SlotObservation slot{},fresh{},classNow{};
    if(!cfgobserver::Same(r,module+cfghunter::disk::ManagerReflection,native))return refuse(2);
    if(!cfgobserver::Same(r,module+cfghunter::disk::ManagerConstructor,native))return refuse(3);
    if(!cfgspawn::ObjectSlot(r,objects,native,classNow)||(!(classNow==nativeSlot)&&
       !(hostedMetadata&&nativeSlot.serial==0&&classNow.HasAssignedSerial()&&
         classNow.object==nativeSlot.object&&classNow.index==nativeSlot.index)))return refuse(4);
    if(!r.Read(pawn+cfghunter::disk::Manager,component))return refuse(5);
    if(!cfgspawn::ObjectSlot(r,objects,component,slot))return refuse(6);
    if(!cfgspawn::ObjectIs(r,component,native))return refuse(7);
    if(!cfgobserver::Same(r,component+0x20,pawn))return refuse(8);
    if(!cfgobserver::Same(r,component+0xE0,pawn))return refuse(9);
    if(!cfgspawn::ObjectSlot(r,objects,component,fresh)||!(fresh==slot))return refuse(10);
    if(!cfgobserver::Same(r,pawn+cfghunter::disk::Manager,component))return refuse(11);
    if(hostedMetadata){
        if(!cfgspawn::ObjectSlot(r,objects,native,fresh)||!(fresh==classNow)||
           !cfgobserver::Same(r,module+cfghunter::disk::ManagerReflection,native)||
           !cfgobserver::Same(r,module+cfghunter::disk::ManagerConstructor,native))return refuse(12);
        *hostedMetadata=classNow;
    }
    return true;
}
template<class R> bool Alive(R& r,std::uintptr_t pawn,bool& alive){
    float health=0;alive=false;
    if(!r.Read(pawn+cfghunter::disk::Health,health)||!std::isfinite(health))return false;
    alive=health>cfghunter::disk::AliveThreshold;return true;
}
}
