#define CFG_UNVERIFIED_RUNTIME_FIXTURE
#define main ObjectiveRepairAdapterSetupMain
#include "SetupWidgetObservationTests.cpp"
#undef main
#include "ObjectiveHudRepair.hpp"
#include <string>
struct AdapterReader {
 OwnedByteReader& r;unsigned reads=0;bool exhausted=false;
 template<class T>bool Read(std::uintptr_t at,T& out){if(reads>=1536){exhausted=true;return false;}++reads;return r.Read(at,out);}
 bool ReadBytes(std::uintptr_t at,void* out,std::size_t n){if(reads>=1536){exhausted=true;return false;}++reads;auto* p=static_cast<unsigned char*>(out);for(std::size_t i=0;i<n;++i)if(!r.Read(at+i,p[i]))return false;return true;}
};
struct AdapterFixture:MenuFixture {
 static constexpr std::uintptr_t Registry=0xA00000,Panel=0xA10000,PanelType=0xA20000,Slot=0xA30000,SlotType=0xA40000,Child=0xA50000,CarType=0xA60000,WidgetType=0xA70000,List=0xA80000,BoolType=0xA90000,BoolField=0xAA0000,RadioType=0xAB0000,PhoneType=0xAC0000,Generated=0xAD0000,BaseState=0xAE0000,Hunt=0xAF0000,Script=0xB00000,Dest=0xB10000,Source=0xB20000;
 AdapterReader reader{r};cfgobjectivepanel::Binding binding;cfgobjective::Patch patch;
 void Identity(std::uintptr_t object,unsigned i){r.Set(object+12,static_cast<int>(i));r.Set(Registry+i*24,object);r.Set(Registry+i*24+8,std::uint32_t{});r.Set(Registry+i*24+16,static_cast<int>(i+1));}
 AdapterFixture(unsigned spec=21){using namespace cfgobjectivepanel;
  Name(Type,"ObjectivesWidget_C");Name(PanelType,"PanelWidget");Name(SlotType,"PanelSlot");Name(WidgetType,"Widget");Name(CarType,"Objective_Car_Widget_C");Name(BoolType,"BoolProperty");Name(RadioType,"Objective_Radio_Widget_C");Name(PhoneType,"Objective_Police_Widget_C");Name(Generated,"WidgetBlueprintGeneratedClass");Name(BaseState,"SCGameState");Name(Hunt,"SCGameState_Hunt");Name(DonorType,"Class");
  r.Set(Base+disk::PanelWidget,PanelType);r.Set(Base+disk::PanelWidgetConstructor,PanelType);r.Set(Base+disk::PanelSlot,SlotType);r.Set(Base+disk::PanelSlotConstructor,SlotType);r.Set(Base+disk::Widget,WidgetType);r.Set(Base+disk::WidgetConstructor,WidgetType);
  r.Set(Base+disk::NativeCountRva,disk::NativeCountCode);r.Set(Base+disk::NativeChildRva,disk::NativeChildCode);r.Set(Base+disk::NativeParentRva,disk::NativeParentCode);r.Set(Base+cfgdeathend::BoolAssignProof,cfgdeathend::BoolAssignCode);
  r.Set(Type+16,Generated);r.Set(PanelType+cfgprobe::ClassSuper,WidgetType);r.Set(WidgetType+cfgprobe::ClassSuper,std::uintptr_t{});r.Set(SlotType+cfgprobe::ClassSuper,std::uintptr_t{});
  for(auto type:{CarType,RadioType,PhoneType})r.Set(type+cfgprobe::ClassSuper,WidgetType);
  Field(Member,Type,ObjectProperty,"ObjectivesVerticleBox",0);Property(Member,8,0x180);r.Set(Widget+0x180,Panel);
  Field(BoolField,CarType,BoolType,"Is4DoorCar",0);Property(BoolField,1,0x200);r.Set(CarType+cfgreflection::disk::Children,BoolField);r.Set(BoolField+0x70,std::array<unsigned char,4>{1,0,4,4});
  r.Set(Panel+16,PanelType);r.Set(Panel+disk::PanelSlots,cfgmenu::panelcapture::Array{List,4,4});
  r.Set(Base+cfgprobe::lifetime::ObjectArrayRva+0x10,Registry);r.Set(Base+cfgprobe::lifetime::ObjectArrayRva+0x1c,12);Identity(Widget,0);Identity(Panel,1);
  for(unsigned i=0;i<4;++i){const auto slot=Slot+i*0x1000,child=Child+i*0x1000;const auto type=i<2?CarType:(i==2?RadioType:PhoneType);
   r.Set(List+i*8,slot);r.Set(slot+16,SlotType);r.Set(slot+disk::SlotParent,Panel);r.Set(slot+disk::SlotContent,child);r.Set(child+16,type);r.Set(child+disk::WidgetSlot,slot);r.Set(child+0x200,std::uint8_t(i==1?4:0));Identity(slot,2+i*2);Identity(child,3+i*2);
  }
  r.Set(BaseState+16,DonorType);r.Set(Hunt+16,DonorType);r.Set(Hunt+cfgprobe::ClassSuper,BaseState);
  r.Set(Base+cfgcompat::cfg35::objects::SCGameStateReflection,BaseState);
  r.Set(Base+cfgcompat::cfg35::objects::SCGameStateConstructor,BaseState);
  r.Set(Base+0x03199662,std::uint8_t{});r.Set(Base+0x03198110,std::uint32_t{});r.Set(Base+cfgmenu::counselorpickerdisk::DynamicCastSlot,Base+cfgmenu::counselorpickerdisk::DynamicCastExecutor);r.Set(Base+cfgmenu::counselorpickerdisk::DynamicCastExecutor,cfgmenu::counselorpickerdisk::DynamicCastCode);
  const auto& s=cfgobjective::Specification(spec);std::string label;for(auto c:s.name)label+=static_cast<char>(c);Field(Start,Type,FunctionClass,label.c_str(),0);r.Set(Start+cfgmenu::startdisk::Executor,Base+cfgmenu::startdisk::ScriptExecutor);
  Field(Dest,Start,ObjectProperty,"CastDestination",Source);Property(Dest,8,0);Field(Source,Start,ObjectProperty,"CastSource",0);Property(Source,8,8);
  r.Set(Dest,Base+cfgmenu::counselorpickerdisk::ObjectPropertyVtable);r.Set(Source,Base+cfgmenu::counselorpickerdisk::ObjectPropertyVtable);
  patch.spec=spec;patch.owner=Type;patch.base=BaseState;patch.original=Hunt;patch.script={Script,static_cast<int>(s.bytes),static_cast<int>(s.bytes)};
  auto& b=patch.bytes;b[s.site-9]=0x76;std::memcpy(b.data()+s.site-8,&Dest,8);b[s.site]=0x0f;std::memcpy(b.data()+s.site+1,&Hunt,8);b[s.site+9]=0x76;std::memcpy(b.data()+s.site+10,&Source,8);b[s.site+18]=0x9e;
  auto installed=b;std::memcpy(installed.data()+s.site+1,&BaseState,8);r.Set(Script,installed);r.Set(Start+cfgmenu::scriptdisk::Script,patch.script);Identity(Type,10);Identity(Start,11);patch.ownerSlot={Type,10,11};patch.functionSlot={Start,11,12};
 }
 bool PanelRead(cfgobjectivepanel::Snapshot& s){return cfgobjectivepanel::ReadPanel(reader,Base,Widget,Type,s,&binding);}
};
// Exact production non-panel/non-Metadata read upper bound:
// ObjectiveScope = 11 + ObjectIs(32) =77, three times in Dispatch.
// ObjectiveWidgetOwner remainder: LookupSafe2 + registry2 + three ObjectSlot12
// + object class/default2 + UserWidget NativeClass(2+14) + ObjectIs66
// + vtable/getter/code3 + final class1 =104. Final LookupSafe2, dispatcher1,
// repeated zero-frame6, and eight reads reserved before native invocation.
constexpr unsigned OwnerReserve=3*77+104+2+1+6+8; //352 of1536; one getter + one dispatch of12 calls.
int main(){unsigned failures=0,peak=0;auto check=[&](bool yes){if(!yes)++failures;};
 for(unsigned spec:{20u,21u,22u,23u,24u}){AdapterFixture f(spec);cfgobjectivepanel::Snapshot first,fresh;
  check(f.PanelRead(first));f.reader.reads=OwnerReserve;f.reader.exhausted=false;
  check(f.PanelRead(fresh)&&cfgobjectivepanel::Same(first,fresh));const auto panel=f.reader.reads-OwnerReserve;
  check(cfgobjective::Metadata(f.reader,f.Base,f.patch,true));check(!f.reader.exhausted&&f.reader.reads<=1536);peak=(std::max)(peak,f.reader.reads);
  check(fresh.children[0].kind==cfgobjectivepanel::Kind::Car2&&fresh.children[1].kind==cfgobjectivepanel::Kind::Car4&&fresh.children[2].kind==cfgobjectivepanel::Kind::Radio&&fresh.children[3].kind==cfgobjectivepanel::Kind::Phone);
  std::printf("Objective repair finder%u: panel=%u helper=%u outer=%u total=%u/1536 reads,2/12 calls reserved\n",spec,panel,f.reader.reads-OwnerReserve-panel,OwnerReserve,f.reader.reads);
 }
 {AdapterFixture f;cfgobjectivepanel::Snapshot s;check(f.PanelRead(s));f.reader.reads=OwnerReserve;f.r.Set(f.Base+0x03198110,std::uint32_t{1});check(!cfgobjective::Metadata(f.reader,f.Base,f.patch,true));}
 {AdapterFixture f;cfgobjectivepanel::Snapshot s;check(f.PanelRead(s));f.reader.reads=1535;check(!f.PanelRead(s)&&f.reader.exhausted);}
 {AdapterFixture f;cfgobjectivepanel::Snapshot s;check(f.PanelRead(s));
  auto prior=f.patch;prior.ownerSlot.serial=0;prior.functionSlot.serial=0;
  // The actual late root owns this class; native weak identities are assigned
  // after the metadata survey. Reacquisition must retain full script proof.
  cfgobjective::Patch refreshed;f.reader.reads=0;
  check(!cfgobjective::Metadata(f.reader,f.Base,prior,true));f.reader.reads=0;
  check(cfgobjective::ReacquireRootedHelper(f.reader,f.Base,prior,s.rootIdentity,refreshed));
  check(refreshed.ownerSlot.serial==11&&refreshed.functionSlot.serial==12);
  for(unsigned fault=0;fault<4;++fault){AdapterFixture bad;auto expected=prior;cfgobjective::Patch rejected;cfgobjectivepanel::Snapshot root;
   check(bad.PanelRead(root));bad.reader.reads=0;
   if(fault==0)root.rootIdentity.serial=0;
   if(fault==1)expected.ownerSlot.serial=10;
   if(fault==2)bad.r.Set(bad.Widget+cfgprobe::ObjectClass,bad.Generated);
   if(fault==3)bad.r.Set(bad.Base+0x03198110,std::uint32_t{1});
   check(!cfgobjective::ReacquireRootedHelper(bad.reader,bad.Base,expected,root.rootIdentity,rejected));
  }
 }
 std::printf("ObjectiveHudRepairAdapterTests: %u failures, peak%u/1536. In-memory fixture; no native calls or game access.\n",failures,peak);return failures?1:0;
}
