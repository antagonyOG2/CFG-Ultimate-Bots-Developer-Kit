#define CFG_J214_WALL_TEST_MAIN ExistingWallChecks
#include "J214WallRuntimeTests.cpp"
#include "CounselorPracticeObservation.hpp"
struct WidgetFixture:WallFixture {
 static constexpr std::uintptr_t Root=0x2800000,Child=0x2810000,Tree=0x2820000,SlotObject=0x2830000,
  Slots=0x2840000,WidgetType=0x2850000,PanelType=0x2860000,SlotType=0x2870000,TreeType=0x2880000;
 cfgsweater::HudEvidence hud{};cfgpracticeobs::WidgetGraph graph{};
 WidgetFixture(){using namespace cfgprobe;namespace d=cfgpractice::disk;
  r.Set(Module+lifetime::ObjectArrayRva+28,int{24});Slot(Root,16,0);Slot(Child,17,0);Slot(Tree,18,0);Slot(SlotObject,19,0);
  hud.widget={0x2890000,20,120};Slot(hud.widget.object,20,120);graph.hudWidget=hud.widget;
  r.Set(Module+d::WidgetReflection,WidgetType);r.Set(Module+d::WidgetConstructor,WidgetType);
  r.Set(Module+d::PanelWidgetReflection,PanelType);r.Set(Module+d::PanelWidgetConstructor,PanelType);
  r.Set(Module+d::PanelSlotReflection,SlotType);r.Set(Module+d::PanelSlotConstructor,SlotType);
  r.Set(Module+d::WidgetTreeReflection,TreeType);r.Set(Module+d::WidgetTreeConstructor,TreeType);
  r.Set(PanelType+ClassSuper,WidgetType);r.Set(WidgetType+ClassSuper,std::uintptr_t{});r.Set(SlotType+ClassSuper,std::uintptr_t{});r.Set(TreeType+ClassSuper,std::uintptr_t{});
  r.Set(Root+ObjectClass,PanelType);r.Set(Child+ObjectClass,WidgetType);r.Set(Tree+ObjectClass,TreeType);r.Set(SlotObject+ObjectClass,SlotType);
  r.Set(hud.widget.object+d::WidgetTree,Tree);r.Set(Tree+d::RootWidget,Root);
  cfgpracticeobs::WidgetNode::List list{Slots,1,1};r.Set(Root+d::Slots,list);r.Set(Slots,SlotObject);r.Set(SlotObject+d::Content,Child);
  graph.count=2;graph.nodes[0].widget={Root,16,0};graph.nodes[0].tree=Tree;
  graph.nodes[1].widget={Child,17,0};graph.nodes[1].parent=0;graph.nodes[1].kind=2;graph.nodes[1].slot=SlotObject;graph.nodes[1].list=list;
 }
};
#ifndef CFG_J215_HUD_TEST_MAIN
#define CFG_J215_HUD_TEST_MAIN main
#endif
int CFG_J215_HUD_TEST_MAIN(){unsigned checks=0,failures=0;const auto check=[&](bool good,const char* text){++checks;if(!good){++failures;std::printf("FAIL %s\n",text);}};
 for(unsigned fault=0;fault<10;++fault){WidgetFixture f;namespace d=cfgpractice::disk;auto& memory=f.r;
  cfgprobe::lifetime::ArrayHeader array;check(cfgprobe::lifetime::ReadArrayHeader(memory,f.Module+cfgprobe::lifetime::ObjectArrayRva,array),"registry fixture");
  if(fault==1)memory.Set(f.Tree+d::RootWidget,f.Child);
  if(fault==2)memory.Set(f.Slots,f.Tree);
  if(fault==3)memory.Set(f.SlotObject+d::Content,f.Root);
  if(fault==4)memory.Set(f.Root+d::Slots,cfgpracticeobs::WidgetNode::List{f.Slots,0,1});
  if(fault==5)memory.Set(f.Child+8,std::uint32_t{0x20});
  if(fault==6){f.graph.nodes[1].widget.serial=1;memory.Set(f.Registry+24*17+16,int{2});}
  if(fault==7)f.graph.nodes[1].parent=1;
  if(fault==8)memory.Set(f.hud.widget.object+d::WidgetTree,f.Slots);
  if(fault==9)memory.Set(f.Child+cfgprobe::ObjectClass,f.VolumeType);
  check(cfgpracticeobs::NodeCurrent(memory,f.Module,array,f.hud,f.graph,1)==(fault==0),"actual strong tree/slot/child path; serial0 borrowed only under repeated chain");
 }
 std::printf("J215 widget-tree path/identity/foreign mutation: %u checks, %u failures; no game access.\n",checks,failures);return failures?1:0;
}
