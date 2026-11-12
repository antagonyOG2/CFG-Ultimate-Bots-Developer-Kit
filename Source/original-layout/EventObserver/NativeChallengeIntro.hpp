#pragma once
#include "NativeHumanJasonOptionsRuntime.hpp"
#include "PostMatchRecoveryObservation.hpp"
#include "../RuntimeProbe/MenuScriptObservation.hpp"
#include "../RuntimeProbe/Compatibility/CfgBuildProfile.hpp"
#include "../Core/ChallengeIntroSkipPolicy.hpp"
#include "../Core/ChallengeIntroDispatchPolicy.hpp"
#include "../Core/ChallengeNativeScopePolicy.hpp"
#include "../RuntimeProbe/ChallengeIntro.generated.hpp"
#include "../RuntimeProbe/ScopedLookupEligibility.hpp"
#include "../Core/J24FeatureFlags.hpp"
namespace cfgmenu::challengeintro {
// Standalone stock Challenges only, independent of whether modifiers are On.
// Native sequence skip is shared with outro; exact PreMatchIntro + native
// intro-active/outro-inactive witnesses must still hold at the dispatch edge.
class Runtime {
 freshgame::Source source_{};bool armed_=false,replay_=false;std::uint64_t next_=0;
 template<class R>bool Typed(R& r,std::uintptr_t object,std::uintptr_t cache,bool exact=false){
  std::uintptr_t type=0,ctor=0,actual=0,defaults=0;
  return cfgprobe::ObjectPointer(object)&&r.Read(source_.module+cache,type)&&r.Read(source_.module+cache+8,ctor)&&
   cfgprobe::MatchingClassCache(type,ctor)&&r.Read(object+cfgprobe::ObjectClass,actual)&&
   (exact?actual==type:maps::Subclass(r,actual,type))&&r.Read(actual+cfgspawn::ClassDefaultObject,defaults)&&
   cfgprobe::ObjectPointer(defaults)&&object!=defaults;
 }
public:
 InputPolicy input;DispatchReceipt dispatch{};unsigned failure=0,lastRefusal=0,ownerFailure=0,warmAttempts=0,warmReturns=0,peakReads=0,scopeFailure=0,introValidateStage=0;
 // replaySkip: donor Skip Intro on Replay. Only the restart coordinator's
 // positively proved same-mission fresh attempt requests it; it uses the same
 // owner/phase/intro-active proofs as a Start press and is consumed once.
 void Arm(const active::Context& source,bool replaySkip=false){
  source_={source.module,source.engine,source.viewport,source.player,source.gameInstance,source.world,source.thread,false};
  armed_=true;replay_=replaySkip;next_=0;input={};dispatch={};failure=lastRefusal=ownerFailure=warmAttempts=warmReturns=peakReads=scopeFailure=introValidateStage=0;
 }
 void Cancel(){armed_=false;replay_=false;input.pending=false;dispatch.Cancel();}
 bool Armed()const{return armed_;}
 bool ReplayPending()const{return armed_&&replay_;}
 bool PollDue(std::uint64_t now)const{return armed_&&now>=next_;}
 // This read-only consumer proof deliberately does not borrow the menu HUD or
 // Offline Bots bundle. Failure is a refusal, never authority to dispatch.
 template<class R>bool CurrentRequest(R& r,const DispatchRequest& request,std::uintptr_t callback,
     std::uint32_t thread,unsigned& stage){
  using namespace cfgprobe;stage=1;
  if(!request.Valid()||request.module!=source_.module||thread!=source_.thread||request.thread!=thread)return false;
  std::uintptr_t world=0,mode=0,state=0,controller=0,players=0,type=0,level=0;int count=0,capacity=0;
  const auto same=[&](std::uintptr_t at,auto value){return offlineowner::Same(r,at,value);};
  stage=2;if(!cfgprimary::LookupVetoClearForObservation(r,source_.module)||
     !same(source_.module+dispatch::ThreadFlagRva,std::uint8_t{1})||!same(source_.module+dispatch::ThreadIdRva,thread)||
     !same(source_.module+EngineGlobalCandidate,source_.engine)||!same(source_.engine+EngineGameViewport,source_.viewport)||
     !r.Read(source_.viewport+ViewportWorld,world)||world!=request.world||world==source_.frontendWorld||
     !same(world+WorldNetDriver,std::uintptr_t{})||!r.Read(world+WorldGameMode,mode)||mode!=request.mode||
     !r.Read(world+WorldGameState,state)||!same(world+WorldGameInstance,source_.gameInstance))return false;
  stage=3;if(!r.Read(source_.gameInstance+GameInstanceLocalPlayers,players)||!ObjectPointer(players)||
     !r.Read(source_.gameInstance+GameInstanceLocalPlayers+8,count)||count!=1||
     !r.Read(source_.gameInstance+GameInstanceLocalPlayers+12,capacity)||capacity<1||capacity>4||
     !same(players,source_.player)||!r.Read(source_.player+PlayerController,controller)||
     !same(controller+ControllerPlayer,source_.player)||
     (callback!=mode&&callback!=controller&&callback!=state))return false;
  stage=4;if(!Typed(r,mode,NativeClasses[ChallengesMode].reflectionCacheRva,true)||
     !Typed(r,controller,NativeClasses[PlayerControllerClass].reflectionCacheRva)||
     !Typed(r,state,cfgcompat::cfg35::objects::SCGameStateReflection)||!same(mode+0x110,std::uint8_t{3}))return false;
  cfgnames::Observation<R> names(r,source_.module);cfgnames::Text label;cfgnames::Words phase{};
  stage=5;if(!r.Read(state+ObjectClass,type)||!names.Object(type,label)||label.View()!=u"SCGameState_SPChallenges"||
     !r.Read(world+0x20,level)||!names.Object(level,label)||label.View().find(u"/Game/Maps/Single_Player/")!=0||
     !r.Read(mode+0x3F8,phase)||!names.Decode(phase,label)||label.View()!=u"PreMatchIntro")return false;
  stage=6;std::array<std::uintptr_t,4> objects{{world,mode,controller,state}};
  lifetime::ArrayHeader registry{};std::array<lifetime::SlotObservation,4> slots{};
  if(!lifetime::ObserveBatch(r,source_.module+lifetime::ObjectArrayRva,objects,registry,slots))return false;
  for(const auto& slot:slots)if(!slot.HasAssignedSerial())return false;
  if(slots[0].serial!=request.worldSerial)return false;
  cfgoutro::Owner owner;unsigned ownerStage=0;
  stage=7;if(!cfgoutro::ObserveOwner(r,registry,source_.module,world,owner,&ownerStage)||
      owner.settings.object!=request.settings||owner.level.object!=request.level||
      owner.settings.serial!=request.settingsSerial||owner.level.serial!=request.levelSerial||
      (request.strongChildren&& (owner.settings.index!=request.settingsIndex||owner.level.index!=request.levelIndex))||
      (!request.strongChildren&&(!owner.settings.HasAssignedSerial()||!owner.level.HasAssignedSerial()))||
      (request.strongChildren&&(!CFG_J33_INTRO_STRONG_CHILDREN||
       !same(world+cfgoutro::disk::PersistentLevel,request.level)||
       !same(request.level+cfgoutro::disk::WorldSettings,request.settings)||
       !same(request.level+cfgoutro::disk::OwningWorld,request.world))))return false;
  std::uint32_t levelFlags=0,settingsFlags=0;
  stage=8;if(!r.Read(registry.data+24ULL*static_cast<unsigned>(owner.level.index)+8,levelFlags)||
     !r.Read(registry.data+24ULL*static_cast<unsigned>(owner.settings.index)+8,settingsFlags)||
     ((levelFlags|settingsFlags)&0x30000000u)||!same(owner.settings.object+0x729,std::uint8_t{1})||
     !same(owner.settings.object+0x7C8,std::uint8_t{}))return false;
  stage=9;std::uintptr_t function=0;if(!r.Read(source_.module+cfgoutro::disk::SkipCache,function)||function!=request.function)return false;
  cfgreflection::Observation<R> reflection(r,source_.module);cfgreflection::Field field;
  if(!reflection.ReadField(function,owner.type,field)||field.kind.View()!=u"Function"||
     field.label.View()!=u"SkipLevelIntroOutro")return false;
  std::uint32_t flags=0;std::uint8_t parms=1;std::uint16_t bytes=1;
  stage=10;if(!same(function+0x20,owner.type)||!r.Read(function+0x88,flags)||!(flags&0x400u)||
     !r.Read(function+0x8C,parms)||parms||!r.Read(function+0x8E,bytes)||bytes||
     !same(function+0xB0,source_.module+cfgoutro::disk::SkipExecutor))return false;
  std::array<unsigned char,cfgoutro::disk::SkipExecutorCode.size()> executor{};
  std::array<unsigned char,cfgoutro::disk::SkipNativeCode.size()> native{};
  stage=11;if(!r.ReadBytes(source_.module+cfgoutro::disk::SkipExecutor,executor.data(),executor.size())||
     executor!=cfgoutro::disk::SkipExecutorCode||
     !r.ReadBytes(source_.module+cfgoutro::disk::SkipNative,native.data(),native.size())||
     native!=cfgoutro::disk::SkipNativeCode)return false;
  stage=12;return lifetime::RevalidateBatch(r,source_.module+lifetime::ObjectArrayRva,registry,slots)&&
      same(source_.viewport+ViewportWorld,world)&&same(world+WorldNetDriver,std::uintptr_t{})&&
      same(world+WorldGameMode,mode)&&same(world+WorldGameState,state)&&same(mode+0x3F8,phase)&&
      same(source_.module+cfgoutro::disk::SkipCache,function)&&cfgprimary::LookupVetoClearForObservation(r,source_.module);
 }
 template<class R,class Engine>void Step(R& r,Engine& engine,std::uintptr_t callback,std::uint32_t thread,
    std::uint64_t now,bool foreground,bool down){
  if(!armed_||thread!=source_.thread||now<next_)return;
  using namespace cfgprobe;
  const auto same=[&](std::uintptr_t at,auto value){return offlineowner::Same(r,at,value);};
  std::uintptr_t world=0,mode=0,state=0,controller=0,players=0,outer=0;int count=0,capacity=0;
  cfgnames::Words phase{};cfgnames::Text label;cfgnames::Observation<R> names(r,source_.module);
  failure=1;
  if(!r.Read(source_.viewport+ViewportWorld,world)||world==source_.frontendWorld)return;
  if(!r.Read(world+WorldGameMode,mode))return;
  // An unrelated actor must not spend the owner's50ms opportunity. Input is
  // sampled separately; every due callback gets only this cheap source hint.
  // Full positive ownership proof remains mandatory before a native action.
  if(callback!=mode){
   if(!r.Read(world+WorldGameState,state)||!r.Read(source_.player+PlayerController,controller)||
      (callback!=state&&callback!=controller))return;
  }
  next_=now+50;
  if(!r.Read(mode+0x3F8,phase)||!names.Decode(phase,label))return;
  const bool intro=label.View()==u"PreMatchIntro";
  input.Observe(world,intro,foreground,down,now);
  if(!intro){if(label.View()==u"InProgress"||label.View()==u"PostMatchOutro"||label.View()==u"LeavingMap"){armed_=false;replay_=false;}failure=0;return;}
  if(!input.Due(now)&&!replay_){failure=0;return;}
  struct RefusalReceipt {Runtime& runtime;~RefusalReceipt(){if(runtime.failure)runtime.lastRefusal=runtime.failure;}} receipt{*this};
  failure=2;
  // Read-only veto is a prerequisite for collecting the owner witness; it
  // never authorizes a native action. Strict LookupSafe is repeated inside
  // the positively admitted scope below, before factory or skip dispatch.
  if(CFG_J29_CHALLENGE_INTRO_SCOPE&&!cfgprimary::LookupVetoClearForObservation(r,source_.module)){
   failure=8;scopeFailure=1;return; // active GC/serialization: not even a read-only witness
  }
  if((!CFG_J29_CHALLENGE_INTRO_SCOPE&&!offlineowner::LookupSafe(r,source_.module))||
   !same(source_.module+dispatch::ThreadFlagRva,std::uint8_t{1})||
   !same(source_.module+dispatch::ThreadIdRva,thread)||!same(source_.module+EngineGlobalCandidate,source_.engine)||
   !same(source_.engine+EngineGameViewport,source_.viewport)||!same(world+WorldNetDriver,std::uintptr_t{})||
   !same(world+WorldGameInstance,source_.gameInstance)||!Typed(r,mode,NativeClasses[ChallengesMode].reflectionCacheRva,true)||!same(mode+0x110,std::uint8_t{3})||
   !r.Read(source_.gameInstance+GameInstanceLocalPlayers,players)||!ObjectPointer(players)||
   !r.Read(source_.gameInstance+GameInstanceLocalPlayers+8,count)||count!=1||
   !r.Read(source_.gameInstance+GameInstanceLocalPlayers+12,capacity)||capacity<1||capacity>4||!same(players,source_.player)||
   !r.Read(source_.player+PlayerController,controller)||!Typed(r,controller,NativeClasses[PlayerControllerClass].reflectionCacheRva)||
   !same(controller+ControllerPlayer,source_.player)||!r.Read(world+WorldGameState,state)||!Typed(r,state,cfgcompat::cfg35::objects::SCGameStateReflection)||
   !r.Read(state+ObjectClass,outer)||!names.Object(outer,label)||label.View()!=u"SCGameState_SPChallenges"||
   !r.Read(world+0x20,outer)||!names.Object(outer,label)||label.View().find(u"/Game/Maps/Single_Player/")!=0)return;
  // Only an actual callback on the current native mode/controller/state is
  // authority here. No departed actor, arbitrary instance or broad tick hook.
  if(callback!=mode&&callback!=controller&&callback!=state){failure=3;return;}
  std::array<std::uintptr_t,4> objects{{world,mode,controller,state}};
  lifetime::ArrayHeader registry;std::array<lifetime::SlotObservation,4> slots{};
  failure=4;if(!lifetime::ObserveBatch(r,source_.module+lifetime::ObjectArrayRva,objects,registry,slots))return;
  for(const auto& slot:slots)if(!slot.HasAssignedSerial())return;
  cfgoutro::Owner owner;
  if(!cfgoutro::ObserveOwner(r,registry,source_.module,world,owner,&ownerFailure))return;
  // Borrow only inside THIS callback from positive current world -> typed
  // native PersistentLevel -> WorldSettings strong links. UE can leave these
  // reachable children serial-unassigned; don't turn zero into a lease or
  // call a serial allocator. ObserveOwner rechecks exact slot/strong links;
  // current() repeats them before every factory/action. Nothing retained here.
  const auto flagsCurrent=[&](){std::uint32_t levelFlags=0,settingsFlags=0;
   return r.Read(registry.data+24ULL*static_cast<unsigned>(owner.level.index)+8,levelFlags)&&
    r.Read(registry.data+24ULL*static_cast<unsigned>(owner.settings.index)+8,settingsFlags)&&
    !(levelFlags&0x30000000u)&&!(settingsFlags&0x30000000u);};
  if(!flagsCurrent()){ownerFailure=5;return;}
  // CFG 3.5 has no global runtime lookup permission. An intro Start callback
  // must earn its Reader-typed scope from THIS independently proved native
  // mode/controller/state and the current world->level->settings strong owner.
  // The validator deliberately uses only the veto, not LookupSafe (which would
  // recursively ask this same scope for permission). Every subsequent lookup
  // repeats these proofs, including the offline gate and positive serials.
  auto scopeCurrent=[&](R& probe){
   if(&probe!=&r||thread!=source_.thread||(callback!=mode&&callback!=controller&&callback!=state))return false;
   const auto valid=[&](std::uintptr_t at,auto value){return offlineowner::Same(probe,at,value);};
   cfgnames::Observation<R> localNames(probe,source_.module);cfgnames::Text actualName;
   cfgoutro::Owner fresh;std::uintptr_t type=0,level=0;std::uint32_t levelFlags=0,settingsFlags=0;
   return cfgprimary::LookupVetoClearForObservation(probe,source_.module)&&
    valid(source_.module+dispatch::ThreadFlagRva,std::uint8_t{1})&&valid(source_.module+dispatch::ThreadIdRva,thread)&&
    valid(source_.module+EngineGlobalCandidate,source_.engine)&&valid(source_.engine+EngineGameViewport,source_.viewport)&&
    valid(source_.viewport+ViewportWorld,world)&&world!=source_.frontendWorld&&
    valid(world+WorldNetDriver,std::uintptr_t{})&&valid(world+WorldGameMode,mode)&&valid(world+WorldGameState,state)&&
    valid(world+WorldGameInstance,source_.gameInstance)&&
    valid(source_.gameInstance+GameInstanceLocalPlayers,players)&&valid(source_.gameInstance+GameInstanceLocalPlayers+8,count)&&
    count==1&&capacity>=1&&capacity<=4&&valid(source_.gameInstance+GameInstanceLocalPlayers+12,capacity)&&
    valid(players,source_.player)&&valid(source_.player+PlayerController,controller)&&
    valid(controller+ControllerPlayer,source_.player)&&valid(mode+0x110,std::uint8_t{3})&&
    valid(mode+0x3F8,phase)&&Typed(probe,mode,NativeClasses[ChallengesMode].reflectionCacheRva,true)&&
    Typed(probe,controller,NativeClasses[PlayerControllerClass].reflectionCacheRva)&&
    Typed(probe,state,cfgcompat::cfg35::objects::SCGameStateReflection)&&
    probe.Read(state+ObjectClass,type)&&localNames.Object(type,actualName)&&actualName.View()==u"SCGameState_SPChallenges"&&
    probe.Read(world+0x20,level)&&localNames.Object(level,actualName)&&actualName.View().find(u"/Game/Maps/Single_Player/")==0&&
    lifetime::RevalidateBatch(probe,source_.module+lifetime::ObjectArrayRva,registry,slots)&&
    cfgoutro::ObserveOwner(probe,registry,source_.module,world,fresh,&ownerFailure)&&
    fresh.settings==owner.settings&&fresh.level==owner.level&&
    probe.Read(registry.data+24ULL*static_cast<unsigned>(owner.level.index)+8,levelFlags)&&
    probe.Read(registry.data+24ULL*static_cast<unsigned>(owner.settings.index)+8,settingsFlags)&&
    !(levelFlags&0x30000000u)&&!(settingsFlags&0x30000000u)&&
    valid(owner.settings.object+0x729,std::uint8_t{1})&&valid(owner.settings.object+0x7C8,std::uint8_t{});
  };
  using ScopeCurrent=decltype(scopeCurrent);
  const bool admitted=CFG_J29_CHALLENGE_INTRO_SCOPE&&scopeCurrent(r);
  cfgprimary::ScopedLookupEligibility<R> lookup(source_.module,&scopeCurrent,
   [](R& reader,void* context){return (*static_cast<ScopeCurrent*>(context))(reader);},admitted);
  if(CFG_J29_CHALLENGE_INTRO_SCOPE){
   failure=8; // Scope-specific refusal; existing LastRefusal field remains sticky.
   if(!admitted){scopeFailure=1;return;} // current owner or GC/serialization veto failed
   if(!lookup.Entered()){scopeFailure=2;return;} // foreign/reentrant lookup scope occupied
   if(!offlineowner::LookupSafe(r,source_.module)){scopeFailure=3;return;} // scope reproof failed
   scopeFailure=0;
  }
  const auto current=[&](){cfgoutro::Owner fresh;
   return offlineowner::LookupSafe(r,source_.module)&&same(source_.module+dispatch::ThreadFlagRva,std::uint8_t{1})&&
    same(source_.module+dispatch::ThreadIdRva,thread)&&same(source_.module+EngineGlobalCandidate,source_.engine)&&
    same(source_.engine+EngineGameViewport,source_.viewport)&&
    lifetime::RevalidateBatch(r,source_.module+lifetime::ObjectArrayRva,registry,slots)&&
    same(source_.viewport+ViewportWorld,world)&&same(world+WorldNetDriver,std::uintptr_t{})&&same(world+WorldGameMode,mode)&&
    same(world+WorldGameState,state)&&same(world+WorldGameInstance,source_.gameInstance)&&same(source_.player+PlayerController,controller)&&
    same(controller+ControllerPlayer,source_.player)&&same(players,source_.player)&&
    same(source_.gameInstance+GameInstanceLocalPlayers,players)&&same(source_.gameInstance+GameInstanceLocalPlayers+8,count)&&
    same(source_.gameInstance+GameInstanceLocalPlayers+12,capacity)&&same(mode+0x3F8,phase)&&Typed(r,mode,NativeClasses[ChallengesMode].reflectionCacheRva,true)&&
    Typed(r,controller,NativeClasses[PlayerControllerClass].reflectionCacheRva)&&Typed(r,state,cfgcompat::cfg35::objects::SCGameStateReflection)&&
    cfgoutro::ObserveOwner(r,registry,source_.module,world,fresh,&ownerFailure)&&fresh.settings==owner.settings&&fresh.level==owner.level&&flagsCurrent()&&
    same(owner.settings.object+0x729,std::uint8_t{1})&&same(owner.settings.object+0x7C8,std::uint8_t{});};
  failure=5;if(!current())return;
  std::uintptr_t function=0;if(!r.Read(source_.module+cfgoutro::disk::SkipCache,function))return;
  if(!function){
   if(warmAttempts)return;
   std::array<unsigned char,disk::Factory.size()> code{};
   if(!r.ReadBytes(source_.module+cfgoutro::disk::SkipFactory,code.data(),code.size())||code!=disk::Factory||!current()||!engine.BeginPhase())return;
   ++warmAttempts;const bool returned=engine.Factory(source_.module+cfgoutro::disk::SkipFactory,function);
   if(returned&&function&&same(source_.module+cfgoutro::disk::SkipCache,function))++warmReturns;
   return; // input retained for bounded next callback; never action after registration
  }
  failure=6;cfgreflection::Observation<R> reflection(r,source_.module);cfgreflection::Field field;
  std::uint32_t flags=0;std::uint8_t parms=1;std::uint16_t bytes=1;int storage=1;std::uintptr_t cleanup=1;
  capture::ScriptHeader script{};
  std::array<unsigned char,cfgoutro::disk::SkipExecutorCode.size()> executor{};
  // Every check has a stable diagnostic stage. Leave the failed stage sticky
  // after the intro ends; a successful complete proof resets it to zero.
  introValidateStage=1;if(!reflection.ReadField(function,owner.type,field))return;
  introValidateStage=2;if(field.kind.View()!=u"Function")return;
  introValidateStage=3;if(field.label.View()!=u"SkipLevelIntroOutro")return;
  if(CFG_J30_INTRO_SKIP_35_VALIDATE){
   introValidateStage=4;if(!same(function+0x20,owner.type))return;
  }
  introValidateStage=5;
  if(!r.Read(function+0x88,flags)||(CFG_J30_INTRO_SKIP_35_VALIDATE?!(flags&0x400u):flags!=0x04020401u))return;
  introValidateStage=6;if(!r.Read(function+0x8C,parms)||parms)return;
  introValidateStage=7;if(!r.Read(function+0x8E,bytes)||bytes)return;
  if(!CFG_J30_INTRO_SKIP_35_VALIDATE){
   // Preserve the old J29 CFG 3.4 metadata and script checks exactly when OFF.
   introValidateStage=8;if(!r.Read(function+cfgreflection::disk::StorageBytes,storage)||storage)return;
   introValidateStage=9;if(!r.Read(function+0x68,cleanup)||cleanup)return;
   introValidateStage=10;if(!r.Read(function+scriptdisk::Script,script)||script.data||script.count||script.capacity)return;
  }
  introValidateStage=11;if(!same(function+0xB0,source_.module+cfgoutro::disk::SkipExecutor))return;
  introValidateStage=12;
  if(!r.ReadBytes(source_.module+cfgoutro::disk::SkipExecutor,executor.data(),executor.size())||executor!=cfgoutro::disk::SkipExecutorCode)return;
  introValidateStage=13;
  if(CFG_J30_INTRO_SKIP_35_VALIDATE){
   // The existing successful 3.5 outro path pins this same native prefix.
   std::array<unsigned char,cfgoutro::disk::SkipNativeCode.size()> native{};
   if(!r.ReadBytes(source_.module+cfgoutro::disk::SkipNative,native.data(),native.size())||native!=cfgoutro::disk::SkipNativeCode)return;
  }else{
   std::array<unsigned char,disk::Native.size()> native{};
   if(!r.ReadBytes(source_.module+cfgoutro::disk::SkipNative,native.data(),native.size())||native!=disk::Native)return;
  }
  introValidateStage=14;if(!same(source_.module+cfgoutro::disk::SkipCache,function))return;
  introValidateStage=15;if(!current())return;
  introValidateStage=16;
  if(CFG_J31_INTRO_SKIP_GAMEPLAY_DISPATCH){
   // Do not borrow frontend-menu retention from this gameplay callback.
   // Queue a finite single-use request only after ALL existing proofs pass.
   // The J29 observation has already proved the exact world->level->settings
   // links, pinned their current registry slots and vetoed GC flags. Transfer
   // only their identity/index evidence, not a lease or serial allocation.
   const bool strongChildren=owner.settings.serial==0||owner.level.serial==0;
   const DispatchRequest request{source_.module,world,mode,owner.settings.object,owner.level.object,function,
       slots[0].serial,owner.settings.serial,owner.level.serial,thread,now+1000,
       strongChildren?owner.level.index:-1,strongChildren?owner.settings.index:-1,strongChildren};
   if(!dispatch.Post(request,now)){failure=17;return;}
   introValidateStage=0;input.Consume();armed_=false;replay_=false;failure=0;return;
  }
  if(!engine.BeginPhase())return;
  introValidateStage=0;
  // Consumed before the call: synchronous native transition or exception must
  // never replay a shared skip into an outro. Native dispatcher packet is empty.
  input.Consume();armed_=false;replay_=false;std::array<unsigned char,16> packet{};
  if(engine.Invoke(owner.settings.object,function,packet.data())){++input.returns;failure=0;}
  else failure=7;
 }
};
}
