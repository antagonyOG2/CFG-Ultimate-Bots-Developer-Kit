#pragma once
#include "HostedLobbyDiscovery.hpp"
#include "HostedLobbyPlacement.hpp"
namespace cfghostedlobby {
// Original native anchor path stays fixed. Native appends may reallocate Slots;
// prove the exact indexed slot/content afresh instead of comparing stale headers.
template<class R>bool LayoutCurrent(R& r,const Discovery& saved,const RowLayout& layout){
 const auto& c=saved.context;Context fresh;Failure why;
 if(!Observe(r,c.module,c.thread,fresh,why)||!fresh.listenHost||!Same(c,fresh)||layout.world!=c.world||layout.menu!=c.menu||!cfghosted::native::WorldOwned(r,c.controller,c.world))return false;
 const auto& graph=saved.graph;unsigned at=UINT32_MAX;std::uintptr_t type=0;
 for(unsigned i=0;i<graph.count;++i)if(graph.nodes[i].widget.object==layout.anchor)at=i;
 if(at>=graph.count||graph.nodes[at].parent>=graph.count||graph.nodes[graph.nodes[at].parent].widget.object!=layout.panel||
    graph.nodes[at].index+1!=layout.after||!r.Read(layout.anchor+cfgprobe::ObjectClass,type)||type!=layout.rowClass)return false;
 for(unsigned depth=0;depth<NodeLimit;++depth){
  if(at>=graph.count)return false;const auto& node=graph.nodes[at];Slot slot;
  if(!cfgpracticeobs::Clear(r,fresh.registry,node.widget.object,slot)||!(slot==node.widget))return false;
  std::uintptr_t value=0;
  if(node.parent==UINT32_MAX)return at==0&&cfgobserver::Same(r,c.menu+cfgpractice::disk::WidgetTree,node.tree)&&
      cfgobserver::Same(r,node.tree+cfgpractice::disk::RootWidget,node.widget.object);
  if(node.parent>=at)return false;const auto parent=graph.nodes[node.parent].widget.object;
  if(node.kind==1){if(!cfgobserver::Same(r,parent+cfgpractice::disk::WidgetTree,node.tree)||
      !cfgobserver::Same(r,node.tree+cfgpractice::disk::RootWidget,node.widget.object))return false;
  }else if(node.kind==2){cfgpracticeobs::WidgetNode::List first{},again{};
   if(!cfgpracticeobs::Typed(r,fresh.registry,c.module,parent,cfgpractice::disk::PanelWidgetReflection)||
      !r.Read(parent+cfgpractice::disk::Slots,first)||!first.Sane()||node.index>=static_cast<unsigned>(first.count)||
      !r.Read(first.data+8ULL*node.index,value)||value!=node.slot||
      !cfgpracticeobs::Typed(r,fresh.registry,c.module,node.slot,cfgpractice::disk::PanelSlotReflection)||
      !cfgobserver::Same(r,node.slot+cfgpractice::disk::Content,node.widget.object)||
      !r.Read(parent+cfgpractice::disk::Slots,again)||!(again==first))return false;
  }else return false;
  at=node.parent;
 }
 return false;
}
}
