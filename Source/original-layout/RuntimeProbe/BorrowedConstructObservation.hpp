#pragma once
#include "PickerPrecreationObservation.hpp"
#include "ConstructBoundary.generated.hpp"
namespace cfgmenu::constructboundary {
constexpr unsigned MaximumRoots=512;
struct Evidence {
    active::Context source{};pickerprepare::Receiver receiver{};
    std::uintptr_t frame=0,node=0,executor=0,returnPc=0,getter=0;
    std::array<cfgprobe::lifetime::SlotObservation,MaximumRoots> roots{};unsigned count=0;
};
template<class Reader>bool Origin(Reader& r,std::uintptr_t module,std::uintptr_t pc){
    if(pc!=module+constructdisk::Return)return false;
    std::array<unsigned char,constructdisk::InvokeCode.size()> bytes{};
    return r.ReadBytes(module+constructdisk::Invoke,bytes.data(),bytes.size())&&bytes==constructdisk::InvokeCode;
}
template<class Add>bool Dependencies(const active::Context& actual,const pickerprepare::Prepared& prepared,std::uintptr_t widget,Add&& add){
    const auto field=[&](const cfgreflection::Field& f){return !f.object||(add(f.object)&&add(f.type)&&add(f.owner));};
    for(const auto object:{actual.world,actual.mode,actual.controller,actual.hud,actual.stack,actual.gameInstance,actual.player,
        widget,prepared.type,prepared.lobbyClass,prepared.baseClass,prepared.nativeState,prepared.source.transition,
        prepared.owningPlayer.owner,prepared.owningPlayer.controllerClass})if(!add(object))return false;
    for(const auto& slot:prepared.source.classes)if(!add(slot.object))return false;
    for(const auto* f:{&prepared.source.mainGraph.function,&prepared.source.customizeGraph.function,&prepared.graph.function,
        &prepared.owningPlayer.function.function})if(!field(*f))return false;
    for(const auto& handler:prepared.handlers)if(handler.present){
        if(!field(handler.slot.field))return false;
        for(unsigned i=0;i<handler.fieldCount;++i)if(!field(handler.fields[i].field))return false;}
    for(const auto* p:{&prepared.index,&prepared.roster,&prepared.lobbyFlag,&prepared.lobbyLocal,&prepared.baseLocal,
        &prepared.owningPlayer.function.fields[0]})if(!field(p->field))return false;
    return true;
}
// Called synchronously from the real published Construct thunk, never from a
// saved request/timer. It supplies provenance, not collection/registration
// authority: the provider must still acquire the normal engine/list window.
template<class Reader>bool Observe(Reader& r,const active::Context& actual,const pickerprepare::Prepared& prepared,
    const pickerprepare::Executors& expected,std::uintptr_t widget,std::uintptr_t frame,std::uintptr_t pc,
    std::uint32_t thread,Evidence& out,unsigned* stage=nullptr){
    out={};auto value=std::make_unique<Evidence>();auto& e=*value;e.source=actual;e.frame=frame;e.returnPc=pc;e.getter=prepared.owningPlayer.function.function.object;
    if(stage)*stage=1;
    if(thread!=actual.thread||!Origin(r,actual.module,pc))return false;
    if(stage)*stage=2;
    if(!pickerprepare::FirstConstruct(r,actual,prepared,expected,widget,frame,thread,e.receiver)||
       !r.Read(frame+startdisk::FrameNode,e.node)||!r.Read(e.node+startdisk::Executor,e.executor))return false;
    const auto add=[&](std::uintptr_t object){
        if(!object)return true;for(unsigned i=0;i<e.count;++i)if(e.roots[i].object==object)return true;
        if(e.count==e.roots.size()||!cfgprobe::lifetime::ReadSlot(r,actual.registry,object,e.roots[e.count]))return false;
        ++e.count;return true;
    };
    if(stage)*stage=3;
    if(!Dependencies(actual,prepared,widget,add))return false;
    if(stage)*stage=0;out=e;return true;
}
// After forwarding, Code legitimately advances and the active menu may be
// replaced. Fixed infrastructure and the SAME live engine invocation frame
// remain required. A new world/controller/HUD or receiver is never adopted.
template<class Reader>bool Current(Reader& r,const Evidence& e,std::uint32_t thread){
    active::Context c;std::uintptr_t node=0,object=0,native=0,type=0,executor=0;
    cfgprobe::lifetime::SlotObservation receiver;
    if(thread!=e.source.thread||!Origin(r,e.source.module,e.returnPc)||!active::Observe(r,e.source.module,c)||
       c.engine!=e.source.engine||c.viewport!=e.source.viewport||c.world!=e.source.world||c.mode!=e.source.mode||
       c.gameInstance!=e.source.gameInstance||c.player!=e.source.player||c.controller!=e.source.controller||
       c.hud!=e.source.hud||c.stack!=e.source.stack)return false;
    for(const auto i:{0U,1U,3U})if(!(c.identities[i]==e.source.identities[i]))return false;
    return r.Read(e.frame+startdisk::FrameNode,node)&&node==e.node&&
        r.Read(e.frame+startdisk::FrameObject,object)&&object==e.receiver.widget&&
        r.Read(e.frame+startdisk::FrameCurrentNative,native)&&native==e.node&&
        r.Read(e.node+startdisk::Executor,executor)&&executor==e.executor&&
        r.Read(object+cfgprobe::ObjectClass,type)&&type==e.receiver.type&&
        cfgprobe::lifetime::ReadSlot(r,c.registry,object,receiver)&&receiver==e.receiver.identity;
}
}
