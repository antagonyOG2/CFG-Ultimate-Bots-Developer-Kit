#pragma once
#include "ObjectivePanelObservation.hpp"
namespace cfgobjectiveattachment {
struct Edge {
    cfgprobe::lifetime::SlotObservation widget{},slot{},parent{};
    std::uintptr_t widgetType=0,parentType=0;
};
// Diagnostic evidence ONLY. A PanelSlot edge is not a local-player ownership
// proof. No native getter, finder, allocation, write or gameplay polling here.
template<class Reader> bool Parent(Reader& r,const cfgprobe::lifetime::ArrayHeader& registry,
    std::uintptr_t object,std::uintptr_t widgetClass,std::uintptr_t slotClass,std::uintptr_t panelClass,Edge& out){
    using namespace cfgobjectivepanel::disk;out={};Edge edge;
    std::uintptr_t slot=0,parent=0,content=0,type=0,again=0;
    if(!cfgprobe::lifetime::ReadSlot(r,registry,object,edge.widget)||!edge.widget.HasAssignedSerial()||
       !r.Read(object+cfgprobe::ObjectClass,edge.widgetType)||!cfgmenu::maps::Subclass(r,edge.widgetType,widgetClass)||
       !r.Read(object+WidgetSlot,slot)||!slot||
       !cfgprobe::lifetime::ReadSlot(r,registry,slot,edge.slot)||!edge.slot.HasAssignedSerial()||
       !r.Read(slot+cfgprobe::ObjectClass,type)||!cfgmenu::maps::Subclass(r,type,slotClass)||
       !r.Read(slot+SlotContent,content)||content!=object||!r.Read(slot+SlotParent,parent)||!parent||
       !cfgprobe::lifetime::ReadSlot(r,registry,parent,edge.parent)||!edge.parent.HasAssignedSerial()||
       !r.Read(parent+cfgprobe::ObjectClass,edge.parentType)||!cfgmenu::maps::Subclass(r,edge.parentType,panelClass))return false;
    cfgprobe::lifetime::SlotObservation current;
    if(!r.Read(object+WidgetSlot,again)||again!=slot||!r.Read(slot+SlotContent,again)||again!=object||
       !r.Read(slot+SlotParent,again)||again!=parent||
       !cfgprobe::lifetime::ReadSlot(r,registry,object,current)||!(current==edge.widget)||
       !cfgprobe::lifetime::ReadSlot(r,registry,slot,current)||!(current==edge.slot)||
       !cfgprobe::lifetime::ReadSlot(r,registry,parent,current)||!(current==edge.parent))return false;
    out=edge;return true;
}
}
