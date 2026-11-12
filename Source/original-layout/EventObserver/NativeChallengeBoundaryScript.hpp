#pragma once
#include "NativeChallengeEscapeActors.hpp"
#include "../RuntimeProbe/MenuScriptObservation.hpp"
#include "../RuntimeProbe/SettingsStartObservation.hpp"
#include <memory>
namespace cfgchallengeboundary {
using Evidence=cfgmenu::capture::FunctionEvidenceStorage<32,1024>;
inline constexpr std::array<std::u16string_view,3> Names{{u"SP_Level_Boundary_Inside",u"SP_Level_Boundary_Outside",u"Restart_Mission_Out_Of_Bounds"}};
// Independently captured CFG wrapper and all three boundary functions end in
// A5 39 E5 (void Return/Nothing/End). Donor04 0B must NEVER be written to CFG.
inline constexpr std::array<unsigned char,2> Return{{0xA5,0x39}};
struct Local {std::u16string_view name,kind;int offset,bytes;std::uint64_t flags;};
inline constexpr std::uint64_t ObjectLocal=0x0/*CFG3.5 address withheld*/40000200ULL,BoolLocal=0x0000001040000200ULL;
inline constexpr std::array<Local,14> InsideLocals{{
 {u"SCKillerCharacter",u"ObjectProperty",0,8,0x0008001040000294ULL},
 {u"Temp_struct_Variable",u"StructProperty",16,48,0x0000001040000000ULL},
 {u"K2Node_DynamicCast_AsSCKiller_Character",u"ObjectProperty",64,8,ObjectLocal},
 {u"K2Node_DynamicCast_bSuccess",u"BoolProperty",72,1,BoolLocal},
 {u"CallFunc_GetController_ReturnValue",u"ObjectProperty",80,8,ObjectLocal},
 {u"CallFunc_GetGameState_ReturnValue",u"ObjectProperty",88,8,ObjectLocal},
 {u"K2Node_DynamicCast_AsSCPlayer_Controller",u"ObjectProperty",96,8,ObjectLocal},
 {u"K2Node_DynamicCast_bSuccess2",u"BoolProperty",104,1,BoolLocal},
 {u"K2Node_DynamicCast_AsSCGame_State_SPChallenges",u"ObjectProperty",112,8,ObjectLocal},
 {u"K2Node_DynamicCast_bSuccess3",u"BoolProperty",120,1,BoolLocal},
 {u"CallFunc_GetHUD_ReturnValue",u"ObjectProperty",128,8,ObjectLocal},
 {u"K2Node_DynamicCast_AsSCIn_Game_HUD",u"ObjectProperty",136,8,ObjectLocal},
 {u"K2Node_DynamicCast_bSuccess4",u"BoolProperty",144,1,BoolLocal},
 {u"CallFunc_AddComponent_ReturnValue",u"ObjectProperty",152,8,ObjectLocal}
}};
inline constexpr std::array<Local,17> OutsideLocals{{
 {u"SCKillerCharacter",u"ObjectProperty",0,8,0x0008001040000294ULL},
 {u"K2Node_DynamicCast_AsSCKiller_Character",u"ObjectProperty",8,8,ObjectLocal},
 {u"K2Node_DynamicCast_bSuccess",u"BoolProperty",16,1,BoolLocal},
 {u"CallFunc_GetGameState_ReturnValue",u"ObjectProperty",24,8,ObjectLocal},
 {u"CallFunc_GetController_ReturnValue",u"ObjectProperty",32,8,ObjectLocal},
 {u"K2Node_DynamicCast_AsSCGame_State_SPChallenges",u"ObjectProperty",40,8,ObjectLocal},
 {u"K2Node_DynamicCast_bSuccess2",u"BoolProperty",48,1,BoolLocal},
 {u"K2Node_DynamicCast_AsSCPlayer_Controller",u"ObjectProperty",56,8,ObjectLocal},
 {u"K2Node_DynamicCast_bSuccess3",u"BoolProperty",64,1,BoolLocal},
 {u"CallFunc_GetHUD_ReturnValue",u"ObjectProperty",72,8,ObjectLocal},
 {u"K2Node_DynamicCast_AsSCIn_Game_HUD",u"ObjectProperty",80,8,ObjectLocal},
 {u"K2Node_DynamicCast_bSuccess4",u"BoolProperty",88,1,BoolLocal},
 {u"Temp_struct_Variable",u"StructProperty",96,48,0x0000001040000000ULL},
 {u"CallFunc_AddComponent_ReturnValue",u"ObjectProperty",144,8,ObjectLocal},
 {u"K2Node_MakeArray_Array",u"ArrayProperty",152,16,0x0000000008000200ULL},
 {u"CallFunc_RandomInteger_ReturnValue",u"IntProperty",168,4,ObjectLocal},
 {u"CallFunc_Array_Get_Item",u"ObjectProperty",176,8,ObjectLocal}
}};
inline constexpr std::array<Local,3> RestartLocals{{
 {u"CallFunc_GetGameMode_ReturnValue",u"ObjectProperty",0,8,ObjectLocal},
 {u"K2Node_DynamicCast_AsSCGame_Mode",u"ObjectProperty",8,8,ObjectLocal},
 {u"K2Node_DynamicCast_bSuccess",u"BoolProperty",16,1,BoolLocal}
}};
template<std::size_t N>bool Locals(const Evidence& e,const std::array<Local,N>& specs){
 if(e.fieldsCount!=N)return false;
 for(unsigned i=0;i<N;++i){const auto& p=e.fields[i];const auto& s=specs[i];const auto label=p.field.label.View();
  if((label!=s.name&&!(i==0&&s.name==u"SCKillerCharacter"&&label==u"ScKillerCharacter"))||p.field.kind.View()!=s.kind||
     p.offset!=s.offset||p.dimension!=1||p.elementBytes!=s.bytes||p.flags!=s.flags)return false;
 }return true;
}
// Counts every metadata read independently of the caller and reserves enough
// work for BOTH rollback checks before the first edit. No exhausted-budget
// rollback followed by forgetting an owned partial patch.
template<class R>struct Budget {
 R& source;unsigned reads=0,cap=65536;
 unsigned Remaining()const{return reads<cap?cap-reads:0;}
 template<class T>bool Read(std::uintptr_t at,T& out){if(!Remaining())return false;++reads;return source.Read(at,out);}
 bool ReadBytes(std::uintptr_t at,void* out,std::size_t bytes){if(!Remaining())return false;++reads;return source.ReadBytes(at,out,bytes);}
};
inline bool Shape(const Evidence& e,unsigned kind,std::uintptr_t module){
 if(kind>=Names.size()||e.function.kind.View()!=u"Function"||e.function.label.View()!=Names[kind]||
    e.nativeExecutor!=module+cfgmenu::startdisk::ScriptExecutor||e.script.capacity<e.script.count||
    e.script.count!=(kind==0?692:kind==1?922:161)||e.bytes[e.script.count-3]!=0xA5||
    e.bytes[e.script.count-2]!=0x39||e.bytes[e.script.count-1]!=0xE5)return false;
 if(kind==2){if(e.flags!=0x0C020000||e.parameterBytes||e.storageBytes!=17||e.destructorLink||
    !Locals(e,RestartLocals)||e.bytes[0]!=0x0D||e.bytes[1]!=0x76)return false;
  return true;
 }
 if(e.flags!=0x0C820000||e.parameterBytes!=8||e.storageBytes!=(kind==0?160:184))return false;
 unsigned inputs=0;
 for(unsigned i=0;i<e.fieldsCount;++i){const auto& p=e.fields[i];if(!(p.flags&0x80))continue;++inputs;
  if((p.field.label.View()!=u"SCKillerCharacter"&&p.field.label.View()!=u"ScKillerCharacter")||
     p.field.kind.View()!=u"ObjectProperty"||p.offset||p.dimension!=1||p.elementBytes!=8||p.flags!=0x0/*CFG3.5 address withheld*/40000294ULL)return false;
 }
 if(inputs!=1||(kind==0?!Locals(e,InsideLocals):!Locals(e,OutsideLocals)))return false;
 if(kind==0&&e.destructorLink)return false;
 if(kind==1&&e.destructorLink!=e.fields[14].field.object)return false;
 {const std::array<unsigned char,5> prefix{{0x22,static_cast<unsigned char>(kind==0?0xB1:0x97),static_cast<unsigned char>(kind==0?2:3),0,0}};
  if(!std::equal(prefix.begin(),prefix.end(),e.bytes.begin()))return false;
  std::uint32_t target=0;std::memcpy(&target,e.bytes.data()+1,4);
  if(target!=static_cast<unsigned>(e.script.count-3))return false;
 }
 return true;
}
inline bool Same(const Evidence& a,const Evidence& b){
 if(!(a.slot==b.slot)||a.function.object!=b.function.object||a.function.owner!=b.function.owner||
    a.function.type!=b.function.type||a.function.next!=b.function.next||!(a.function.name==b.function.name)||
    a.flags!=b.flags||a.parameterBytes!=b.parameterBytes||a.storageBytes!=b.storageBytes||
    a.nativeExecutor!=b.nativeExecutor||a.destructorLink!=b.destructorLink||!(a.script==b.script)||
    a.fieldsCount!=b.fieldsCount||a.bytes!=b.bytes)return false;
 for(unsigned i=0;i<a.fieldsCount;++i)if(!cfgmenu::capture::SameProperty(a.fields[i],b.fields[i]))return false;
 return true;
}
struct Owner {cfgprobe::lifetime::SlotObservation slot{};std::uintptr_t type=0,head=0,defaults=0;cfgnames::Words name{};};
template<class R>bool ObserveOwner(R& r,std::uintptr_t module,std::uintptr_t object,Owner& out){
 cfgprobe::lifetime::ArrayHeader registry;std::uint32_t internal=0,flags=0;cfgnames::Text name;
 cfgnames::Observation<R> names(r,module);out={};
 return cfgprobe::lifetime::ReadArrayHeader(r,module+cfgprobe::lifetime::ObjectArrayRva,registry)&&
  cfgprobe::lifetime::ReadSlot(r,registry,object,out.slot)&&r.Read(registry.data+24ULL*static_cast<unsigned>(out.slot.index)+8,internal)&&!(internal&0x30000000u)&&
  r.Read(object+8,flags)&&!(flags&0x30u)&&r.Read(object+cfgprobe::ObjectClass,out.type)&&
  r.Read(object+cfgreflection::disk::Children,out.head)&&r.Read(object+cfgspawn::ClassDefaultObject,out.defaults)&&
  cfgprobe::ObjectPointer(out.defaults)&&r.Read(object+cfgnames::disk::ObjectName,out.name)&&
  names.Decode(out.name,name)&&name.View()==u"SinglePlayer_Dispatcher_C";
}
inline bool SameOwner(const Owner& a,const Owner& b){return a.slot==b.slot&&a.type==b.type&&a.head==b.head&&a.defaults==b.defaults&&a.name==b.name;}
inline bool BorrowSlot(const cfgprobe::lifetime::SlotObservation& old,const cfgprobe::lifetime::SlotObservation& fresh){
 return old==fresh||(old.serial==0&&fresh.HasAssignedSerial()&&old.object==fresh.object&&old.index==fresh.index);
}
struct Record {std::unique_ptr<Evidence> original;Owner owner{};bool owned=false;};
struct ProofDiagnostic {unsigned stage=0,record=0,difference=0;};
inline unsigned Difference(const Evidence& a,const Evidence& b){
 if(!(a.slot==b.slot))return 1;
 if(a.function.object!=b.function.object||a.function.owner!=b.function.owner||a.function.type!=b.function.type||
    a.function.next!=b.function.next||!(a.function.name==b.function.name))return 2;
 if(a.flags!=b.flags||a.parameterBytes!=b.parameterBytes||a.storageBytes!=b.storageBytes||
    a.nativeExecutor!=b.nativeExecutor||a.destructorLink!=b.destructorLink)return 3;
 if(!(a.script==b.script))return 4;
 if(a.fieldsCount!=b.fieldsCount)return 5;
 if(a.bytes!=b.bytes)return 6;
 for(unsigned i=0;i<a.fieldsCount;++i)if(!cfgmenu::capture::SameProperty(a.fields[i],b.fields[i]))return 7;
 return 0;
}
// Metadata serial0 is a diagnostic observation, NOT a native-call lease. New
// edits additionally require a fresh positive stock Challenge source and its
// current strong actor/class/field links. Cleanup can only conditionally undo
// OUR exact edit after the full metadata, slot and whole script match; no
// native call, stale World read, serial allocation or foreign overwrite.
template<class R>bool Current(R& r,std::uintptr_t module,Record& record,bool patched,ProofDiagnostic* diagnostic=nullptr){
 ProofDiagnostic local;if(!diagnostic)diagnostic=&local;
 diagnostic->stage=1;diagnostic->difference=0;
 if(!record.original||!cfgmenu::offlineowner::ChallengeLookupSafe(r,module))return false;const auto& old=*record.original;
 cfgprobe::lifetime::ArrayHeader registry;cfgprobe::lifetime::SlotObservation slot;
 std::uint32_t flags=0,objectFlags=0;
 diagnostic->stage=2;
 if(!cfgprobe::lifetime::ReadArrayHeader(r,module+cfgprobe::lifetime::ObjectArrayRva,registry)||
    !cfgprobe::lifetime::ReadSlot(r,registry,old.function.object,slot)||!BorrowSlot(old.slot,slot)||
    !r.Read(registry.data+24ULL*static_cast<unsigned>(slot.index)+8,flags)||(flags&0x30000000u)||
    !r.Read(slot.object+8,objectFlags)||(objectFlags&0x30u))return false;
 diagnostic->stage=3;
 Owner owner;if(!ObserveOwner(r,module,old.function.owner,owner)||!BorrowSlot(record.owner.slot,owner.slot))return false;
 diagnostic->stage=4;
 Owner normalized=owner;normalized.slot=record.owner.slot;if(!SameOwner(normalized,record.owner))return false;
 // Confirm this function is still a declared child, not merely a field with
 // the expected Outer. Class/function serial0 remains a strong-link borrow.
 diagnostic->stage=5;
 std::array<std::uintptr_t,512> walked{},links{};unsigned count=0,matches=0;
 for(auto node=owner.head;node;){if(count==walked.size()||!cfgprobe::ObjectPointer(node))return false;
  for(unsigned i=0;i<count;++i)if(walked[i]==node)return false;
  std::uintptr_t next=0;if(!r.Read(node+cfgreflection::disk::Next,next))return false;
  walked[count]=node;links[count++]=next;if(node==old.function.object)++matches;node=next;
 }
 if(matches!=1)return false;
 for(unsigned i=0;i<count;++i)if(!cfgchallengeescape::actors::Same(r,walked[i]+cfgreflection::disk::Next,links[i]))return false;
 if(!cfgchallengeescape::actors::Same(r,owner.slot.object+cfgreflection::disk::Children,owner.head))return false;
 diagnostic->stage=6;
 cfgreflection::Observation<R> reflection(r,module);cfgreflection::Field field;
 auto fresh=std::make_unique<Evidence>();
 if(!reflection.ReadField(old.function.object,old.function.owner,field)||
    !cfgmenu::capture::KnownFunction(r,module,field,old.function.label.View(),*fresh))return false;
 diagnostic->stage=7;
 if(patched){if(fresh->bytes[0]!=Return[0]||fresh->bytes[1]!=Return[1])return false;
  fresh->bytes[0]=old.bytes[0];fresh->bytes[1]=old.bytes[1];}
 // An engine-assigned serial is not reincarnation of an unassigned metadata
 // borrow. Admit that ONE upgrade only after the full unchanged script/schema,
 // declaring-class child chain and double-sampled owner/slot proof. Latch it:
 // a later different positive serial can never be treated as another upgrade.
 const auto assigned=fresh->slot;fresh->slot=old.slot;Owner end;cfgprobe::lifetime::SlotObservation finalSlot;
 diagnostic->stage=8;diagnostic->difference=Difference(old,*fresh);
 if(!(assigned==slot)||!Same(old,*fresh))return false;
 diagnostic->stage=9;
 if(!ObserveOwner(r,module,old.function.owner,end)||!SameOwner(owner,end)||
    !cfgprobe::lifetime::ReadSlot(r,registry,old.function.object,finalSlot)||!(finalSlot==assigned)||
    !cfgmenu::offlineowner::ChallengeLookupSafe(r,module))return false;
 record.original->slot=assigned;record.owner.slot=owner.slot;diagnostic->stage=0;return true;
}
class Pair {
 std::array<Record,2> records_;std::uintptr_t module_=0;
public:
 unsigned applied=0,restored=0,failure=0;
 ProofDiagnostic verifyFailure{};
 unsigned verifyRetries=0,verifyRecoveries=0;
 bool Owned()const{return records_[0].owned||records_[1].owned;}
 bool Ready()const{return records_[0].owned&&records_[1].owned;}
 template<class R>bool Verify(R& r){
  ProofDiagnostic detail;
  if(!Ready()||r.Remaining()<16384){detail.stage=10;verifyFailure=detail;return false;}
  for(unsigned i=0;i<2;++i){detail.record=i+1;
   if(!Current(r,module_,records_[i],true,&detail)){verifyFailure=detail;return false;}}
  return true;
 }
 // H2.94 read-only: both functions are still the exact registry objects we patched (same slot object and serial), their
 // script header is unchanged and its first two bytes are still our Return patch. Failing this means a REAL change
 // (removed/recycled/rewritten): the caller retires at once. Passing it only permits waiting out a full-Verify read race.
 template<class R>bool StillOursPatched(R& r){
  if(!Ready())return false;
  for(const auto& record:records_){
   cfgprobe::lifetime::ArrayHeader registry;std::uintptr_t object=0;int serial=0;const auto& slot=record.original->slot;
   cfgmenu::capture::ScriptHeader header;std::array<unsigned char,2> bytes{};
   if(!cfgprobe::lifetime::ReadArrayHeader(r,module_+cfgprobe::lifetime::ObjectArrayRva,registry)||slot.index<0||slot.index>=registry.count||
      !r.Read(registry.data+24ULL*static_cast<unsigned>(slot.index),object)||object!=slot.object||
      !r.Read(registry.data+24ULL*static_cast<unsigned>(slot.index)+16,serial)||(slot.HasAssignedSerial()&&serial!=slot.serial)||
      !r.Read(slot.object+cfgmenu::scriptdisk::Script,header)||!(header==record.original->script)||
      !r.Read(record.original->script.data,bytes)||bytes!=Return)return false;
   // H2.96 (audit 2 P2): function liveness and schema metadata must also equal the install snapshot.
   {std::uint32_t internal=0,objectFlags=0,flags=0;std::uint16_t parameters=0;std::int32_t storage=0;std::uintptr_t executor=0,cleanup=0;
    if(!r.Read(registry.data+24ULL*static_cast<unsigned>(slot.index)+8,internal)||(internal&0x30000000u)||
       !r.Read(slot.object+8,objectFlags)||(objectFlags&0x30u)||
       !r.Read(slot.object+cfgreflection::disk::FunctionFlags,flags)||flags!=record.original->flags||
       !r.Read(slot.object+cfgreflection::arraydisk::FunctionParameterBytes,parameters)||parameters!=record.original->parameterBytes||
       !r.Read(slot.object+cfgreflection::disk::StorageBytes,storage)||storage!=record.original->storageBytes||
       !r.Read(slot.object+0xB0,executor)||executor!=record.original->nativeExecutor||
       !r.Read(slot.object+0x68,cleanup)||cleanup!=record.original->destructorLink)return false;}
   // Whole script beyond our two patched bytes must equal the snapshot taken at install (a rewrite anywhere retires).
   const auto count=static_cast<unsigned>(header.count);if(count<2||count>record.original->bytes.size())return false;
   auto copy=std::make_unique<std::array<unsigned char,cfgmenu::scriptdisk::MaxBytes>>();
   if(!r.ReadBytes(header.data,copy->data(),count))return false;
   for(unsigned i=2;i<count;++i)if((*copy)[i]!=record.original->bytes[i])return false;
  }
  return true;
 }
 template<class R,class W>bool Restore(R& r,W& writer){
  if(Owned()&&r.Remaining()<16384){failure=6;return false;}
  bool good=true;
  for(auto& record:records_)if(record.owned){
   // Only the registry item is read first. A removed/recycled function is no
   // longer our object and must never be dereferenced or restored.
   cfgprobe::lifetime::ArrayHeader registry;std::uintptr_t object=0;int serial=0;const auto& slot=record.original->slot;
   if(cfgprobe::lifetime::ReadArrayHeader(r,module_+cfgprobe::lifetime::ObjectArrayRva,registry)&&slot.index>=0&&slot.index<registry.count&&
      r.Read(registry.data+24ULL*static_cast<unsigned>(slot.index),object)&&
      r.Read(registry.data+24ULL*static_cast<unsigned>(slot.index)+16,serial)&&
      (object!=slot.object||(slot.HasAssignedSerial()&&serial!=slot.serial))){writer.Forget(record.original->script.data);record.owned=false;continue;}
   if(Current(r,module_,record,false)){
    // Bytes are already restored. A failed protection restoration is retried
    // only after THIS full current metadata proof, not from a stale address.
    const std::array<unsigned char,2> original{{record.original->bytes[0],record.original->bytes[1]}};
    if(writer.Pending()&&!writer.Exchange(record.original->script.data,original,original)){good=false;continue;}
    record.owned=false;continue;
   }
   if(!Current(r,module_,record,true)){good=false;continue;}
   const std::array<unsigned char,2> original{{record.original->bytes[0],record.original->bytes[1]}};
   if(writer.Exchange(record.original->script.data,Return,original)&&Current(r,module_,record,false)){
    record.owned=false;++restored;
   }else good=false;
  }
  if(!good)failure=5;return good;
 }
 template<class R,class W,class Lease>bool Apply(R& r,W& writer,std::uintptr_t module,
  const std::array<std::unique_ptr<Evidence>,3>& functions,Lease&& lease){
  failure=1;if(Owned()||r.Remaining()<32768||!lease()||!functions[0]||!functions[1]||!functions[2])return false;
  for(unsigned i=0;i<functions.size();++i)if(!Shape(*functions[i],i,module)||functions[i]->function.owner!=functions[0]->function.owner)return false;
  module_=module;Owner owner;if(!ObserveOwner(r,module,functions[0]->function.owner,owner))return false;
  for(unsigned i=0;i<2;++i){records_[i].original=std::make_unique<Evidence>(*functions[i+1]);records_[i].owner=owner;}
  failure=2;if(!Current(r,module_,records_[0],false)||!Current(r,module_,records_[1],false)||!lease())return false;
  for(unsigned i=0;i<2;++i){auto& record=records_[i];failure=3;
   if(!lease()||!Current(r,module_,record,false)){(void)Restore(r,writer);return false;}
   const std::array<unsigned char,2> original{{record.original->bytes[0],record.original->bytes[1]}};
   if(!writer.Exchange(record.original->script.data,original,Return)){
    // A protection-restoration failure may still have changed the bytes. Keep
    // cleanup ownership unless a full original snapshot proves no mutation.
    record.owned=writer.Pending()||!Current(r,module_,record,false);(void)Restore(r,writer);return false;
   }
   record.owned=true;++applied;failure=4;
   if(!Current(r,module_,record,true)||!lease()){(void)Restore(r,writer);return false;}
  }
  failure=0;return true;
 }
};
}
