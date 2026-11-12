#pragma once
#include "NativeOfflineOwnerObservation.hpp"
#include "CounselorTerminalObservation.hpp"
#include "HeldWeaponObservation.hpp"
#include "DifficultyPolicy.hpp"
#include "../RuntimeProbe/KnifeSequenceBindings.generated.hpp"
#include "../RuntimeProbe/ConvergenceBindings.generated.hpp"
namespace cfgmenu::humanjason {
// Captured only by the existing fully proved native OfflineBots human-Jason
// arrival. Hit-time observations never scan rosters/world levels or call native
// getters, allocate, retain outputs or acquire native-call authority.
struct DamageWitness {
 promotion::Context owner{};
 cfgprobe::lifetime::SlotObservation pawn{},state{};
 bool bound=false;
 template<class R>static bool SameSlot(R& r,const cfgprobe::lifetime::ArrayHeader& registry,
     const cfgprobe::lifetime::SlotObservation& expected){
  cfgprobe::lifetime::SlotObservation actual;
  return expected.HasAssignedSerial()&&cfgspawn::ObjectSlot(r,registry,expected.object,actual)&&actual==expected;
 }
 template<class R>bool Bind(R& r,const promotion::Context& c,
     const cfgprobe::lifetime::SlotObservation& killer,std::uintptr_t gameState,
     offlineowner::Purpose purpose=offlineowner::Purpose::Runtime){
  if(bound||!killer.HasAssignedSerial()||!offlineowner::Current(r,c,c.thread,nullptr,purpose))return false;
  cfgprobe::lifetime::SlotObservation s;
  if(!cfgspawn::ObjectSlot(r,c.registry,gameState,s)||!s.HasAssignedSerial()||
     !offlineowner::Same(r,c.world+cfgprobe::WorldGameState,gameState)||
     !offlineowner::Same(r,gameState+cfgprimary::CurrentKiller,killer.object)||
     !offlineowner::Same(r,gameState+cfgprimary::CurrentKillerState,c.playerState))return false;
  owner=c;pawn=killer;state=s;bound=true;return true;
 }
 template<class R>bool Current(R& r,std::uint32_t thread,cfgprobe::lifetime::ArrayHeader& registry)const{
  using namespace cfgprobe;const auto& c=owner;
  if(!bound||thread!=c.thread||!c.exactImageVerified||!pawn.HasAssignedSerial()||!state.HasAssignedSerial()||
     !offlineowner::Registry(r,c,registry,offlineowner::Purpose::DiagnosticReadOnly))return false;
  std::uintptr_t level=0;std::uint32_t flags=0;int count=0;std::uint8_t role=0,dead=1;
  float health=0;
  return offlineowner::Same(r,c.module+dispatch::ThreadIdRva,thread)&&
   offlineowner::Same(r,c.module+dispatch::ThreadFlagRva,std::uint8_t{1})&&
   offlineowner::Same(r,c.module+EngineGlobalCandidate,c.engine)&&offlineowner::Same(r,c.engine+EngineGameViewport,c.viewport)&&
   offlineowner::Same(r,c.viewport+ViewportWorld,c.world)&&offlineowner::Same(r,c.world+WorldNetDriver,std::uintptr_t{})&&
   offlineowner::Same(r,c.world+WorldGameMode,c.mode)&&offlineowner::Same(r,c.world+WorldGameInstance,c.gameInstance)&&
   offlineowner::Same(r,c.gameInstance+GameInstanceLocalPlayers,c.localPlayers)&&
   r.Read(c.gameInstance+GameInstanceLocalPlayers+8,count)&&count==1&&offlineowner::Same(r,c.localPlayers,c.player)&&
   offlineowner::Same(r,c.player+PlayerController,c.controller)&&offlineowner::Same(r,c.controller+ControllerPlayer,c.player)&&
   offlineowner::Same(r,c.controller+ControllerPawn,pawn.object)&&offlineowner::Same(r,c.controller+ControllerAcknowledgedPawn,pawn.object)&&
   offlineowner::Same(r,pawn.object+cfgescape::disk::Controller,c.controller)&&
   offlineowner::Same(r,pawn.object+cfgescape::disk::PlayerState,c.playerState)&&offlineowner::Same(r,c.controller+0x388,c.playerState)&&
   offlineowner::Same(r,c.playerState+0x88,c.controller)&&r.Read(c.playerState+0x394,flags)&&!(flags&6)&&
   offlineowner::ExactType(r,c.module,c.mode,offlinelifecycledisk::OfflineBotsClass)&&
   offlineowner::ExactType(r,c.module,c.controller,intrometadatadisk::SCPlayerController_OfflineBotsClass)&&
   offlineowner::ExactType(r,c.module,c.playerState,intrometadatadisk::SCPlayerState_OfflineBotsClass)&&
   offlineowner::Same(r,c.mode,c.module+offlinelifecycledisk::OfflineBotsVtable)&&
   r.Read(c.mode+0x20,level)&&offlineowner::ExactType(r,c.module,level,cfgkill::disk::LevelReflection)&&
   offlineowner::Same(r,level+cfgkill::disk::OwningWorld,c.world)&&
   SameSlot(r,registry,pawn)&&SameSlot(r,registry,state)&&
   offlineowner::Same(r,c.world+WorldGameState,state.object)&&
   offlineowner::Same(r,state.object+cfgprimary::CurrentKiller,pawn.object)&&
   offlineowner::Same(r,state.object+cfgprimary::CurrentKillerState,c.playerState)&&
   r.Read(pawn.object+cfgknife::disk::Role,role)&&role==3&&
   r.Read(pawn.object+cfghunter::disk::Health,health)&&std::isfinite(health)&&health>cfghunter::disk::AliveThreshold&&
   r.Read(c.playerState+cfgescape::disk::bDead,dead)&&!dead&&
   cfgprimary::LookupVetoClearForObservation(r,c.module);
 }
 template<class R>bool Native(R& r,std::uintptr_t cache,std::uintptr_t ctor,std::uintptr_t& type)const{
  std::uintptr_t other=0;return r.Read(owner.module+cache,type)&&r.Read(owner.module+ctor,other)&&cfgprobe::MatchingClassCache(type,other);
 }
 template<class R>bool ActorWorld(R& r,std::uintptr_t actor)const{
  std::uintptr_t level=0;return r.Read(actor+0x20,level)&&
   offlineowner::ExactType(r,owner.module,level,cfgkill::disk::LevelReflection)&&
   offlineowner::Same(r,level+cfgkill::disk::OwningWorld,owner.world);
 }
 template<class R>float Damage(R& r,std::uint32_t thread,unsigned difficulty,
     std::uintptr_t victim,float incoming,std::uintptr_t instigator,std::uintptr_t causer)const{
  if(difficulty>2||difficulty==1||!cfgdifficulty::EligibleAmount(incoming)||!victim||!causer||
     !bound||thread!=owner.thread||victim==pawn.object||(instigator&&instigator!=owner.controller))return incoming;
  cfgprobe::lifetime::ArrayHeader registry;cfgprobe::lifetime::SlotObservation v{},a{};
  std::uintptr_t type=0;cfgescape::State living;cfgnames::Words phase;cfgnames::Text name;
  cfgnames::Observation<R> names(r,owner.module);
  if(!Current(r,thread,registry)||!Native(r,birthdisk::SCKillerCharacterClass,birthdisk::SCKillerCharacterConstructor,type)||
     !cfgspawn::ObjectIs(r,pawn.object,type)||!r.Read(owner.mode+cfgescape::disk::MatchState,phase)||
     !names.Decode(phase,name)||name.View()!=u"InProgress"||
     !cfgspawn::ObjectSlot(r,registry,victim,v)||!v.HasAssignedSerial()||
     !cfgspawn::ObjectSlot(r,registry,causer,a)||!a.HasAssignedSerial()||
     !Native(r,birthdisk::SCCounselorCharacterClass,birthdisk::SCCounselorCharacterConstructor,type)||
     !cfgescape::Counselor(r,registry,owner.module,victim,type,living)||!cfgescape::Driving(living)||
     !ActorWorld(r,victim)||!ActorWorld(r,causer))return incoming;
  auto source=cfgdifficulty::Source::None;
  if(causer==pawn.object)source=cfgdifficulty::Source::Jason;
  else if(Native(r,cfgconvergence::disk::SCWeaponReflection,cfgconvergence::disk::SCWeaponConstructor,type)&&
      cfgkill::OwnedHeldWeapon(r,registry,pawn.object,causer,type))source=cfgdifficulty::Source::Weapon;
  else if(Native(r,cfgknife::disk::ThrowableReflection,cfgknife::disk::ThrowableConstructor,type)&&
      cfgspawn::ObjectIs(r,causer,type)&&offlineowner::Same(r,causer+cfgknife::disk::ThrownBy,pawn.object))source=cfgdifficulty::Source::Projectile;
  cfgprobe::lifetime::ArrayHeader again;
  if(source==cfgdifficulty::Source::None||!SameSlot(r,registry,v)||!SameSlot(r,registry,a)||
     !offlineowner::Same(r,owner.mode+cfgescape::disk::MatchState,phase)||!Current(r,thread,again))return incoming;
  return cfgdifficulty::Scale(incoming,difficulty,source);
 }
};
}
