#pragma once
#include "../EventObserver/NativeOriginalStartFrame.hpp"
#include "ConstructBoundary.generated.hpp"
namespace cfgmenu::startboundary {
struct Evidence {
    startbinding::Binding binding{};startroute::OriginalFrame invocation{};
    std::uintptr_t returnPc=0,wrapper=0,graph=0,node=0;
    std::array<cfgprobe::lifetime::SlotObservation,64> roots{};unsigned count=0;
};
template<class Reader>bool Origin(Reader& r,std::uintptr_t module,std::uintptr_t pc){
    if(pc!=module+constructdisk::Return)return false;
    std::array<unsigned char,constructdisk::InvokeCode.size()> code{};
    return r.ReadBytes(module+constructdisk::Invoke,code.data(),code.size())&&code==constructdisk::InvokeCode;
}
template<class Reader>bool Observe(Reader& r,const startbinding::Binding& binding,
    const startroute::OriginalFrame& frame,std::uintptr_t pc,std::uint32_t thread,
    std::uintptr_t wrapper,std::uintptr_t graph,Evidence& out,unsigned* stage=nullptr){
    out={};
    if(stage)*stage=1;if(!Origin(r,binding.context.module,pc))return false;
    if(stage)*stage=2;if(!startroute::CurrentFrame(r,binding,frame,thread))return false;
    if(stage)*stage=3;if(!startbinding::Current(r,binding,thread,wrapper,graph))return false;
    Evidence candidate;candidate.binding=binding;candidate.invocation=frame;
    candidate.returnPc=pc;candidate.wrapper=wrapper;candidate.graph=graph;
    candidate.node=frame.kind==startroute::SlotKind::Wrapper?binding.start.field.object:binding.graph.field.object;
    const auto add=[&](std::uintptr_t object){if(!object)return false;
        for(unsigned i=0;i<candidate.count;++i)if(candidate.roots[i].object==object)return true;
        if(candidate.count==candidate.roots.size()||!cfgprobe::lifetime::ReadSlot(r,binding.context.registry,object,candidate.roots[candidate.count]))return false;
        ++candidate.count;return true;};
    const auto& c=binding.context;
    if(stage)*stage=4;
    for(const auto object:{c.world,c.mode,c.controller,c.hud,c.stack,c.gameInstance,c.player,c.widget,c.type})if(!add(object))return false;
    if(stage)*stage=5;
    for(const auto* field:{&binding.start.field,&binding.construct.field,&binding.graph.field,&binding.entry.field})
        for(const auto object:{field->object,field->type,field->owner})if(!add(object))return false;
    if(stage)*stage=0;out=candidate;return true;
}
// Original Start may progress bytecode and replace the active menu. Keep the
// same borrowed native invocation/infrastructure, but never require its old
// Code cursor or invent a replacement frame after native entry.
template<class Reader>bool Current(Reader& r,const Evidence& e,std::uint32_t thread){
    const auto& original=e.binding.context;active::Context current;
    if(thread!=original.thread||!Origin(r,original.module,e.returnPc)||!active::Observe(r,original.module,current)||
       current.world!=original.world||current.engine!=original.engine||current.viewport!=original.viewport||
       current.mode!=original.mode||current.gameInstance!=original.gameInstance||current.player!=original.player||
       current.controller!=original.controller||current.hud!=original.hud||current.stack!=original.stack)return false;
    for(const auto i:{0u,1u,3u})if(!(current.identities[i]==original.identities[i]))return false;
    std::uintptr_t node=0,object=0,native=0,type=0,executor=0;
    cfgprobe::lifetime::SlotObservation widget;
    return r.Read(e.invocation.frame+startdisk::FrameNode,node)&&node==e.node&&
        r.Read(e.invocation.frame+startdisk::FrameObject,object)&&object==e.invocation.receiver&&
        r.Read(e.invocation.frame+startdisk::FrameCurrentNative,native)&&native==e.node&&
        r.Read(object+cfgprobe::ObjectClass,type)&&type==original.type&&
        r.Read(node+startdisk::Executor,executor)&&executor==(e.invocation.kind==startroute::SlotKind::Wrapper?e.wrapper:e.graph)&&
        cfgprobe::lifetime::ReadSlot(r,current.registry,object,widget)&&widget==original.identities[2];
}
// AFTER the already-admitted native executor returns, menu removal/world
// travel is expected. Completion may validate and release EXISTING claims,
// never acquire objects or admit another native call. No active-menu lookup.
// Caller must additionally prove its resident root handles remain acquired.
template<class Reader>bool ReturnedInvocation(Reader& r,const Evidence& e,std::uint32_t thread){
    const auto& c=e.binding.context;cfgprobe::lifetime::ArrayHeader registry{};
    if(thread!=c.thread||!Origin(r,c.module,e.returnPc)||!e.count||e.count>e.roots.size())return false;
    std::uint32_t ownerThread=0;unsigned char active=0;std::uintptr_t value=0;int count=0,capacity=0;
    if(!r.Read(c.module+cfgprobe::dispatch::ThreadIdRva,ownerThread)||ownerThread!=thread||
       !r.Read(c.module+cfgprobe::dispatch::ThreadFlagRva,active)||active!=1||
       !r.Read(c.module+cfgprobe::EngineGlobalCandidate,value)||value!=c.engine||
       !r.Read(c.engine+cfgprobe::EngineGameViewport,value)||value!=c.viewport||
       !r.Read(c.gameInstance+cfgprobe::GameInstanceLocalPlayers,value)||value!=c.players||
       !r.Read(c.gameInstance+cfgprobe::GameInstanceLocalPlayers+8,count)||count!=1||
       !r.Read(c.gameInstance+cfgprobe::GameInstanceLocalPlayers+12,capacity)||capacity<1||capacity>4||
       !r.Read(c.players,value)||value!=c.player||
       !cfgprobe::lifetime::ReadArrayHeader(r,c.module+cfgprobe::lifetime::ObjectArrayRva,registry))return false;
    for(const auto& owner:c.identities){cfgprobe::lifetime::SlotObservation observed;
        if(!owner.HasAssignedSerial()||!cfgprobe::lifetime::ReadSlot(r,registry,owner.object,observed)||!(observed==owner))return false;}
    std::uintptr_t node=0,object=0,native=0,type=0,executor=0;
    return r.Read(e.invocation.frame+startdisk::FrameNode,node)&&node==e.node&&
        r.Read(e.invocation.frame+startdisk::FrameObject,object)&&object==e.invocation.receiver&&
        r.Read(e.invocation.frame+startdisk::FrameCurrentNative,native)&&native==e.node&&
        r.Read(object+cfgprobe::ObjectClass,type)&&type==c.type&&
        r.Read(node+startdisk::Executor,executor)&&executor==(e.invocation.kind==startroute::SlotKind::Wrapper?e.wrapper:e.graph);
}
}
