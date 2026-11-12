#define main ObjectivePanelFixtureMain
#include "SetupWidgetObservationTests.cpp"
#undef main
#include "ObjectivePanelObservation.hpp"
struct PanelReader {
 OwnedByteReader& r;unsigned reads=0;bool exhausted=false;
 template<class T>bool Read(std::uintptr_t at,T& value){if(reads>=1536){exhausted=true;return false;}++reads;return r.Read(at,value);}
 bool ReadBytes(std::uintptr_t at,void* out,std::size_t bytes){if(reads>=1536){exhausted=true;return false;}++reads;auto* p=static_cast<unsigned char*>(out);for(std::size_t i=0;i<bytes;++i)if(!r.Read(at+i,p[i]))return false;return true;}
};
struct PanelFixture:MenuFixture {
 static constexpr std::uintptr_t Registry=0xA00000,Panel=0xA10000,PanelType=0xA20000,Slot=0xA30000,SlotType=0xA40000,Child=0xA50000,ChildType=0xA60000,WidgetType=0xA70000,List=0xA80000,BoolType=0xA90000,BoolField=0xAA0000;
 PanelReader reader{r};cfgobjectivepanel::Binding binding;
 void Identity(std::uintptr_t object,unsigned i){r.Set(object+12,static_cast<int>(i));r.Set(Registry+i*24,object);r.Set(Registry+i*24+16,static_cast<int>(i+1));}
 PanelFixture(){using namespace cfgobjectivepanel;
  Name(Type,"ObjectivesWidget_C");Name(PanelType,"PanelWidget");Name(SlotType,"PanelSlot");Name(WidgetType,"Widget");Name(ChildType,"Objective_Car_Widget_C");Name(BoolType,"BoolProperty");
  r.Set(Base+disk::PanelWidget,PanelType);r.Set(Base+disk::PanelWidgetConstructor,PanelType);r.Set(Base+disk::PanelSlot,SlotType);r.Set(Base+disk::PanelSlotConstructor,SlotType);r.Set(Base+disk::Widget,WidgetType);r.Set(Base+disk::WidgetConstructor,WidgetType);
  r.Set(Base+disk::NativeCountRva,disk::NativeCountCode);r.Set(Base+disk::NativeChildRva,disk::NativeChildCode);r.Set(Base+disk::NativeParentRva,disk::NativeParentCode);
  r.Set(Base+cfgdeathend::BoolAssignProof,cfgdeathend::BoolAssignCode);
  r.Set(PanelType+cfgprobe::ClassSuper,WidgetType);r.Set(WidgetType+cfgprobe::ClassSuper,std::uintptr_t{});r.Set(SlotType+cfgprobe::ClassSuper,std::uintptr_t{});r.Set(ChildType+cfgprobe::ClassSuper,WidgetType);
  Field(Member,Type,ObjectProperty,"ObjectivesVerticleBox",0);Property(Member,8,0x180);r.Set(Widget+0x180,Panel);
  Field(BoolField,ChildType,BoolType,"Is4DoorCar",0);Property(BoolField,1,0x200);r.Set(ChildType+cfgreflection::disk::Children,BoolField);r.Set(BoolField+0x70,std::array<unsigned char,4>{1,0,4,4});r.Set(Child+0x200,std::uint8_t{4});
  r.Set(Panel+16,PanelType);r.Set(Slot+16,SlotType);r.Set(Child+16,ChildType);r.Set(Panel+disk::PanelSlots,cfgmenu::panelcapture::Array{List,1,1});r.Set(List,Slot);
  r.Set(Slot+disk::SlotParent,Panel);r.Set(Slot+disk::SlotContent,Child);r.Set(Child+disk::WidgetSlot,Slot);
  r.Set(Base+cfgprobe::lifetime::ObjectArrayRva+0x10,Registry);r.Set(Base+cfgprobe::lifetime::ObjectArrayRva+0x1c,4);
  Identity(Widget,0);Identity(Panel,1);Identity(Slot,2);Identity(Child,3);
 }
 bool Read(cfgobjectivepanel::Snapshot& s){return cfgobjectivepanel::ReadPanel(reader,Base,Widget,Type,s,&binding);}
};
int main(){using namespace cfgobjectivepanel;unsigned failures=0,peak=0;auto check=[&](bool v){if(!v)++failures;};
 for(unsigned fault=0;fault<12;++fault){PanelFixture f;Snapshot s;
  switch(fault){
  case 1:f.r.Set(f.Slot+disk::SlotParent,f.Widget);break;
  case 2:f.r.Set(f.Child+disk::WidgetSlot,f.Panel);break;
  case 3:f.r.Set(f.Registry+16,0);break;
  case 4:f.r.Set(f.Panel+disk::PanelSlots,cfgmenu::panelcapture::Array{f.List,65,65});break;
  case 5:f.r.Set(f.BoolField+0x70,std::array<unsigned char,4>{1,0,3,3});break;
  case 6:f.Property(f.Member,8,0x181);break;
  case 7:f.r.Set(f.Base+disk::NativeCountRva,std::uint8_t{});break;
  case 8:f.Name(f.BoolField,"bIsFourSeat");break;
  case 9:f.r.Set(f.PanelType+cfgprobe::ClassSuper,f.PanelType);break;
  case 10:f.r.changeAt=f.List;f.r.changeSlot=f.Child+disk::WidgetSlot;f.r.newPointer=f.Panel;break;
  case 11:f.r.Set(f.Slot+16,f.WidgetType);break;
  }
  check(f.Read(s)==(fault==0));if(!fault){check(s.children[0].kind==Kind::Car4);Snapshot next;f.reader.reads=128;check(f.Read(next)&&Same(s,next));peak=f.reader.reads;}
  else check(!s.widget);
 }
 {PanelFixture f;Snapshot s;f.r.Set(f.Child+0x200,std::uint8_t{0});check(f.Read(s)&&s.children[0].kind==Kind::Car2);}
 for(unsigned object=1;object<=3;++object){PanelFixture f;Snapshot s;
  f.r.Set(f.Registry+24*object+16,0);check(f.Read(s)&&s.rootedTopology);
  check(f.binding.stage==12);
  check(f.binding.evidence.panel==f.Panel&&!f.binding.pending);
 }
 {PanelFixture f;Snapshot s;f.r.Set(f.Registry+24+16,0);f.r.Set(f.Registry+48+16,0);f.r.Set(f.Registry+72+16,0);
  check(f.Read(s)&&s.rootedTopology&&s.panelIdentity.serial==0&&s.children[0].slotIdentity.serial==0);
  f.r.changeAt=f.List;f.r.changeSlot=f.Child+disk::WidgetSlot;f.r.newPointer=f.Panel;
  f.reader.reads=0;check(!f.Read(s)&&!s.widget);}
 {PanelFixture f;Snapshot s;f.r.Set(f.Child+disk::WidgetSlot,f.Panel);
  check(!f.Read(s)&&!s.widget&&f.binding.stage==6&&f.binding.evidence.children[0].widget==f.Child);}
 {PanelFixture f;Snapshot s;f.reader.reads=1536;
  check(!f.Read(s)&&!s.widget&&f.reader.exhausted&&f.binding.stage==1);}
 {PanelFixture f;Snapshot s;f.r.Set(f.Panel+disk::PanelSlots,cfgmenu::panelcapture::Array{});check(f.Read(s)&&s.slots.count==0);}
 {PanelFixture f;Snapshot s;f.r.Set(f.Base+cfgprobe::lifetime::ObjectArrayRva+0x1c,10);
  f.r.Set(f.Panel+disk::PanelSlots,cfgmenu::panelcapture::Array{f.List,4,4});
  for(unsigned i=1;i<4;++i){const std::uintptr_t slot=f.Slot+i*0x1000,child=f.Child+i*0x1000;
   f.r.Set(f.List+i*8,slot);f.r.Set(slot+16,f.SlotType);f.r.Set(slot+disk::SlotParent,f.Panel);f.r.Set(slot+disk::SlotContent,child);
   f.r.Set(child+16,f.ChildType);f.r.Set(child+disk::WidgetSlot,slot);f.r.Set(child+0x200,std::uint8_t{0});f.Identity(slot,2+i*2);f.Identity(child,3+i*2);
  }
  f.reader.reads=128;check(f.Read(s)&&s.slots.count==4);std::printf("Objective panel four-car cold+128 outer=%u/1536 reads\n",f.reader.reads);
 }
 {Snapshot a,b;a.slots.count=b.slots.count=65;check(!Same(a,b));}
 {PanelFixture f;Snapshot s;f.r.Set(f.Table+0x400,std::array<int,2>{300,1});
  // Real capture has roughly20 root functions and8 car helpers. Add60/16
  // other fields too: cold discovery must progress rather than restart forever.
  f.r.Set(f.Type+cfgreflection::disk::Children,std::uintptr_t{0xB00000});
  for(unsigned i=0;i<80;++i)f.Field(0xB00000+i*0x100,f.Type,i<20?f.FunctionClass:f.ObjectProperty,"UnrelatedRootField",i==79?f.Member:0xB00000+(i+1)*0x100);
  f.r.Set(f.ChildType+cfgreflection::disk::Children,std::uintptr_t{0xB10000});
  for(unsigned i=0;i<24;++i)f.Field(0xB10000+i*0x100,f.ChildType,i<8?f.FunctionClass:f.ObjectProperty,"UnrelatedCarField",i==23?f.BoolField:0xB10000+(i+1)*0x100);
  bool complete=false;unsigned turns=0,worst=0;
  for(;turns<40&&!complete;++turns){f.reader.reads=128;f.reader.exhausted=false;complete=f.Read(s);worst=(std::max)(worst,f.reader.reads);check(!f.reader.exhausted);if(!complete)check(f.binding.pending&&!f.binding.failure);}
  check(complete&&turns>10&&turns<40&&s.children[0].kind==Kind::Car4);
  std::printf("Objective production-shaped81/25-field cold scan: turns=%u peak=%u/1536 including128 outer\n",turns,worst);
 }
 std::printf("Objective panel observation: %u failures; warm one-car+128 outer=%u/1536 reads. No native calls or writes.\n",failures,peak);return failures?1:0;
}
