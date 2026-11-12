#pragma once
#include "NativeOfflineOwnerObservation.hpp"
#include "../RuntimeProbe/PrimaryJasonResources.hpp"
#include "NativeOfflineSettings.hpp"
#include "NativeHumanJasonDifficulty.hpp"
#include "../Core/JasonRageDifficultyPolicy.hpp"
#include <cmath>
namespace cfgmenu::humanjason {
inline constexpr auto OptionsPurpose=CFG_J214_HUMAN_JASON_OPTIONS?offlineowner::Purpose::TypedOwnValueOptions:offlineowner::Purpose::Runtime;
struct Options {bool knives=false,movement=false,recharge=false;unsigned seconds=0,nativeSkill=3,difficulty=1;};
// CFG property records independently checked by Inspect-CfgNativeJasonExtrasCompatibility.ps1.
constexpr std::uintptr_t Knives=0x169C,Movement=0x3D0,Speeds=0x720;
constexpr std::uintptr_t MovementClass=cfgcompat::cfg35::objects::SCCharacterMovementReflection;
struct Report {unsigned samples=0,knivesWrites=0,movementWrites=0,rechargeWrites=0,durationState=0,skillState=0,failure=0,rageState=0,damageScaled=0;bool armed=false,bound=false,retired=false;};
template<class R,class W,class F>struct DurationHost {
 R& r;W& writer;F& current;
 bool Current(const cfgofflinesettings::Request&){return current();}
 bool BeforeHelpers(const cfgofflinesettings::Request&){return false;}
 cfgofflinesettings::Write Exchange(std::uintptr_t at,std::int32_t old,std::int32_t value){
  // NativeAccess has JUST called Current before each Exchange and calls it
  // again after the write. No native dispatch/yield occurs between that gate
  // and this typed compare/write/readback. Repeating the entire owner census
  // here exhausted the phase budget after the first of the two timer writes.
  return writer.Write(at,old,value)?cfgofflinesettings::Write::Changed:cfgofflinesettings::Write::Indeterminate;
 }
};
template<class Reader,class T>bool Write(Reader& r,std::uintptr_t address,const T& expected,const T& desired){
 MEMORY_BASIC_INFORMATION m{};T actual{};SIZE_T bytes=0;
 if(VirtualQuery(reinterpret_cast<void*>(address),&m,sizeof(m))!=sizeof(m)||m.State!=MEM_COMMIT||m.Type!=MEM_PRIVATE||m.Protect!=PAGE_READWRITE)return false;
 const auto start=reinterpret_cast<std::uintptr_t>(m.BaseAddress);
 if(address<start||sizeof(T)>m.RegionSize||address-start>m.RegionSize-sizeof(T)||!r.Read(address,actual)||actual!=expected)return false;
 return WriteProcessMemory(GetCurrentProcess(),reinterpret_cast<void*>(address),&desired,sizeof(T),&bytes)&&bytes==sizeof(T)&&r.Read(address,actual)&&actual==desired;
}
class Runtime {
 freshgame::Source source_{};Options options_{};std::uint64_t arrival_=0,next_=0;
 cfgprobe::lifetime::SlotObservation world_{},pawn_{},movement_{};bool moved_=false,recharged_=false;
 cfgofflinesettings::State durationState_=cfgofflinesettings::State::Empty;
 cfgnames::Words durationPhase_{};
 std::uintptr_t callbackHint_=0;std::uint64_t hintNext_=0;
 cfgragethreshold::Rage rage_{};DamageWitness damage_{};
public:
 Report report{};
 bool Arm(const active::Context& menu,Options options,std::uint64_t now){
  if(report.armed||!menu.thread||!menu.world||now>UINT64_MAX-90000)return false;
  source_={menu.module,menu.engine,menu.viewport,menu.player,menu.gameInstance,menu.world,menu.thread,true};
  if(options.difficulty>2||options.nativeSkill>3||(options.seconds&&options.seconds!=1800&&options.seconds!=3600&&options.seconds!=5400&&options.seconds!=UINT32_MAX))return false;
  options_=options;arrival_=now+90000;report.armed=true;
  report.retired=!options.knives&&!options.movement&&!options.recharge&&!options.seconds&&options.difficulty==1;return true;
 }
 bool Armed()const{return report.armed&&!report.retired;}
 template<class Reader>float DifficultyDamage(Reader& r,std::uint32_t thread,std::uintptr_t victim,float incoming,
     std::uintptr_t instigator,std::uintptr_t causer){
  if(!rage_.applied||!report.armed)return incoming;
  const float result=damage_.Damage(r,thread,options_.difficulty,victim,incoming,instigator,causer);
  if(result!=incoming)++report.damageScaled;return result;
 }
 // Caller has just verified the actual native frontend route and positive
 // current menu/HUD. No old gameplay writer is invoked on menu return.
 bool ResetForVerifiedMenu(const active::Context& menu){
  if(!report.bound||!world_.HasAssignedSerial()||menu.world==world_.object||
   menu.module!=source_.module||menu.engine!=source_.engine||menu.viewport!=source_.viewport||
   menu.player!=source_.player||menu.gameInstance!=source_.gameInstance||menu.thread!=source_.thread)return false;
  *this=Runtime{};report.retired=true;return true;
 }
 template<class Reader,class Writer>void BeforeIntro(Reader& r,std::uintptr_t mode,std::uint32_t thread,Writer& writer){
  if(!report.armed||report.skillState||options_.nativeSkill==3||thread!=source_.thread)return;
  report.skillState=1;promotion::Context c;
  if(!offlineowner::Observe(r,source_,mode,thread,true,c,nullptr,nullptr,OptionsPurpose))return;
  report.skillState=2;std::uint8_t old=3;std::array<unsigned char,cfgofflinesettings::disk::SkillConsumerCode.size()> code{};
  if(!r.Read(source_.module+cfgofflinesettings::disk::SkillConsumer,code)||code!=cfgofflinesettings::disk::SkillConsumerCode||
     !r.Read(mode+cfgofflinesettings::disk::CounselorSkill,old)||old>2||!offlineowner::Current(r,c,thread,nullptr,OptionsPurpose))return;
  report.skillState=4;
  if(old!=options_.nativeSkill&&!writer.Write(mode+cfgofflinesettings::disk::CounselorSkill,old,static_cast<std::uint8_t>(options_.nativeSkill)))return;
  if(!offlineowner::Current(r,c,thread,nullptr,OptionsPurpose)||!offlineowner::Same(r,mode+cfgofflinesettings::disk::CounselorSkill,static_cast<std::uint8_t>(options_.nativeSkill)))return;
  report.skillState=3;
 }
 template<class Reader>bool Callback(Reader& r,std::uintptr_t object,std::uint64_t now){
  if(!Armed())return false;if(report.bound)return object==pawn_.object;
  if(now>=hintNext_){hintNext_=now+100;std::uintptr_t controller=0;callbackHint_=0;
   if(r.Read(source_.player+cfgprobe::PlayerController,controller)&&cfgprobe::ObjectPointer(controller))r.Read(controller+cfgprobe::ControllerPawn,callbackHint_);}
  return callbackHint_&&object==callbackHint_;
 }
 template<class Reader,class Writer>void Step(Reader& r,std::uintptr_t callback,std::uint32_t thread,std::uint64_t now,Writer& writer){
  using namespace cfgprobe;
  if(!Armed()||thread!=source_.thread||now<next_)return;next_=now+100;
  const bool pendingOnce=(options_.movement&&!moved_)||(options_.recharge&&!recharged_)||
      (options_.difficulty!=1&&!rage_.attempted)||
      (options_.seconds&&(durationState_==cfgofflinesettings::State::Empty||durationState_==cfgofflinesettings::State::Waiting||durationState_==cfgofflinesettings::State::Applying));
  if(now>=arrival_&&(!report.bound||pendingOnce)){
   report.retired=true;report.failure=1;
   if(durationState_==cfgofflinesettings::State::Applying){
    durationState_=cfgofflinesettings::State::Indeterminate;report.durationState=static_cast<unsigned>(durationState_);report.failure=9;
   }return;
  }
  std::uintptr_t world=0,mode=0,controller=0,pawn=0;
  if(!r.Read(source_.viewport+ViewportWorld,world))return;
  if(report.bound&&world!=world_.object){report.retired=true;return;}
  if(world==source_.frontendWorld)return;
  if(!r.Read(source_.player+PlayerController,controller)||!ObjectPointer(controller)||
     !r.Read(controller+ControllerPawn,pawn)||pawn!=callback||!ObjectPointer(pawn)||!r.Read(world+WorldGameMode,mode))return;
  ++report.samples;report.failure=2;promotion::Context c;
  if(!offlineowner::Observe(r,source_,mode,thread,true,c,nullptr,nullptr,OptionsPurpose))return;
  std::uintptr_t killer=0,ctor=0,state=0,defaults=0,type=0;cfgprobe::lifetime::SlotObservation identity;
  report.failure=3;
  if(!r.Read(source_.module+birthdisk::SCKillerCharacterClass,killer)||!r.Read(source_.module+birthdisk::SCKillerCharacterConstructor,ctor)||
     !MatchingClassCache(killer,ctor)||!cfgspawn::ObjectIs(r,pawn,killer)||!r.Read(pawn+ObjectClass,type)||
     !r.Read(type+0xF8,defaults)||defaults==pawn||!offlineowner::Same(r,pawn+0x3A0,controller)||
     !offlineowner::Same(r,pawn+0x388,c.playerState)||!r.Read(world+WorldGameState,state)||
     !offlineowner::Same(r,state+cfgprimary::CurrentKiller,pawn)||!offlineowner::Same(r,state+cfgprimary::CurrentKillerState,c.playerState)||
     !lifetime::ReadSlot(r,c.registry,pawn,identity)||!identity.HasAssignedSerial())return;
  if(report.bound&&(!(identity==pawn_)||!(c.identities[0]==world_))){report.retired=true;report.failure=4;return;}
  if(!report.bound){world_=c.identities[0];pawn_=identity;report.bound=true;}
  const auto current=[&](){cfgprobe::lifetime::SlotObservation again;return offlineowner::Current(r,c,thread,nullptr,OptionsPurpose)&&
    lifetime::ReadSlot(r,c.registry,pawn,again)&&again==identity&&offlineowner::Same(r,controller+ControllerPawn,pawn)&&
    offlineowner::Same(r,pawn+0x3A0,controller)&&offlineowner::Same(r,pawn+0x388,c.playerState)&&
    offlineowner::Same(r,world+WorldGameState,state)&&offlineowner::Same(r,state+cfgprimary::CurrentKiller,pawn)&&
    offlineowner::Same(r,state+cfgprimary::CurrentKillerState,c.playerState)&&cfgspawn::ObjectIs(r,pawn,killer);};
  if(options_.difficulty!=1&&!rage_.attempted){
   float before=0;report.failure=12;
   if(!r.Read(pawn+cfgofflinesettings::disk::RageUnlockTime,before)||
      !rage_.Prepare(pawn,identity.serial,options_.difficulty,before)||!current()||!damage_.Bind(r,c,identity,state,OptionsPurpose)){
    rage_.attempted=true;report.rageState=3;report.retired=true;return;
   }
   // Consume before the CAS: an ambiguous result can never rescale this pawn.
   rage_.attempted=true;report.rageState=1;
   rage_.applied=(before==rage_.desired||writer.Write(pawn+cfgofflinesettings::disk::RageUnlockTime,before,rage_.desired))&&
      offlineowner::Same(r,pawn+cfgofflinesettings::disk::RageUnlockTime,rage_.desired)&&current();
   report.rageState=rage_.applied?2u:3u;report.failure=rage_.applied?0u:12u;
   if(!rage_.applied)report.retired=true;return;
  }
  if(options_.movement&&!moved_){
   std::uintptr_t movement=0,movementType=0;std::array<float,2> speed{};
   report.failure=5;
   if(!r.Read(pawn+Movement,movement)||!r.Read(source_.module+MovementClass,movementType)||
      !offlineowner::Same(r,source_.module+MovementClass+8,movementType)||!cfgspawn::ObjectIs(r,movement,movementType)||
      !offlineowner::Same(r,movement+0x20,pawn)||!lifetime::ReadSlot(r,c.registry,movement,movement_)||
      !r.Read(movement+Speeds,speed)||!std::isfinite(speed[0])||!std::isfinite(speed[1])||speed[0]<=0||speed[1]<=0||speed[0]>5000||speed[1]>5000||!current())return;
   cfgprobe::lifetime::SlotObservation again;
   std::uint32_t flags=0;
   if(!lifetime::ReadSlot(r,c.registry,movement,again)||!(again==movement_)||
      !r.Read(c.registry.data+24*static_cast<std::uintptr_t>(again.index)+8,flags)||(flags&0x20000000)||
      !offlineowner::Same(r,pawn+Movement,movement)||!offlineowner::Same(r,movement+0x20,pawn)||!cfgspawn::ObjectIs(r,movement,movementType))return;
   const std::array<float,2> desired{{speed[0]*2,speed[1]*2}};
   // Publish before write: ambiguous failure must never multiply a second time.
   moved_=true;if(!writer.Write(movement+Speeds,speed,desired)){report.retired=true;report.failure=6;return;}
   ++report.movementWrites;report.failure=0;return;
  }
  if(options_.recharge&&!recharged_){
   std::array<float,5> old{},desired{};report.failure=10;
   if(!r.Read(pawn+0x129C,old)||!current())return;
   for(unsigned i=0;i<old.size();++i){if(!std::isfinite(old[i])||old[i]<=0||old[i]>3600)return;desired[i]=old[i]*0.5f;}
   recharged_=true;
   if(!writer.Write(pawn+0x129C,old,desired)){report.retired=true;report.failure=11;return;}
   ++report.rechargeWrites;report.failure=0;return;
  }
  if(options_.seconds&&(durationState_==cfgofflinesettings::State::Empty||durationState_==cfgofflinesettings::State::Waiting||durationState_==cfgofflinesettings::State::Applying)){
   cfgofflinesettings::Request request{1,c.world,pawn,world_.serial,pawn_.serial,options_.seconds,3};
   using S=cfgofflinesettings::State;
   if(durationState_==S::Empty)durationState_=S::Waiting;
   const bool partial=durationState_==S::Applying;
   const auto refuse=[&](){durationState_=partial?S::Indeterminate:S::Refused;};
   DurationHost<Reader,Writer,decltype(current)> host{r,writer,current};
   cfgofflinesettings::NativeAccess<Reader,decltype(host)> access(r,host,request,{source_.module,c.mode,state,true});
   cfgofflinesettings::Snapshot snapshot;bool ready=false;
   if(now>=arrival_||!request.Valid()||!access.Observe(snapshot,ready))refuse();
   else if(!ready){if(partial)refuse();}
   else if(partial&&!(snapshot.phase==durationPhase_))refuse();
   else {
    const auto result=access.ApplyPart(snapshot,partial);
    if(result==cfgofflinesettings::Write::Changed){durationPhase_=snapshot.phase;durationState_=partial?S::Applied:S::Applying;}
    else if(result==cfgofflinesettings::Write::Indeterminate)durationState_=S::Indeterminate;
    else refuse();
   }
   report.durationState=static_cast<unsigned>(durationState_);
   report.failure=report.durationState>=static_cast<unsigned>(cfgofflinesettings::State::Refused)?9u:0u;return;
  }
  if(options_.knives){
   int count=0;report.failure=7;if(!r.Read(pawn+Knives,count)||count<0||count>9999||!current())return;
   if(count<99){if(!writer.Write(pawn+Knives,count,99)){report.retired=true;report.failure=8;return;}++report.knivesWrites;}
  }
  report.failure=report.durationState>=static_cast<unsigned>(cfgofflinesettings::State::Refused)?9u:0u;
  if(!options_.knives&&(!options_.movement||moved_)&&(!options_.recharge||recharged_))report.retired=true;
 }
};
}
