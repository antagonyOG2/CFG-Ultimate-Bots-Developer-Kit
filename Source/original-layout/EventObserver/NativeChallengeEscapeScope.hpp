#pragma once
#include "NativeChallengeOptions.hpp"
#include "../RuntimeProbe/ChallengeEscapeRecovery.generated.hpp"
#include <cstring>

namespace cfgchallengeescape::scope {
// Fresh index then object/flags/serial observation. The complete bounded
// registry extent is checked first; ignored cluster bytes grant no authority.
template<class R>bool ReadIdentity(R& r,const cfgprobe::lifetime::ArrayHeader& registry,
 std::uintptr_t object,cfgprobe::lifetime::SlotObservation& out){
 namespace life=cfgprobe::lifetime;out={};
 static_assert(life::ItemBytes==24&&life::ItemObjectOffset==0&&life::ItemSerialOffset==16);
 if(!registry.Sane()||!cfgprobe::ObjectPointer(object)||!life::Extent(object,life::ObjectIndexOffset+sizeof(std::int32_t)))return false;
 int index=-1;if(!r.Read(object+life::ObjectIndexOffset,index)||index<0||index>=registry.count)return false;
 std::array<unsigned char,20> bytes{};std::uintptr_t seen=0;std::uint32_t flags=0;int serial=0;
 if(!r.Read(registry.data+life::ItemBytes*static_cast<std::uintptr_t>(index),bytes))return false;
 std::memcpy(&seen,bytes.data(),8);std::memcpy(&flags,bytes.data()+8,4);std::memcpy(&serial,bytes.data()+16,4);
 if(seen!=object||serial<=0||(flags&0x30000000u))return false;out={object,index,serial};return true;
}
// This is an admission proof, not a game writer. An explicit, monotonically
// increasing setup submission owns one exact stock map/mission pair. As in the
// donor MissionSettingsBinding, that pair is first resolved from the native
// dossier after travel; another mission requires another setup submission.
struct Witness {
 std::uint64_t generation=0;
 cfgprobe::lifetime::ArrayHeader registry{};
 std::array<cfgprobe::lifetime::SlotObservation,6> owners{};
 std::uintptr_t players=0,dossier=0,missionClass=0,mapOuter=0,missionOuter=0,nameTable=0;
 cfgnames::Words phase{},map{},mission{},missionPackage{};
 bool operator==(const Witness& b)const{
  return generation==b.generation&&registry==b.registry&&owners==b.owners&&players==b.players&&
   dossier==b.dossier&&missionClass==b.missionClass&&mapOuter==b.mapOuter&&missionOuter==b.missionOuter&&nameTable==b.nameTable&&
   phase==b.phase&&map==b.map&&mission==b.mission&&missionPackage==b.missionPackage;
 }
 std::uintptr_t World()const{return owners[0].object;}
 std::uintptr_t Mode()const{return owners[1].object;}
 std::uintptr_t State()const{return owners[4].object;}
};
enum class Failure:unsigned {None,Disabled,Submission,Thread,Context,Identity,Metadata,Mission,Phase,Changed,Expired};
class Runtime {
 cfgmenu::freshgame::Source source_{};
 std::uint64_t generation_=0,deadline_=0;bool enabled_=false,bound_=false;
 Witness boundWitness_{};
 struct Member {std::uintptr_t type=0,head=0;cfgreflection::Property property{};cfgprobe::Ancestry ancestry{};cfgnames::Words kind{};};
 std::array<Member,3> members_{};
 template<class R>bool Typed(R& r,std::uintptr_t object,std::uintptr_t cache,bool exact=false){
  std::array<std::uintptr_t,2> classes{};std::uintptr_t actual=0,defaults=0;
  return cfgprobe::ObjectPointer(object)&&r.Read(source_.module+cache,classes)&&
   cfgprobe::MatchingClassCache(classes[0],classes[1])&&r.Read(object+cfgprobe::ObjectClass,actual)&&
   (exact?actual==classes[0]:cfgmenu::maps::Subclass(r,actual,classes[0]))&&r.Read(actual+cfgspawn::ClassDefaultObject,defaults)&&
   cfgprobe::ObjectPointer(defaults)&&defaults!=object;
 }
 template<class R>bool Field(R& r,unsigned index,std::uintptr_t object,std::u16string_view name,
  std::u16string_view kind,int offset,std::uintptr_t& at){
  at=0;std::uintptr_t type=0;if(index>=members_.size()||!r.Read(object+cfgprobe::ObjectClass,type))return false;
  auto& m=members_[index];cfgreflection::Observation<R> reflection(r,source_.module);
  if(!m.type){cfgreflection::Field f;std::uintptr_t owner=type;
   // NetConnection belongs to native PlayerController, not CFG's much larger
   // derived SC controller. Do not run the UI's full 256-field census through
   // that derived hierarchy (which silently makes escape admission impossible).
   if(index==0){std::uintptr_t ctor=0;cfgprobe::Ancestry ancestry;
    if(!r.Read(source_.module+disk::PlayerController,owner)||!r.Read(source_.module+disk::PlayerControllerCtor,ctor)||
       !cfgprobe::MatchingClassCache(owner,ctor)||!cfgprobe::CaptureAncestry(r,object,ancestry)||!ancestry.Contains(owner))return false;
   }
   std::uintptr_t head=0,child=0;std::array<std::uintptr_t,512> walked{},links{};unsigned count=0;
   cfgnames::Observation<R> names(r,source_.module);bool found=false;
   if(!r.Read(owner+cfgreflection::disk::Children,head))return false;child=head;
   while(child){if(count==walked.size())return false;
    for(unsigned i=0;i<count;++i)if(walked[i]==child)return false;
    cfgnames::Text label;std::uintptr_t next=0;if(!r.Read(child+cfgreflection::disk::Next,next)||!names.Object(child,label))return false;
    walked[count]=child;links[count++]=next;
    if(label.View()==name){found=reflection.ReadField(child,owner,f);break;}child=next;
   }
   if(!found||!cfgmenu::offlineowner::Same(r,owner+cfgreflection::disk::Children,head))return false;
   for(unsigned i=0;i<count;++i)if(!cfgmenu::offlineowner::Same(r,walked[i]+cfgreflection::disk::Next,links[i]))return false;
   if(!cfgmenu::capture::RawProperty(r,f,m.property)||
      !r.Read(f.owner+cfgreflection::disk::Children,m.head)||!cfgprobe::CaptureAncestry(r,object,m.ancestry)||
      !r.Read(f.type+cfgnames::disk::ObjectName,m.kind))return false;
   m.type=type;
  }
  const auto& p=m.property;cfgreflection::Field fresh;cfgreflection::Property actual;
  cfgprobe::Ancestry ancestry;std::uintptr_t head=0;
  if(m.type!=type||!cfgprobe::CaptureAncestry(r,object,ancestry)||!ancestry.Contains(p.field.owner)||
     !r.Read(p.field.owner+cfgreflection::disk::Children,head)||head!=m.head||
     !reflection.ReadField(p.field.object,p.field.owner,fresh)||
     !cfgmenu::capture::RawProperty(r,fresh,actual)||!cfgmenu::capture::SameProperty(p,actual)||
     fresh.label.View()!=name||fresh.kind.View()!=kind||(offset>=0&&actual.offset!=offset)||(offset<0&&(actual.offset<=0||actual.offset>=0x0/*CFG3.5 address withheld*/))||actual.dimension!=1||actual.elementBytes!=8||
     !cfgprobe::StableAncestry(r,object,ancestry))return false;
  at=object+static_cast<unsigned>(offset>=0?offset:actual.offset);return true;
 }
 template<class R>bool Flags(R& r,const cfgprobe::lifetime::ArrayHeader& registry,
  const cfgprobe::lifetime::SlotObservation& slot){
  std::uint32_t flags=0;
  return r.Read(registry.data+24ULL*static_cast<unsigned>(slot.index)+8,flags)&&!(flags&0x30000000u);
 }
 template<class R>bool Sample(R& r,std::uint32_t thread,Witness& out,bool intro=false){
  using namespace cfgprobe;out={};Witness w;w.generation=generation_;
  if(!r.Read(source_.module+cfgnames::disk::Global,w.nameTable)||!ObjectPointer(w.nameTable))return false;
  const auto same=[&](std::uintptr_t at,auto expected){return cfgmenu::offlineowner::Same(r,at,expected);};
  failure=Failure::Thread;
  if(thread!=source_.thread||!cfgmenu::offlineowner::ChallengeLookupSafe(r,source_.module)||
     !same(source_.module+cfgprobe::dispatch::ThreadFlagRva,std::uint8_t{1})||!same(source_.module+cfgprobe::dispatch::ThreadIdRva,thread))return false;
  std::uintptr_t world=0,mode=0,controller=0,pawn=0,state=0,ps=0,connection=0,at=0;
  int count=0,capacity=0;failure=Failure::Context;
  if(!same(source_.module+EngineGlobalCandidate,source_.engine)||!same(source_.engine+EngineGameViewport,source_.viewport)||
     !r.Read(source_.viewport+ViewportWorld,world)||world==source_.frontendWorld||
     !same(world+WorldNetDriver,std::uintptr_t{})||!same(world+WorldGameInstance,source_.gameInstance)||
     !r.Read(world+WorldGameMode,mode)||!Typed(r,mode,disk::SCGameMode_SPChallenges,true)||!same(mode+0x110,std::uint8_t{3})||
     !r.Read(source_.gameInstance+GameInstanceLocalPlayers,w.players)||!ObjectPointer(w.players)||
     !r.Read(source_.gameInstance+GameInstanceLocalPlayers+8,count)||count!=1||
     !r.Read(source_.gameInstance+GameInstanceLocalPlayers+12,capacity)||capacity<1||capacity>4||!same(w.players,source_.player)||
     !r.Read(source_.player+PlayerController,controller)||!Typed(r,controller,NativeClasses[PlayerControllerClass].reflectionCacheRva)||
     !same(controller+ControllerPlayer,source_.player)||!r.Read(controller+ControllerPawn,pawn)||!Typed(r,pawn,NativeClasses[Killer].reflectionCacheRva)||
     !same(pawn+0x110,std::uint8_t{3})||!same(pawn+0x3A0,controller)||!r.Read(controller+0x388,ps)||!ObjectPointer(ps)||
     !same(pawn+0x388,ps)||!same(ps+0x88,controller)||
     !r.Read(world+WorldGameState,state)||!Typed(r,state,disk::SCGameState_SPChallenges,true)||
     !same(state+0x420,pawn)||!same(state+0x490,ps))return false;
  failure=Failure::Metadata;
  // Native PlayerController.NetConnection descriptor is disk-proven at 0x578;
  // name/kind/size/owner and stability are also checked on the current class.
  if(!Field(r,0,controller,u"NetConnection",u"ObjectProperty",cfgmenu::challengeoptions::kReflectedOffset(0x578),at)||!r.Read(at,connection)||connection)return false;
  const std::array<std::uintptr_t,6> objects{{world,mode,controller,pawn,state,ps}};
  failure=Failure::Identity;
  if(!lifetime::ObserveBatch(r,source_.module+lifetime::ObjectArrayRva,objects,w.registry,w.owners))return false;
  for(const auto& slot:w.owners)if(!slot.HasAssignedSerial()||!Flags(r,w.registry,slot))return false;
  failure=Failure::Metadata;
  if(!Field(r,1,state,u"Dossier",u"ObjectProperty",cfgmenu::challengeoptions::kReflectedOffset(0x5B8),at)||!r.Read(at,w.dossier)||!ObjectPointer(w.dossier)||
     !Field(r,2,w.dossier,u"ActiveChallengeClass",u"ClassProperty",cfgmenu::challengeoptions::kReflectedOffset(0x58),at)||
     !r.Read(at,w.missionClass)||!ObjectPointer(w.missionClass))return false;
  // A serial-unassigned dossier is only borrowed from this positive state's
  // current strong field, never retained as an action/lifetime lease.
  lifetime::SlotObservation dossier;
  if(!lifetime::ReadSlot(r,w.registry,w.dossier,dossier)||!Flags(r,w.registry,dossier))return false;
  cfgnames::Observation<R> names(r,source_.module);cfgnames::Text label,package;
  failure=Failure::Mission;
  if(!names.Object(w.missionClass,label)||!cfgmenu::challengeoptions::StockClass(label.View())||
     !r.Read(w.missionClass+0x20,w.missionOuter)||!names.Object(w.missionOuter,package))return false;
  const auto missionName=label.View();
  const std::u16string expected=u"/Game/Blueprints/MapRegistry/Challenges/"+
     std::u16string(missionName.substr(0,missionName.size()-2));
  if(package.View()!=expected||!r.Read(world+0x20,w.mapOuter)||!names.Object(w.mapOuter,package)||
     package.View().find(u"/Game/Maps/Single_Player/")!=0||
     !r.Read(w.mapOuter+cfgnames::disk::ObjectName,w.map)||
     !r.Read(w.missionClass+cfgnames::disk::ObjectName,w.mission)||
     !r.Read(w.missionOuter+cfgnames::disk::ObjectName,w.missionPackage))return false;
  failure=Failure::Phase;
  if(!r.Read(mode+0x3F8,w.phase)||!names.Decode(w.phase,label)||
     (label.View()!=u"InProgress"&&(!intro||label.View()!=u"PreMatchIntro")))return false;
  failure=Failure::Changed;
  if(!lifetime::RevalidateBatch(r,source_.module+lifetime::ObjectArrayRva,w.registry,w.owners)||
     !same(source_.viewport+ViewportWorld,world)||!same(world+WorldGameMode,mode)||!same(world+WorldGameState,state)||
     !same(world+WorldNetDriver,std::uintptr_t{})||!same(controller+0x578,std::uintptr_t{})||
     !same(controller+ControllerPawn,pawn)||!same(pawn+0x3A0,controller)||!same(state+0x5B8,w.dossier)||
     !same(w.dossier+0x58,w.missionClass)||!same(mode+0x3F8,w.phase))return false;
  out=w;failure=Failure::None;return true;
 }
public:
 // Read-only attempt observation for the restart coordinator. This does NOT
 // bind an escape authority or change the InProgress-only action guards.
 template<class R>bool Attempt(R& r,std::uint32_t thread,Witness& out){
  out={};if(!enabled_)return false;Witness a,b;
  if(!Sample(r,thread,a,true)||!Sample(r,thread,b,true)||!(a==b))return false;
  out=b;return true;
 }
 Failure failure=Failure::Disabled;unsigned binds=0,retirements=0;
 bool Armed()const{return enabled_;}
 std::uint64_t Generation()const{return generation_;}
 bool Arm(const cfgmenu::active::Context& source,std::uint64_t generation,bool enabled,std::uint64_t now){
  if(!generation||generation<=generation_||now>UINT64_MAX-90000||!source.thread||!source.module||!source.world)return false;
  source_={source.module,source.engine,source.viewport,source.player,source.gameInstance,source.world,source.thread,false};
  generation_=generation;enabled_=enabled;bound_=false;boundWitness_={};members_={};deadline_=now+90000;
  failure=enabled?Failure::None:Failure::Disabled;return true;
 }
 void Retire(){if(enabled_)++retirements;enabled_=bound_=false;boundWitness_={};members_={};}
 template<class R>bool Observe(R& r,std::uint32_t thread,std::uint64_t now,Witness& out){
  out={};if(!enabled_){failure=Failure::Disabled;return false;}
  if(!bound_&&now>=deadline_){Retire();failure=Failure::Expired;return false;}
  if(bound_){std::uintptr_t world=0;if(!r.Read(source_.viewport+cfgprobe::ViewportWorld,world))return false;
   if(world!=boundWitness_.World()){Retire();failure=Failure::Changed;return false;}}
  Witness first,second;if(!Sample(r,thread,first)||!Sample(r,thread,second)||!(first==second))return false;
  if(bound_&&(first.owners!=boundWitness_.owners||!(first.map==boundWitness_.map)||
      !(first.mission==boundWitness_.mission)||!(first.missionPackage==boundWitness_.missionPackage))){
   Retire();failure=Failure::Mission;return false;
  }
  if(!bound_){boundWitness_=first;bound_=true;++binds;}
  out=second;return true;
 }
 template<class R>bool Current(R& r,std::uint32_t thread,const Witness& expected){
  if(!enabled_||!bound_||expected.generation!=generation_||expected.owners!=boundWitness_.owners||
     !(expected.map==boundWitness_.map)||!(expected.mission==boundWitness_.mission)||
     !(expected.missionPackage==boundWitness_.missionPackage))return false;
  // Full name/path decoding belongs to Observe's two admission samples. During
  // this callback, native FName words and field identities are immutable keys;
  // recheck them, schemas and ownership directly rather than repeatedly walking
  // and decoding the same native metadata around every car action.
  using namespace cfgprobe;const auto same=[&](std::uintptr_t at,auto value){return cfgmenu::offlineowner::Same(r,at,value);};
  const auto world=expected.owners[0].object,mode=expected.owners[1].object,controller=expected.owners[2].object,
   pawn=expected.owners[3].object,state=expected.owners[4].object,ps=expected.owners[5].object;
  int capacity=0;
  if(thread!=source_.thread||!cfgmenu::offlineowner::ChallengeLookupSafe(r,source_.module)||
   !same(source_.module+cfgprobe::dispatch::ThreadFlagRva,std::uint8_t{1})||!same(source_.module+cfgprobe::dispatch::ThreadIdRva,thread)||
   !same(source_.module+EngineGlobalCandidate,source_.engine)||!same(source_.engine+EngineGameViewport,source_.viewport)||
   !same(source_.module+cfgnames::disk::Global,expected.nameTable)||!same(source_.viewport+ViewportWorld,world)||
   !same(world+WorldGameMode,mode)||!same(world+WorldGameState,state)||!same(world+WorldGameInstance,source_.gameInstance)||
   !same(world+WorldNetDriver,std::uintptr_t{})||!same(source_.gameInstance+GameInstanceLocalPlayers,expected.players)||
   !same(source_.gameInstance+GameInstanceLocalPlayers+8,int{1})||!r.Read(source_.gameInstance+GameInstanceLocalPlayers+12,capacity)||capacity<1||capacity>4||
   !same(expected.players,source_.player)||!same(source_.player+PlayerController,controller)||!same(controller+ControllerPlayer,source_.player)||
   !same(controller+ControllerPawn,pawn)||!same(controller+0x388,ps)||!same(controller+0x578,std::uintptr_t{})||
   !same(pawn+0x3A0,controller)||!same(pawn+0x388,ps)||!same(ps+0x88,controller)||
   !same(pawn+0x110,std::uint8_t{3})||!same(mode+0x110,std::uint8_t{3})||!same(state+0x420,pawn)||!same(state+0x490,ps)||
   !Typed(r,mode,disk::SCGameMode_SPChallenges,true)||!Typed(r,state,disk::SCGameState_SPChallenges,true)||
   !Typed(r,controller,NativeClasses[PlayerControllerClass].reflectionCacheRva)||!Typed(r,pawn,NativeClasses[Killer].reflectionCacheRva))return false;
  lifetime::ArrayHeader registry;
  if(!lifetime::ReadArrayHeader(r,source_.module+lifetime::ObjectArrayRva,registry)||!(registry==expected.registry))return false;
  for(const auto& slot:expected.owners){lifetime::SlotObservation fresh;
   if(!ReadIdentity(r,registry,slot.object,fresh)||!(fresh==slot))return false;
  }
  const std::array<std::uintptr_t,3> objects{{controller,state,expected.dossier}};
  cfgreflection::Observation<R> reflection(r,source_.module);
  for(unsigned i=0;i<members_.size();++i){const auto& m=members_[i];const auto& p=m.property;
   if(!m.type||!same(objects[i]+ObjectClass,m.type)||!cfgprobe::StableAncestry(r,objects[i],m.ancestry)||
      !same(p.field.owner+cfgreflection::disk::Children,m.head)||!reflection.CurrentProperty(p)||
      !same(p.field.type+cfgnames::disk::ObjectName,m.kind))return false;
  }
  lifetime::SlotObservation dossier;
  return same(state+0x5B8,expected.dossier)&&lifetime::ReadSlot(r,expected.registry,expected.dossier,dossier)&&Flags(r,expected.registry,dossier)&&
   same(expected.dossier+0x58,expected.missionClass)&&same(expected.missionClass+0x20,expected.missionOuter)&&
   same(world+0x20,expected.mapOuter)&&same(expected.mapOuter+cfgnames::disk::ObjectName,expected.map)&&
   same(expected.missionClass+cfgnames::disk::ObjectName,expected.mission)&&
   same(expected.missionOuter+cfgnames::disk::ObjectName,expected.missionPackage)&&same(mode+0x3F8,expected.phase);
 }
};
}
