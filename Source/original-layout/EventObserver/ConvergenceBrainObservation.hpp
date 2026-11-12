#pragma once
#include "JasonInstanceObservation.hpp"
namespace cfgconvergencetravel {
// A native controller strongly owns its BrainComponent. Like Jason's path
// component, it need not have an allocated weak serial. Never retain it on
// slot equality alone: recheck both owner links, native type and controller.
template<class R> bool Brain(R& r,const cfgprobe::lifetime::ArrayHeader& array,
    const cfgprobe::lifetime::SlotObservation& controller,std::uintptr_t aiClass,
    std::uintptr_t brainClass,cfgprobe::lifetime::SlotObservation& out){
    out={};std::uintptr_t brain=0;cfgprobe::lifetime::SlotObservation slot{};
    if(!cfgjason::SameSlot(r,array,controller)||!cfgspawn::ObjectIs(r,controller.object,aiClass)||
       !r.Read(controller.object+0x418,brain)||!brain||!cfgspawn::ObjectSlot(r,array,brain,slot)||
       !cfgspawn::ObjectIs(r,brain,brainClass)||
       !cfgobserver::Same(r,brain+0x20,controller.object)||
       !cfgobserver::Same(r,brain+cfgjason::disk::ComponentOwner,controller.object)||
       !cfgjason::SameSlot(r,array,controller)||!cfgjason::OwnedComponentSlot(r,array,slot)||
       !cfgobserver::Same(r,controller.object+0x418,brain)||
       !cfgobserver::Same(r,brain+0x20,controller.object)||
       !cfgobserver::Same(r,brain+cfgjason::disk::ComponentOwner,controller.object))return false;
    out=slot;return true;
}
}
