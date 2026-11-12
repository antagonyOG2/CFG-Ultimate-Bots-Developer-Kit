#pragma once
#include "../RuntimeProbe/MenuScriptObservation.hpp"
#include "../RuntimeProbe/MenuClassCache.hpp"
#include "../RuntimeProbe/Compatibility/CfgBuildProfile.hpp"
#include "../RuntimeProbe/CounselorPickerCast.generated.hpp"
#include "../RuntimeProbe/SettingsStart.generated.hpp"
#include "PrimarySpawnObservation.hpp"
#include <cstring>
namespace cfgobjective {
// CFG capture CFG-Objectives-20261007-212342-477-2684.txt. These are exact
// owned Blueprint helpers, not a search for opcode-looking bytes in scripts.
struct Spec {std::u16string_view type,name;unsigned bytes,site;};
inline constexpr std::array<Spec,24> Specs{{
 {u"Objective_Radio_Widget_C",u"HunterSpawned",229,102},{u"Objective_Radio_Widget_C",u"HunterCalled",406,102},
 {u"Objective_Police_Widget_C",u"PoliceArrived",417,102},{u"Objective_Police_Widget_C",u"PoliceCalled",406,102},{u"Objective_Police_Widget_C",u"PhoneBoxRepaired",365,61},
 {u"Objective_GrendelRadio_Widget_C",u"HunterSpawned",229,102},{u"Objective_GrendelRadio_Widget_C",u"HunterCalled",406,102},
 {u"Objective_GrendelPolice_Widget_C",u"PoliceArrived",417,102},{u"Objective_GrendelPolice_Widget_C",u"PoliceCalled",406,102},{u"Objective_GrendelPolice_Widget_C",u"PhoneBoxRepaired",365,61},
 {u"Objective_GrendelBridgeRepairWidget_C",u"GetBridgeObjectives",360,61},
 {u"Objective_Boat_Widget_C",u"IsDisabled",1044,102},{u"Objective_Boat_Widget_C",u"IsStarted",386,102},{u"Objective_Boat_Widget_C",u"HasEscaped",229,102},{u"Objective_Boat_Widget_C",u"IsRepaired",415,102},
 {u"Objective_Car_Widget_C",u"IsDisabled",1080,102},{u"Objective_Car_Widget_C",u"IsStarted",404,102},{u"Objective_Car_Widget_C",u"HasEscaped",314,102},{u"Objective_Car_Widget_C",u"IsRepaired",433,102},
 {u"ObjectivesWidget_C",u"FindBridge",368,61},{u"ObjectivesWidget_C",u"FindBoat",368,61},{u"ObjectivesWidget_C",u"FindCBRadio",654,61},{u"ObjectivesWidget_C",u"FindPhone",654,61},{u"ObjectivesWidget_C",u"Find2DoorCar",402,61}
}};
// Kept separate to make accidental omission of the second car impossible.
inline constexpr Spec FourDoor{u"ObjectivesWidget_C",u"Find4DoorCar",402,61};
constexpr unsigned Count=25,MaxScript=1080,ScanBatch=512,MaxRegistry=1048576;
inline const Spec& Specification(unsigned i){return i<Specs.size()?Specs[i]:FourDoor;}
using Bytes=std::array<unsigned char,MaxScript>;
inline std::uintptr_t Pointer(const Bytes& b,unsigned at){std::uintptr_t p=0;std::memcpy(&p,b.data()+at,8);return p;}
inline bool Shape(const Spec& s,const cfgmenu::capture::ScriptHeader& h,const Bytes& b){
 return h.count==static_cast<int>(s.bytes)&&s.site>=9&&s.site+18<s.bytes&&
 b[s.site]==cfgmenu::counselorpickerdisk::DynamicCast&&b[s.site-9]==0x76&&b[s.site+9]==0x76&&b[s.site+18]==0x9E&&
 cfgprobe::ObjectPointer(Pointer(b,s.site-8))&&cfgprobe::ObjectPointer(Pointer(b,s.site+10))&&cfgprobe::ObjectPointer(Pointer(b,s.site+1));
}
struct Patch {
 unsigned spec=0;std::uintptr_t owner=0,base=0,original=0;
 cfgprobe::lifetime::SlotObservation ownerSlot{},functionSlot{};
 cfgmenu::capture::ScriptHeader script{};Bytes bytes{};bool owned=false,uncertain=false;
};
struct Report {unsigned scanned=0,processed=0,changed=0,alreadyBase=0,refused=0,restored=0,restoreRefused=0,deferred=0,lastReads=0,peakReads=0;
 unsigned processedMask=0,changedMask=0,baseMask=0,refusedMask=0;
 bool discoveryDone=false,retired=false,indeterminate=false,expired=false;};
// Two exact world/scope checks include repeated native ancestry (up to32
// levels). Reserve160 outer reads rather than assuming a shallow blueprint.
constexpr unsigned PhaseReads=1376,CommitReserve=600;
template<class Reader> class Budget {
 Reader& reader_;
public:
 unsigned reads=0;bool exhausted=false;
 explicit Budget(Reader& r):reader_(r){}
 unsigned Remaining()const{return PhaseReads-reads;}
 template<class T>bool Read(std::uintptr_t at,T& value){if(reads==PhaseReads){exhausted=true;return false;}++reads;return reader_.Read(at,value);}
 bool ReadBytes(std::uintptr_t at,void* out,std::size_t size){if(reads==PhaseReads){exhausted=true;return false;}++reads;return reader_.ReadBytes(at,out,size);}
};
template<class Reader> bool NativeBase(Reader& r,std::uintptr_t module,std::uintptr_t& base){
 std::uintptr_t a=0,b=0;cfgnames::Observation<Reader> names(r,module);cfgnames::Text n;
 return r.Read(module+cfgcompat::cfg35::objects::SCGameStateReflection,a)&&cfgprobe::ObjectPointer(a)&&
  r.Read(module+cfgcompat::cfg35::objects::SCGameStateConstructor,b)&&a==b&&names.Object(a,n)&&n.View()==u"SCGameState"&&(base=a)!=0;
}
template<class Reader> bool IsHunt(Reader& r,std::uintptr_t module,std::uintptr_t value,std::uintptr_t base){
 cfgnames::Observation<Reader> names(r,module);cfgnames::Text n;
 std::uintptr_t nativeType=0,huntType=0;
 if(!names.Object(value,n)||n.View()!=u"SCGameState_Hunt"||!r.Read(base+cfgprobe::ObjectClass,nativeType)||
    !r.Read(value+cfgprobe::ObjectClass,huntType)||!cfgprobe::ObjectPointer(nativeType)||huntType!=nativeType||
    !names.Object(nativeType,n)||n.View()!=u"Class")return false;
 std::array<std::uintptr_t,16> seen{};
 for(unsigned i=0;i<seen.size();++i){if(value==base)return true;for(unsigned j=0;j<i;++j)if(value==seen[j])return false;
 seen[i]=value;if(!cfgprobe::ObjectPointer(value)||!r.Read(value+cfgprobe::ClassSuper,value))return false;}return false;
}
template<class Reader> bool Pins(Reader& r,std::uintptr_t module){
 std::array<unsigned char,cfgmenu::counselorpickerdisk::DynamicCastCode.size()> code{};std::uintptr_t entry=0;
 return cfgprimary::LookupSafe(r,module)&&
 r.Read(module+cfgmenu::counselorpickerdisk::DynamicCastSlot,entry)&&entry==module+cfgmenu::counselorpickerdisk::DynamicCastExecutor&&
 r.ReadBytes(entry,code.data(),code.size())&&code==cfgmenu::counselorpickerdisk::DynamicCastCode;
}
template<class Reader> bool Metadata(Reader& r,std::uintptr_t module,const Patch& p,bool installed){
 using namespace cfgprobe;const auto& s=Specification(p.spec);cfgreflection::Observation<Reader> reflection(r,module);
 cfgnames::Observation<Reader> names(r,module);cfgnames::Text ownerName,kind;
 std::uintptr_t ownerType=0,executor=0,base=0;lifetime::ArrayHeader registry;lifetime::SlotObservation ownerSlot,functionSlot;
 cfgreflection::Field f;cfgmenu::capture::ScriptHeader script;Bytes bytes;
 if(!Pins(r,module)||!NativeBase(r,module,base)||base!=p.base||(p.original!=base&&!IsHunt(r,module,p.original,base))||
 !lifetime::ReadArrayHeader(r,module+lifetime::ObjectArrayRva,registry)||!lifetime::ReadSlot(r,registry,p.owner,ownerSlot)||!(ownerSlot==p.ownerSlot)||
 !lifetime::ReadSlot(r,registry,p.functionSlot.object,functionSlot)||!(functionSlot==p.functionSlot)||
 !names.Object(p.owner,ownerName)||ownerName.View()!=s.type||!r.Read(p.owner+ObjectClass,ownerType)||!names.Object(ownerType,kind)||kind.View()!=u"WidgetBlueprintGeneratedClass"||
 !reflection.ReadField(p.functionSlot.object,p.owner,f)||f.label.View()!=s.name||f.kind.View()!=u"Function"||
 !r.Read(f.object+cfgmenu::startdisk::Executor,executor)||executor!=module+cfgmenu::startdisk::ScriptExecutor||
 !cfgmenu::capture::Script(r,f.object,script,bytes)||!(script==p.script)||!Shape(s,script,bytes))return false;
 Bytes expected=p.bytes;if(installed)std::memcpy(expected.data()+s.site+1,&p.base,8);if(bytes!=expected)return false;
 // Captured local-property operands are owned by this exact UFunction, never
 // arbitrary embedded addresses. No Array/FText or native argument assumptions.
 for(unsigned at:{s.site-8,s.site+10}){cfgreflection::Field property;cfgreflection::Property schema;std::uintptr_t vtable=0;
  if(!reflection.ReadField(Pointer(bytes,at),f.object,property)||!reflection.ReadProperty(property,schema)||property.kind.View()!=u"ObjectProperty"||
    schema.dimension!=1||schema.elementBytes!=8||!r.Read(property.object,vtable)||vtable!=module+cfgmenu::counselorpickerdisk::ObjectPropertyVtable)return false;}
 lifetime::ArrayHeader again;lifetime::SlotObservation os,fs;
 return lifetime::ReadArrayHeader(r,module+lifetime::ObjectArrayRva,again)&&again==registry&&
 lifetime::ReadSlot(r,again,p.owner,os)&&os==ownerSlot&&lifetime::ReadSlot(r,again,f.object,fs)&&fs==functionSlot;
}
// Called only for a finder of the exact positively identified owned root.
// A native zero->assigned serial transition is reacquired, never fabricated.
template<class Reader>bool ReacquireRootedHelper(Reader& r,std::uintptr_t module,const Patch& prior,
    cfgprobe::lifetime::SlotObservation rootedWidget,Patch& out){
 out={};if(!rootedWidget.HasAssignedSerial()||prior.spec<19||prior.spec>=Count)return false;
 cfgprobe::lifetime::ArrayHeader registry;cfgprobe::lifetime::SlotObservation root{},owner{},function{};
 std::uintptr_t type=0;
 if(!cfgprobe::lifetime::ReadArrayHeader(r,module+cfgprobe::lifetime::ObjectArrayRva,registry)||
    !cfgspawn::ObjectSlot(r,registry,rootedWidget.object,root)||!(root==rootedWidget)||
    !r.Read(root.object+cfgprobe::ObjectClass,type)||type!=prior.owner||
    !cfgspawn::ObjectSlot(r,registry,prior.owner,owner)||
    !cfgspawn::ObjectSlot(r,registry,prior.functionSlot.object,function)||
    owner.index!=prior.ownerSlot.index||function.index!=prior.functionSlot.index||
    (prior.ownerSlot.serial&&owner.serial!=prior.ownerSlot.serial)||
    (prior.functionSlot.serial&&function.serial!=prior.functionSlot.serial)||
    (owner==prior.ownerSlot&&function==prior.functionSlot))return false;
 Patch refreshed=prior;refreshed.ownerSlot=owner;refreshed.functionSlot=function;
 if(!Metadata(r,module,refreshed,true)||
    !cfgspawn::ObjectSlot(r,registry,root.object,root)||!(root==rootedWidget)||
    !r.Read(root.object+cfgprobe::ObjectClass,type)||type!=refreshed.owner)return false;
 out=refreshed;return true;
}
// Metadata-only retirement adapter. These zero serials describe class/function
// metadata observed before native weak-serial assignment, not a living actor.
// Reacquire ONLY a positive assignment at the same object/index, with the full
// captured installed script/property/native-owner proof repeated. Positive
// serial replacement, changed script, dead slots and uncertain writes refuse.
template<class Reader>bool ReacquireRetiringMetadata(Reader& r,std::uintptr_t module,const Patch& prior,Patch& out){
 out={};if(!prior.owned||prior.uncertain)return false;
 cfgprobe::lifetime::ArrayHeader registry;cfgprobe::lifetime::SlotObservation owner,function;
 if(!cfgprobe::lifetime::ReadArrayHeader(r,module+cfgprobe::lifetime::ObjectArrayRva,registry)||
    !cfgprobe::lifetime::ReadSlot(r,registry,prior.owner,owner)||
    !cfgprobe::lifetime::ReadSlot(r,registry,prior.functionSlot.object,function)||
    owner.index!=prior.ownerSlot.index||function.index!=prior.functionSlot.index||
    (prior.ownerSlot.serial?owner.serial!=prior.ownerSlot.serial:owner.serial<0)||
    (prior.functionSlot.serial?function.serial!=prior.functionSlot.serial:function.serial<0)||
    (owner==prior.ownerSlot&&function==prior.functionSlot))return false;
 Patch fresh=prior;fresh.ownerSlot=owner;fresh.functionSlot=function;
 if(!Metadata(r,module,fresh,true))return false;
 out=fresh;return true;
}
// Host.Current(): exact session/offline gate + native owner-thread. Host.Exchange
// performs an expected-value, writable-region, eight-byte write/readback; operands
// are unaligned, so InterlockedCompareExchangePointer MUST NOT be used here.
// Exchange returns true only after exact readback. Failure may be partial: retained
// uncertain records forbid restart/unload. No blind rollback or foreign overwrite.
class Runtime {
 std::array<std::uintptr_t,Count> owners_{};std::array<bool,Count> done_{};
 std::array<std::uintptr_t,Count> functions_{},nextField_{};
 std::array<unsigned,Count> walked_{};
 std::array<bool,Count> searchStarted_{},restoreAttempted_{},restoreRefresh_{};
 std::array<Patch,Count> patches_{};cfgmenu::capture::MenuClassCache cache_;
 std::array<Patch,Count> ready_{};
 std::array<cfgprobe::lifetime::SlotObservation,8> instances_{};unsigned instancesCount_=0;
 std::array<unsigned,8> eventRetries_{};std::array<std::uint64_t,8> eventNext_{};
 std::array<std::uintptr_t,ScanBatch*3> registryPage_{};
 unsigned cursor_=0,limit_=0;std::uint64_t started_=0,nextProbe_=0;bool startedClock_=false,secondPass_=false;Report report_{};
 std::array<std::uintptr_t,128> eventTypes_{};unsigned eventTypesCount_=0;
 std::uint64_t eventHelpersUntil_=0;
 bool HelpersPending()const noexcept{for(unsigned i=0;i<Count;++i)if(owners_[i]&&!done_[i])return true;return false;}
 bool MissingOwners()const noexcept{for(auto owner:owners_)if(!owner)return true;return false;}
public:
 const Report& Status()const noexcept{return report_;}
 std::uintptr_t RootClass()const noexcept{return owners_[21];}
 bool NeedsEventOwner()const noexcept{return !report_.retired&&!report_.indeterminate&&MissingOwners();}
 bool EventTypeSeen(std::uintptr_t type)const noexcept{for(unsigned i=0;i<eventTypesCount_;++i)if(eventTypes_[i]==type)return true;return false;}
 // A genuine completed Blueprint event supplies an exact class identity.
 // This seeds ONLY captured objective classes, never restarts the registry
 // survey. Negative classification is finite and cached, not repeated each tick.
 template<class Reader,class Host>bool EventOwner(Reader& r,std::uintptr_t module,
     std::uintptr_t object,std::uintptr_t function,std::uint64_t now,Host& host){
  if(!NeedsEventOwner()||now>UINT64_MAX-20000)return false;
  const auto same=[&](std::uintptr_t at,std::uintptr_t value){std::uintptr_t observed=0;return r.Read(at,observed)&&observed==value;};
  std::uintptr_t type=0,kind=0;
  if(!r.Read(object+cfgprobe::ObjectClass,type)||EventTypeSeen(type)||
     !same(function+0x20,type))return false;
  cfgnames::Observation<Reader> names(r,module);cfgnames::Text label,kindName;
  if(!r.Read(type+cfgprobe::ObjectClass,kind)||!names.Object(kind,kindName)||
     !names.Object(type,label))return false;
  bool wanted=false;for(unsigned i=0;i<Count;++i)if(label.View()==Specification(i).type)wanted=true;
  if(!wanted||kindName.View()!=u"WidgetBlueprintGeneratedClass"){
   if(eventTypesCount_<eventTypes_.size())eventTypes_[eventTypesCount_++]=type;return false;
  }
  cfgprobe::lifetime::ArrayHeader registry,fresh;cfgprobe::lifetime::SlotObservation before,after;
  cfgnames::Text again;
  if(!host.Current()||!Pins(r,module)||
     !cfgprobe::lifetime::ReadArrayHeader(r,module+cfgprobe::lifetime::ObjectArrayRva,registry)||
     !cfgprobe::lifetime::ReadSlot(r,registry,type,before)||
     !names.Object(type,again)||again.View()!=label.View()||
     !same(type+cfgprobe::ObjectClass,kind)||
     !same(object+cfgprobe::ObjectClass,type)||!same(function+0x20,type)||
     !cfgprobe::lifetime::ReadSlot(r,registry,type,after)||!(before==after)||
     !cfgprobe::lifetime::ReadArrayHeader(r,module+cfgprobe::lifetime::ObjectArrayRva,fresh)||!(fresh==registry)||!host.Current())return false;
  for(unsigned i=0;i<Count;++i)if(label.View()==Specification(i).type&&owners_[i]&&owners_[i]!=type)return false;
  bool seeded=false;for(unsigned i=0;i<Count;++i)if(!owners_[i]&&label.View()==Specification(i).type){owners_[i]=type;seeded=true;}
  if(eventTypesCount_<eventTypes_.size())eventTypes_[eventTypesCount_++]=type;if(seeded)eventHelpersUntil_=now+20000;
  return seeded;
 }
 unsigned InstanceCount()const noexcept{return instancesCount_;}
 const cfgprobe::lifetime::SlotObservation& Instance(unsigned i)const{return instances_[i];}
 // Event-driven only: late instances do not restart the finite global survey.
 // This proves identity, NOT local-player ownership. The repair adapter must
 // still call its exact native owner getter before reading or changing a panel.
 template<class Reader,class Host>bool EventCandidate(Reader& r,std::uintptr_t module,
     std::uintptr_t object,std::uint64_t now,Host& host,unsigned& candidate){
  if(report_.retired||report_.indeterminate||!RootClass()||!cfgprobe::ObjectPointer(object))return false;
  std::uintptr_t type=0,defaults=0;std::uint32_t flags=0;
  if(!r.Read(object+cfgprobe::ObjectClass,type)||type!=RootClass())return false;
  if(!host.Current()||!r.Read(type+0xF8,defaults)||defaults==object)return false;
  cfgprobe::lifetime::ArrayHeader registry,fresh;cfgprobe::lifetime::SlotObservation slot,again;
  if(!cfgprobe::lifetime::ReadArrayHeader(r,module+cfgprobe::lifetime::ObjectArrayRva,registry)||
     !cfgprobe::lifetime::ReadSlot(r,registry,object,slot)||!slot.HasAssignedSerial()||
     !r.Read(registry.data+24ULL*slot.index+8,flags)||(flags&0x20000000)||
     !cfgprobe::lifetime::ReadSlot(r,registry,object,again)||!(slot==again)||
     !r.Read(object+cfgprobe::ObjectClass,type)||type!=RootClass()||
     !cfgprobe::lifetime::ReadArrayHeader(r,module+cfgprobe::lifetime::ObjectArrayRva,fresh)||!(registry==fresh)||!host.Current())return false;
  unsigned i=0;for(;i<instancesCount_;++i)if(instances_[i]==slot)break;
  if(i==instancesCount_){if(i==instances_.size())return false;instances_[instancesCount_++]=slot;}
  // Construct can precede player ownership. Permit finite, spaced retries on
  // actual future widget events, never an always-on registry or timer scan.
  if(eventRetries_[i]>=16||now<eventNext_[i]||now>UINT64_MAX-1000)return false;
  ++eventRetries_[i];eventNext_[i]=now+1000;candidate=i;return true;
 }
 template<class Reader>bool ReadyHelper(Reader& r,std::uintptr_t module,unsigned i,Patch& out,
     cfgprobe::lifetime::SlotObservation rootedWidget={}){
  out={};if(i>=Count||report_.retired||report_.indeterminate||
    !((report_.changedMask|report_.baseMask)&(1u<<i))||!ready_[i].functionSlot.object)return false;
  if(Metadata(r,module,ready_[i],true)){out=ready_[i];return true;}
  // Late HUD construction can assign a real weak serial to a previously
  // unassigned class/UFunction. A zero serial was never a lifetime identity.
  // Reacquire only through the CURRENT positive owned root's exact class;
  // reject changed positive serials and repeat the entire script/schema proof.
  Patch refreshed;if(!ReacquireRootedHelper(r,module,ready_[i],rootedWidget,refreshed))return false;
  ready_[i]=refreshed;
  if(patches_[i].owned){patches_[i].ownerSlot=refreshed.ownerSlot;patches_[i].functionSlot=refreshed.functionSlot;}
  out=refreshed;return true;
 }
 bool Pending(std::uint64_t now=0)const noexcept{
  if(report_.retired||report_.indeterminate)return false;
  if(report_.expired)return HelpersPending()&&eventHelpersUntil_&&now<eventHelpersUntil_;
  if(!report_.discoveryDone||HelpersPending())return true;
  return MissingOwners()&&(!now||now>=nextProbe_);
 }
 template<class Reader,class Host> void Step(Reader& input,std::uintptr_t module,Host& host,std::uint64_t now=0){
  Budget<Reader> r(input);struct Record{Budget<Reader>& r;Report& report;~Record(){report.lastReads=r.reads;report.peakReads=(std::max)(report.peakReads,r.reads);if(r.exhausted)++report.deferred;}} record{r,report_};
  if(!startedClock_){started_=now;startedClock_=true;}
  if(now<started_||now-started_>=90000){report_.expired=true;
   if(!HelpersPending()||!eventHelpersUntil_||now>=eventHelpersUntil_)return;}
  if(report_.discoveryDone&&!HelpersPending()&&now<nextProbe_)return;
  if(report_.retired||report_.indeterminate||!host.Current()||!Pins(r,module))return;
  cfgprobe::lifetime::ArrayHeader registry;if(!cfgprobe::lifetime::ReadArrayHeader(r,module+cfgprobe::lifetime::ObjectArrayRva,registry)||registry.count>static_cast<int>(MaxRegistry)){++report_.refused;return;}
  if(!limit_)limit_=static_cast<unsigned>(registry.count);
  // One exact helper per phase; no native calls and no object-wide per-frame scan.
  for(unsigned i=0;i<Count;++i)if(owners_[i]&&!done_[i]){
   Patch p;p.spec=i;p.owner=owners_[i];const auto& s=Specification(i);
   cfgreflection::Observation<Budget<Reader>> reflection(r,module);cfgreflection::Field f;
   if(!functions_[i]){
    if(!searchStarted_[i]){if(!r.Read(p.owner+cfgreflection::disk::Children,nextField_[i]))return;searchStarted_[i]=true;}
    if(!nextField_[i]||walked_[i]>=128){for(unsigned j=0;j<Count;++j)if(owners_[j]==p.owner&&!functions_[j]&&!done_[j]){done_[j]=true;++report_.refused;report_.processedMask|=1u<<j;report_.refusedMask|=1u<<j;}return;}
    if(!reflection.ReadField(nextField_[i],p.owner,f)){if(!r.exhausted){done_[i]=true;++report_.refused;report_.processedMask|=1u<<i;report_.refusedMask|=1u<<i;}return;}
    const unsigned walked=walked_[i]+1;
    for(unsigned j=0;j<Count;++j)if(owners_[j]==p.owner){walked_[j]=walked;nextField_[j]=f.next;searchStarted_[j]=true;
     if(f.label.View()==Specification(j).name&&f.kind.View()==u"Function")functions_[j]=f.object;}
    return; // Resumable bounded metadata search, never a whole child-list walk.
   }
   if(!NativeBase(r,module,p.base)||!cfgprobe::lifetime::ReadSlot(r,registry,p.owner,p.ownerSlot)||
    !cfgprobe::lifetime::ReadSlot(r,registry,functions_[i],p.functionSlot)||!cfgmenu::capture::Script(r,functions_[i],p.script,p.bytes)||!Shape(s,p.script,p.bytes)){
     if(!r.exhausted){done_[i]=true;++report_.refused;report_.processedMask|=1u<<i;report_.refusedMask|=1u<<i;}return;}
   p.original=Pointer(p.bytes,s.site+1);
   if(p.original==p.base){
    if(!Metadata(r,module,p,true)){if(!r.exhausted){done_[i]=true;++report_.refused;report_.processedMask|=1u<<i;report_.refusedMask|=1u<<i;}return;}
    ready_[i]=p;done_[i]=true;++report_.processed;++report_.alreadyBase;report_.processedMask|=1u<<i;report_.baseMask|=1u<<i;return;}
   if(!Metadata(r,module,p,false)||!host.Current()){if(!r.exhausted){done_[i]=true;++report_.refused;report_.processedMask|=1u<<i;report_.refusedMask|=1u<<i;}return;}
   if(r.Remaining()<CommitReserve){++report_.deferred;return;}
   done_[i]=true;++report_.processed;report_.processedMask|=1u<<i;
   p.owned=true;patches_[i]=p; // Publish restoration record before any write.
   const bool wrote=host.Exchange(p.script.data+s.site+1,p.original,p.base);
   if(!wrote||!Metadata(r,module,p,true)){patches_[i].uncertain=true;report_.indeterminate=true;report_.refusedMask|=1u<<i;return;}
   ready_[i]=p;++report_.changed;report_.changedMask|=1u<<i;return;
  }
  if(report_.expired)return; // event-seeded helpers only; NEVER restart the census
  if(report_.discoveryDone){
   if(!MissingOwners())return;
   // At most one delayed repeat covers reused slots. Otherwise only newly
   // appended registry slots are examined. Both routes expire at90s; this is
   // not a perpetual object census or a substitute for a future widget event.
   if(!secondPass_&&now-started_>=45000){secondPass_=true;cursor_=0;limit_=static_cast<unsigned>(registry.count);cache_={};report_.discoveryDone=false;}
   else if(static_cast<unsigned>(registry.count)>limit_){limit_=static_cast<unsigned>(registry.count);report_.discoveryDone=false;}
   else{nextProbe_=now+500;return;}
  }
  if(static_cast<unsigned>(registry.count)<limit_){++report_.refused;report_.discoveryDone=true;return;}
  const auto priorOwners=owners_;const unsigned end=(std::min)(limit_,cursor_+ScanBatch);
  const unsigned pageStart=cursor_;
  if(end>cursor_&&!r.ReadBytes(registry.data+24ULL*cursor_,registryPage_.data(),24ULL*(end-cursor_))){++report_.deferred;return;}
  for(;cursor_<end&&cursor_<static_cast<unsigned>(registry.count);++cursor_){
   if(r.Remaining()<100)break; // Worst-case bounded FName plus final header.
   const auto object=registryPage_[(cursor_-pageStart)*3];std::uintptr_t type=0;
   ++report_.scanned;if(!object)continue;
   if(!r.Read(object+cfgprobe::ObjectClass,type)){++report_.refused;continue;}
   if(type==RootClass()&&type&&instancesCount_<instances_.size()){
    bool seen=false;for(unsigned i=0;i<instancesCount_;++i)if(instances_[i].object==object)seen=true;
    std::uintptr_t defaults=0;cfgprobe::lifetime::SlotObservation slot;std::uint32_t flags=0;
    if(!seen&&r.Read(type+0xF8,defaults)&&defaults!=object&&
      cfgprobe::lifetime::ReadSlot(r,registry,object,slot)&&slot.serial>0&&
      r.Read(registry.data+24ULL*slot.index+8,flags)&&!(flags&0x20000000))instances_[instancesCount_++]=slot;
   }
   bool generated=false;cfgnames::Observation<Budget<Reader>> names(r,module);cfgnames::Text name;
   if(!cache_.Lookup(type,generated)){if(!names.Object(type,name)){++report_.refused;continue;}generated=name.View()==u"WidgetBlueprintGeneratedClass";cache_.Store(type,generated);}
   if(!generated||!names.Object(object,name))continue;
   for(unsigned i=0;i<Count;++i)if(!owners_[i]&&name.View()==Specification(i).type)owners_[i]=object;
  }
  cfgprobe::lifetime::ArrayHeader fresh;
  if(!cfgprobe::lifetime::ReadArrayHeader(r,module+cfgprobe::lifetime::ObjectArrayRva,fresh)||!(fresh==registry)){
   owners_=priorOwners;cursor_=pageStart;++report_.deferred;return;
  }
  report_.discoveryDone=cursor_>=limit_;
  if(report_.discoveryDone)nextProbe_=now+500;
 }
 // Call on a safe owner-thread retirement phase, including world transition.
 // Host.Current must allow metadata-only restoration in that phase, never writes
 // to dead world objects. Changed/reused metadata is refused, not repaired.
 bool PendingRestore()const noexcept{for(unsigned i=0;i<Count;++i)if(patches_[i].owned&&!restoreAttempted_[i])return true;return false;}
 template<class Reader,class Host> void RetireOne(Reader& input,std::uintptr_t module,Host& host){
  Budget<Reader> r(input);struct Record{Budget<Reader>& r;Report& report;~Record(){report.lastReads=r.reads;report.peakReads=(std::max)(report.peakReads,r.reads);if(r.exhausted)++report.deferred;}} record{r,report_};
  report_.retired=true;
  for(unsigned i=0;i<Count;++i){auto& p=patches_[i];if(!p.owned||restoreAttempted_[i])continue;
   if(p.uncertain){restoreAttempted_[i]=true;++report_.restoreRefused;return;}
   if(!host.Current())return;
   if(restoreRefresh_[i]){
    Patch fresh;if(!ReacquireRetiringMetadata(r,module,p,fresh)){
     if(!r.exhausted){restoreAttempted_[i]=true;++report_.restoreRefused;}return;}
    p=fresh;restoreRefresh_[i]=false;return; // ordinary exact restore owns next slice
   }
   if(!Metadata(r,module,p,true)){
    if(r.exhausted)return;
    // The failed old-serial walk already spent a slice. Reacquisition owns a
    // fresh bounded callback below; never stack two full metadata proofs.
    if(!p.ownerSlot.serial||!p.functionSlot.serial){restoreRefresh_[i]=true;return;}
    restoreAttempted_[i]=true;++report_.restoreRefused;return;
   }
   if(r.Remaining()<CommitReserve){++report_.deferred;return;}
   restoreAttempted_[i]=true;
   if(!host.Exchange(p.script.data+Specification(p.spec).site+1,p.base,p.original)||!Metadata(r,module,p,false)){p.uncertain=true;report_.indeterminate=true;++report_.restoreRefused;return;}
   p.owned=false;++report_.restored;return;
  }
 }
 // Compatibility alias is deliberately ONE item, never a synchronous batch.
 template<class Reader,class Host>void Retire(Reader& r,std::uintptr_t module,Host& host){RetireOne(r,module,host);}
};
}
