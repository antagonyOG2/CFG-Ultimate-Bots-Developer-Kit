#pragma once
#include "../RuntimeProbe/ObjectivePanelBindings.hpp"
#include "../RuntimeProbe/MenuPanelChildrenObservation.hpp"
#include "../RuntimeProbe/MenuScriptObservation.hpp"
#include "../RuntimeProbe/NativeMapSelectionObservation.hpp"
#include "DefeatedOutroProperty.hpp"
namespace cfgobjectivepanel {
enum class Kind {Other,Car2,Car4,Radio,GrendelRadio,Phone,GrendelPhone,Boat,Bridge};
struct Child {
 std::uintptr_t slot=0,widget=0,type=0;
 cfgprobe::lifetime::SlotObservation slotIdentity{},widgetIdentity{};Kind kind=Kind::Other;
};
struct Snapshot {
 std::uintptr_t widget=0,type=0,panel=0,panelType=0;
 cfgprobe::lifetime::SlotObservation rootIdentity{},panelIdentity{};
 cfgmenu::panelcapture::Array slots{};std::array<Child,64> children{};
 bool rootedTopology=false;
};
// One cold schema advances <=6 fields per callback, including function fields.
// Never restart a whole class walk merely because the shared read budget ended.
struct FieldSearch {
 std::uintptr_t root=0,owner=0,head=0,cursor=0;unsigned owners=0,count=0,matches=0;
 std::array<std::uintptr_t,32> parents{};std::array<std::uintptr_t,256> visited{};
 cfgreflection::Field previous{},match{};bool opened=false,ready=false,refused=false;
};
struct Binding {
 std::uintptr_t ownerType=0,carType=0;cfgreflection::Property panel{},fourDoor{};
 FieldSearch panelSearch{},carSearch{};bool pending=false;unsigned failure=0;
 // Read-only short-circuit evidence; never grants repair authority.
 unsigned stage=0,childIndex=0;Snapshot evidence{};
};
inline bool Same(const Snapshot& a,const Snapshot& b){
 if(a.slots.count<0||a.slots.count>64||b.slots.count<0||b.slots.count>64)return false;
 if(a.rootedTopology!=b.rootedTopology||a.widget!=b.widget||a.type!=b.type||a.panel!=b.panel||a.panelType!=b.panelType||
  !(a.rootIdentity==b.rootIdentity)||!(a.panelIdentity==b.panelIdentity)||!(a.slots==b.slots))return false;
 for(int i=0;i<a.slots.count;++i){const auto& x=a.children[static_cast<unsigned>(i)];const auto& y=b.children[static_cast<unsigned>(i)];
  if(x.slot!=y.slot||x.widget!=y.widget||x.type!=y.type||x.kind!=y.kind||!(x.slotIdentity==y.slotIdentity)||!(x.widgetIdentity==y.widgetIdentity))return false;}
 return true;
}
template<class R> bool PropertyCurrent(R& r,std::uintptr_t module,const cfgreflection::Property& p){
 cfgreflection::Observation<R> reflection(r,module);cfgreflection::Field f;cfgreflection::Property q;
 return reflection.ReadField(p.field.object,p.field.owner,f)&&f.type==p.field.type&&f.name==p.field.name&&f.next==p.field.next&&
  reflection.ReadProperty(f,q)&&q.offset==p.offset&&q.flags==p.flags&&q.dimension==p.dimension&&q.elementBytes==p.elementBytes;
}
template<class R> bool WarmProperty(R& r,std::uintptr_t module,std::uintptr_t type,std::u16string_view name,std::u16string_view kind,int bytes,
 FieldSearch& search,cfgreflection::Property& p){
 if(search.root!=type){search={};search.root=search.owner=type;}
 if(search.refused)return false;if(search.ready)return true;
 cfgreflection::Observation<R> reflection(r,module);std::uintptr_t fresh=0;
 if(!search.opened){
  if(!search.owner||search.owners==search.parents.size()){search.refused=true;return false;}
  for(unsigned i=0;i<search.owners;++i)if(search.parents[i]==search.owner){search.refused=true;return false;}
  if(!r.Read(search.owner+cfgreflection::disk::Children,fresh))return false;
  search.parents[search.owners++]=search.owner;search.head=search.cursor=fresh;search.previous={};search.matches=0;search.opened=true;
 }
 if(!r.Read(search.owner+cfgreflection::disk::Children,fresh))return false;
 if(fresh!=search.head){search.refused=true;return false;}
 if(search.previous.object){cfgreflection::Field field;
  if(!reflection.ReadField(search.previous.object,search.owner,field))return false;
  if(field.type!=search.previous.type||!(field.name==search.previous.name)||field.next!=search.cursor){search.refused=true;return false;}
 }
 for(unsigned step=0;search.cursor&&step<6;++step){
  if(search.count==search.visited.size()){search.refused=true;return false;}
  for(unsigned i=0;i<search.count;++i)if(search.visited[i]==search.cursor){search.refused=true;return false;}
  cfgreflection::Field field;if(!reflection.ReadField(search.cursor,search.owner,field))return false;
  search.visited[search.count++]=search.cursor;search.previous=field;search.cursor=field.next;
  if(field.label.View()==name){search.match=field;++search.matches;}
 }
 if(search.cursor)return false;
 if(search.matches){
  if(search.matches!=1||search.match.kind.View()!=kind){search.refused=true;return false;}
  cfgreflection::Property property;if(!reflection.ReadProperty(search.match,property))return false;
  if(property.dimension!=1||property.elementBytes!=bytes||(bytes==8&&property.offset%8)){search.refused=true;return false;}
  if(!r.Read(search.owner+cfgreflection::disk::Children,fresh))return false;
  if(fresh!=search.head){search.refused=true;return false;}
  p=property;search.ready=true;return true;
 }
 if(!r.Read(search.owner+cfgprobe::ClassSuper,fresh))return false;
 if(!fresh){search.refused=true;return false;}search.owner=fresh;search.opened=false;return false;
}
template<class R> bool TypeName(R& r,std::uintptr_t module,std::uintptr_t type,std::u16string_view wanted){
 cfgnames::Observation<R> names(r,module);cfgnames::Text label;return names.Object(type,label)&&label.View()==wanted;
}
template<class R> bool NativeClass(R& r,std::uintptr_t module,std::uintptr_t cache,std::uintptr_t ctor,std::u16string_view name,std::uintptr_t& out){
 std::uintptr_t second=0;return r.Read(module+cache,out)&&r.Read(module+ctor,second)&&cfgprobe::MatchingClassCache(out,second)&&TypeName(r,module,out,name);
}
template<class R,std::size_t N> bool Code(R& r,std::uintptr_t at,const std::array<unsigned char,N>& expected){
 std::array<unsigned char,N> actual{};return r.ReadBytes(at,actual.data(),N)&&actual==expected;
}
template<class R> bool CarKind(R& r,std::uintptr_t module,const Child& child,Binding& binding,Kind& kind){
 if(binding.carType!=child.type){cfgreflection::Property p;
  if(!WarmProperty(r,module,child.type,u"Is4DoorCar",u"BoolProperty",1,binding.carSearch,p)){
   binding.pending=!binding.carSearch.refused;binding.failure=binding.carSearch.refused?2u:0u;return false;}
  if(binding.carSearch.ready&&!p.field.object)p=binding.fourDoor;
  binding.carType=child.type;binding.fourDoor=p;
 }
 const auto& p=binding.fourDoor;std::array<std::uint8_t,4> layout{},again{};std::uint8_t value=0,last=0;
 if(!Code(r,module+cfgdeathend::BoolAssignProof,cfgdeathend::BoolAssignCode)||!PropertyCurrent(r,module,p)||
  !r.Read(p.field.object+0x70,layout)||layout[0]!=1||layout[1]!=0||!layout[2]||(layout[2]&(layout[2]-1))||
  (layout[3]!=0xff&&layout[3]!=layout[2])||!r.Read(child.widget+static_cast<unsigned>(p.offset),value)||
  !r.Read(p.field.object+0x70,again)||again!=layout||!PropertyCurrent(r,module,p)||
  !r.Read(child.widget+static_cast<unsigned>(p.offset),last)||value!=last)return false;
 kind=(value&layout[2])?Kind::Car4:Kind::Car2;return true;
}
// Metadata/panel topology only. Caller separately proves exact offline world,
// owner thread and native GetOwningPlayer result. No native calls or writes.
// Pass a retained Binding: cold schema advances <=6 fields/call and reports
// binding.pending until ready. No retained binding means only tiny schemas
// can complete. A failed bounded read never approves repair.
template<class R> bool ReadPanel(R& r,std::uintptr_t module,std::uintptr_t widget,std::uintptr_t ownerType,Snapshot& out,Binding* retained=nullptr){
 out={};Binding local;auto& binding=retained?*retained:local;binding.pending=false;binding.failure=0;binding.childIndex=0;binding.evidence={};Snapshot s;s.widget=widget;s.type=ownerType;
 // RAII retains partially observed values on every refusal, without another
 // read or weakening positive-serial/topology checks. Output remains empty.
 struct Evidence {Binding& b;Snapshot& s;~Evidence(){b.evidence=s;}} evidence{binding,s};
 binding.stage=1;
 std::uintptr_t actual=0,panelClass=0,slotClass=0,widgetClass=0;cfgprobe::lifetime::ArrayHeader registry;
 if(!TypeName(r,module,ownerType,u"ObjectivesWidget_C")||!r.Read(widget+cfgprobe::ObjectClass,actual)||actual!=ownerType||
  !cfgprobe::lifetime::ReadArrayHeader(r,module+cfgprobe::lifetime::ObjectArrayRva,registry)||
  !cfgprobe::lifetime::ReadSlot(r,registry,widget,s.rootIdentity)||!s.rootIdentity.HasAssignedSerial()||
  !NativeClass(r,module,disk::PanelWidget,disk::PanelWidgetConstructor,u"PanelWidget",panelClass)||
  !NativeClass(r,module,disk::PanelSlot,disk::PanelSlotConstructor,u"PanelSlot",slotClass)||
  !NativeClass(r,module,disk::Widget,disk::WidgetConstructor,u"Widget",widgetClass)||
  !Code(r,module+disk::NativeCountRva,disk::NativeCountCode)||!Code(r,module+disk::NativeChildRva,disk::NativeChildCode)||
  !Code(r,module+disk::NativeParentRva,disk::NativeParentCode))return false;
 binding.stage=2;
 if(binding.ownerType!=ownerType){cfgreflection::Property p;
  if(!WarmProperty(r,module,ownerType,u"ObjectivesVerticleBox",u"ObjectProperty",8,binding.panelSearch,p)){
   binding.pending=!binding.panelSearch.refused;binding.failure=binding.panelSearch.refused?1u:0u;return false;}
  if(binding.panelSearch.ready&&!p.field.object)p=binding.panel;
  binding.ownerType=ownerType;binding.panel=p;
 }
 const auto& property=binding.panel;
 binding.stage=3;
 if(!PropertyCurrent(r,module,property)||!r.Read(widget+static_cast<unsigned>(property.offset),s.panel)||!cfgprobe::ObjectPointer(s.panel)||
  !r.Read(s.panel+cfgprobe::ObjectClass,s.panelType)||!cfgmenu::maps::Subclass(r,s.panelType,panelClass))return false;
 binding.stage=4;
 // UMG does not allocate weak serials for every native-owned subwidget.
 // Borrow this panel ONLY through the typed reflected strong property of the
 // positive, native-local-player-owned root. Its slot/index/type/property,
 // full child ownership graph and registry are repeated below and again by
 // the host immediately before a finder call. Never invent/allocate a serial.
 if(!cfgprobe::lifetime::ReadSlot(r,registry,s.panel,s.panelIdentity))return false;
 binding.stage=5;
 if(!r.Read(s.panel+disk::PanelSlots,s.slots)||s.slots.count<0||s.slots.count>64||s.slots.capacity<s.slots.count||s.slots.capacity>1048576||
  (s.slots.count&&!cfgprobe::ObjectPointer(s.slots.data)))return false;
 cfgnames::Observation<R> names(r,module);
 for(int i=0;i<s.slots.count;++i){auto& child=s.children[static_cast<unsigned>(i)];std::uintptr_t st=0,parent=0,back=0;cfgnames::Text name;
  binding.childIndex=static_cast<unsigned>(i);binding.stage=6;
  if(!r.Read(s.slots.data+8ULL*i,child.slot)||!cfgprobe::ObjectPointer(child.slot)||!r.Read(child.slot+cfgprobe::ObjectClass,st)||
   !cfgmenu::maps::Subclass(r,st,slotClass)||!r.Read(child.slot+disk::SlotParent,parent)||parent!=s.panel||
   !r.Read(child.slot+disk::SlotContent,child.widget)||!cfgprobe::ObjectPointer(child.widget)||
   !r.Read(child.widget+cfgprobe::ObjectClass,child.type)||!cfgmenu::maps::Subclass(r,child.type,widgetClass)||
   !r.Read(child.widget+disk::WidgetSlot,back)||back!=child.slot)return false;
  binding.stage=7;
  if(!cfgprobe::lifetime::ReadSlot(r,registry,child.slot,child.slotIdentity))return false;
  binding.stage=8;
  if(!cfgprobe::lifetime::ReadSlot(r,registry,child.widget,child.widgetIdentity)||!names.Object(child.type,name))return false;
  binding.stage=9;
  for(int j=0;j<i;++j)if(s.children[static_cast<unsigned>(j)].slot==child.slot||s.children[static_cast<unsigned>(j)].widget==child.widget)return false;
  if(name.View()==u"Objective_Car_Widget_C"){if(!CarKind(r,module,child,binding,child.kind))return false;}
  else if(name.View()==u"Objective_Radio_Widget_C")child.kind=Kind::Radio;
  else if(name.View()==u"Objective_GrendelRadio_Widget_C")child.kind=Kind::GrendelRadio;
  else if(name.View()==u"Objective_Police_Widget_C")child.kind=Kind::Phone;
  else if(name.View()==u"Objective_GrendelPolice_Widget_C")child.kind=Kind::GrendelPhone;
  else if(name.View()==u"Objective_Boat_Widget_C")child.kind=Kind::Boat;
  else if(name.View()==u"Objective_GrendelBridgeRepairWidget_C")child.kind=Kind::Bridge;
 }
 cfgmenu::panelcapture::Array finalSlots;cfgprobe::lifetime::SlotObservation identity;cfgprobe::lifetime::ArrayHeader finalRegistry;
 binding.stage=10;
 if(!PropertyCurrent(r,module,property)||!r.Read(widget+static_cast<unsigned>(property.offset),actual)||actual!=s.panel||
  !r.Read(widget+cfgprobe::ObjectClass,actual)||actual!=ownerType||!r.Read(s.panel+cfgprobe::ObjectClass,actual)||actual!=s.panelType||
  !r.Read(s.panel+disk::PanelSlots,finalSlots)||!(finalSlots==s.slots)||
  !cfgprobe::lifetime::ReadArrayHeader(r,module+cfgprobe::lifetime::ObjectArrayRva,finalRegistry)||!(finalRegistry==registry)||
  !cfgprobe::lifetime::ReadSlot(r,registry,widget,identity)||!(identity==s.rootIdentity)||
  !cfgprobe::lifetime::ReadSlot(r,registry,s.panel,identity)||!(identity==s.panelIdentity))return false;
 for(int i=0;i<s.slots.count;++i){const auto& child=s.children[static_cast<unsigned>(i)];
  binding.stage=11;binding.childIndex=static_cast<unsigned>(i);
  if(!r.Read(s.slots.data+8ULL*i,actual)||actual!=child.slot||!r.Read(child.slot+disk::SlotParent,actual)||actual!=s.panel||
   !r.Read(child.slot+disk::SlotContent,actual)||actual!=child.widget||!r.Read(child.widget+disk::WidgetSlot,actual)||actual!=child.slot||
   !r.Read(child.widget+cfgprobe::ObjectClass,actual)||actual!=child.type||
   !cfgprobe::lifetime::ReadSlot(r,registry,child.slot,identity)||!(identity==child.slotIdentity)||
   !cfgprobe::lifetime::ReadSlot(r,registry,child.widget,identity)||!(identity==child.widgetIdentity))return false;
 }
 binding.stage=12;s.rootedTopology=true;out=s;return true;
}
}
