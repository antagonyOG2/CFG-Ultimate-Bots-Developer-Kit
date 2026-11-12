#define CFG_UNVERIFIED_RUNTIME_FIXTURE
// Composed production spectator adapter. No game process or game allocation.
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include "../RuntimeProbe/SpectatorRuntime.hpp"
#define main SpectatorSparseFixtureMain
#include "../RuntimeProbe/AiResourceTests.cpp"
#undef main
#include "PrimaryContextObservation.hpp"
#include "PrimarySpawnObservation.hpp"
#include "JasonInstanceObservation.hpp"
#include "BundleObservations.hpp"
#include "../RuntimeProbe/SpectatorNativeBindings.generated.hpp"
#include "../RuntimeProbe/SpectatorViewBindings.hpp"
#include "../RuntimeProbe/NativePauseBindings.hpp"
#include "../RuntimeProbe/SweaterHudBindings.generated.hpp"
#include "../RuntimeProbe/StartupTrapBindings.generated.hpp"
#include "../Core/SpectatorStatusObservation.hpp"
#include "../RuntimeProbe/ReflectionObservation.hpp"
#include "../RuntimeProbe/NativeMapSelectionObservation.hpp"
#include "../RuntimeProbe/ArrayReflection.generated.hpp"
#include "../Core/NativeFunctionFrame.hpp"
#include "../Core/SpectatorRefusalPolicy.hpp"
namespace cfgbundle { constexpr unsigned ReadCap=1536,CallCap=12; }
static bool fixturePaused=false;
static Fixture* pauseReader=nullptr;
static std::uintptr_t pauseMutationAt=0;
static bool pauseMutationByte=false;
__declspec(noinline) static bool __fastcall PauseStub(void*) {
 if(pauseReader&&pauseMutationAt){if(pauseMutationByte)pauseReader->Put(pauseMutationAt,std::uint8_t{});else pauseReader->Put(pauseMutationAt,std::uintptr_t{});}
 return fixturePaused;
}
struct SpectatorFixture {
 struct Reader:Fixture { bool ReadBytes(std::uintptr_t at,void* data,std::size_t bytes){auto out=static_cast<unsigned char*>(data);for(std::size_t i=0;i<bytes;++i)if(!Read(at+i,out[i]))return false;return true;} };
 Reader r_;
 cfgobserver::Plan plan{},context{};
 unsigned calls_=0,publications=0,selections=0,blends=0;std::uintptr_t nativeView=0,lastFunction=0;
 unsigned poseMutation=0,rpcMutation=0,teleportMutation=0,teleports=0,lookMutation=0;
 std::uint8_t teleportResult=1;
 cfgspectator::Vector cameraPawnPosition{};
 unsigned pauseMutation=0;
 std::uintptr_t fixtureCamera=0;
 cfgspectator::RefusalReceipt receipt;
 bool exception_=false;
 bool originalActorDispatcher=true;
 struct { std::uintptr_t gameState=0,killerBase=0;std::array<std::uintptr_t,4> classes{},functions{};
  std::array<cfgprobe::lifetime::SlotObservation,4> functionSlots{};
  struct {cfgprobe::lifetime::SlotObservation stateSlot;} primary;
  struct {std::array<std::uintptr_t,8> classes{};} resources; } spawnAssets;
 cfgjason::NewActorEvidence bundleJason;
 cfgprobe::lifetime::SlotObservation bundleHumanTarget;
 std::uintptr_t bundleCounselorClass=0;
 cfgspectator::Runtime bundleSpectator;
 cfgspectator::Identity bundleSpectatorInstalled;
 cfgspawn::PlayerArray bundleSpectatorList{};
 std::array<cfgspectator::Target,16> bundleSpectatorRoster{};
 std::array<std::uintptr_t,16> bundleSpectatorPawnClasses{},bundleSpectatorStateClasses{};
 unsigned bundleSpectatorCursor=0;
 bool bundleSpectatorRosterReady=false;
 std::uint64_t bundleSpectatorCycleEvents=0;
 std::uintptr_t bundleSpectatorStatusFunction=0,bundleSpectatorStatusWidgetHint=0;
 cfgnames::Words bundleSpectatorStatusName{};
 bool bundleSpectatorStatusNameReady=false;
 struct Report {
  unsigned spectatorFailure=0,spectatorLinkFailure=0,spectatorLinkReads=0,spectatorAdmission=0,spectatorAdmissionSerial=0;
  unsigned spectatorResourceBinding=0,spectatorResourceFailure=0,spectatorNativeBytes=0;
  unsigned spectatorStatusJasonView=0,spectatorStatusEvents=0,spectatorLabelOverrides=0,spectatorRosterCount=0;
  std::uintptr_t spectatorStatusWidget=0,spectatorPossessedPawn=0,spectatorStockPawn=0,spectatorCandidatePawn=0;
  std::uintptr_t spectatorAdmissionType=0,spectatorAdmissionOwner=0,spectatorObservedState=0,spectatorNativePawn=0;
  std::array<char16_t,96> spectatorStatusText{};std::array<char16_t,64> spectatorStatusNextMode{};
  std::array<std::uintptr_t,16> spectatorRosterStates{};
 } bundleReport;
 std::uintptr_t fixtureSpectator=0,spectatorType=0,controllerType=0,localState=0,killerState=0,killerController=0;
 std::map<std::uintptr_t,std::uintptr_t> functions;
 // Production Context's terminal-human path; dispatcher existence is supplied
 // by the local Invoke seam. All world/registry/ownership reads are retained.
 bool Context(){return GetCurrentThreadId()==plan.threadId&&cfgprimary::LookupSafe(r_,plan.module)&&
  cfgprimary::RetainedOfflineWorld(r_,plan,context)&&cfgjason::SameSlot(r_,context.objectArray,plan.objects[2])&&
  cfgobserver::Same(r_,plan.world+cfgprobe::WorldGameState,spawnAssets.gameState)&&
  cfgprimary::RegistryMatches(r_,context.objectArray,spawnAssets.primary.stateSlot,bundleJason.pawn.object,plan.controller,spawnAssets.resources.classes[cfgspawn::PlayerState]);}
#include "NativeCallbackObservationMembers.hpp"
 bool Jason(){return cfgjason::LinkedKiller(r_,context.objectArray,bundleJason,plan,plan.pawn,r_.pawnClass,r_.defaults,
  spawnAssets.killerBase,spawnAssets.resources.classes[cfgspawn::KillerAI],spawnAssets.classes[3],plan.controller);}
 bool Alive(std::uintptr_t pawn,bool& alive){
  if(pawn==bundleJason.pawn.object)return cfgbundle::Alive(r_,pawn,alive);
  cfgescape::State state;alive=false;
  if(!cfgescape::Counselor(r_,context.objectArray,plan.module,pawn,bundleCounselorClass,state))return false;
  alive=state==cfgescape::State::Living;return true;
 }
 bool NativeClass(std::uintptr_t reflection,std::uintptr_t constructor,std::uintptr_t& out){
  std::uintptr_t a=0,b=0;cfgprobe::lifetime::SlotObservation slot{};
  out=0;return r_.Read(plan.module+reflection,a)&&r_.Read(plan.module+constructor,b)&&
   (out=cfgprobe::MatchingClassCache(a,b))&&cfgspawn::ObjectSlot(r_,context.objectArray,out,slot);
 }
 bool Location(std::uintptr_t pawn,cfgspawn::Vec3& out){
  cfgprobe::lifetime::SlotObservation fresh{};
  return cfgspawn::ObjectSlot(r_,context.objectArray,spawnAssets.functions[2],fresh)&&fresh==spawnAssets.functionSlots[2]&&
   cfgobserver::Same(r_,plan.module+cfgjason::disk::Functions[2].cache,spawnAssets.functions[2])&&
   cfgobserver::Same(r_,spawnAssets.functions[2]+0x20,spawnAssets.classes[2])&&Invoke(pawn,spawnAssets.functions[2],&out)&&cfgspawn::Finite(out);
 }
 template<class B>bool FeatureFunction(const B& binding,std::uintptr_t& function,std::uintptr_t& owner,std::uint16_t* nativeExtent=nullptr){
  function=owner=0;std::uintptr_t reflection=0,constructor=0;
  if(!r_.Read(plan.module+binding.ownerReflection,reflection)||!r_.Read(plan.module+binding.ownerConstructor,constructor)||
   !(owner=cfgprobe::MatchingClassCache(reflection,constructor))||!cfgbundle::Code(r_,plan.module,binding.factory,binding.code)||
   !r_.Read(plan.module+binding.cache,function))return false;
  cfgprobe::lifetime::SlotObservation ownerSlot{},functionSlot{},fresh{};
  if(!cfgspawn::ObjectSlot(r_,context.objectArray,owner,ownerSlot))return false;
  if(!function){if(calls_>=cfgbundle::CallCap)return false;++calls_;
   __try{function=reinterpret_cast<std::uintptr_t(__fastcall*)()>(plan.module+binding.factory)();}
   __except(EXCEPTION_EXECUTE_HANDLER){exception_=true;return false;}
   if(!function||!Context())return false;
  }
  std::uint16_t parameterBytes=0;
  if(!cfgspawn::ObjectSlot(r_,context.objectArray,function,functionSlot)||
   !r_.Read(function+cfgreflection::arraydisk::FunctionParameterBytes,parameterBytes))return false;
  if(nativeExtent)*nativeExtent=parameterBytes;
  return cfgbundle::NativeFrame(parameterBytes,binding.nativeBytes,binding.storageBytes)&&
   cfgobserver::Same(r_,function+0x20,owner)&&cfgobserver::Same(r_,plan.module+binding.cache,function)&&
   cfgspawn::ObjectSlot(r_,context.objectArray,owner,fresh)&&fresh==ownerSlot&&
   cfgobserver::Same(r_,plan.module+binding.ownerReflection,owner)&&cfgobserver::Same(r_,plan.module+binding.ownerConstructor,owner);
 }
 bool Invoke(std::uintptr_t,std::uintptr_t f,void* p){
  if(calls_>=cfgbundle::CallCap)return false;++calls_;
  if(f==spawnAssets.functions[2])*static_cast<cfgspawn::Vec3*>(p)={100,100,100};
  if(f==functions[cfgspectator::disk::GetControlRotation.cache]){
   *static_cast<cfgspectator::Rotation*>(p)={-10,90,0};
   switch(poseMutation){
    case 1:r_.Put(plan.controller+cfgprobe::ControllerPlayer,std::uintptr_t{});break;
    case 2:Serial(fixtureSpectator,2);break;
    case 3:r_.Put(localState+cfgescape::disk::bDead,std::uint8_t{});break;
    case 4:r_.Put(0x7A0000,std::uintptr_t{});break;
    case 5:r_.Put(r_.bot+cfghunter::disk::Health,0.f);break;
    case 6:r_.Put(plan.controller+cfgspectator::disk::SpectatingPlayer,r_.pawn);break;
   }
  }
  // Model CFG's RPC view reset independently of the publication adapter.
  if(f==functions[cfgspectator::disk::ClientSetSpectatorCamera.cache]){++publications;nativeView=plan.controller;
   if(rpcMutation==1)r_.Put(plan.controller+cfgprobe::ControllerPawn,r_.bot);
   if(rpcMutation==2)r_.Put(plan.world+cfgprobe::WorldNetDriver,plan.controller);
   if(rpcMutation==3)Serial(fixtureSpectator,9);
   if(rpcMutation==4)r_.Put(plan.viewport+cfgprobe::ViewportWorld,std::uintptr_t{});
  }
  if(f==functions[cfgspectator::disk::SetViewTargetWithBlend.cache]){
   ++blends;
   nativeView=*static_cast<std::uintptr_t*>(p);
   if(fixtureCamera)r_.Put(fixtureCamera+cfgspectatorview::disk::Active,nativeView);
   if(nativeView==fixtureSpectator){std::uintptr_t viewing=0;
    if(r_.Read(plan.controller+cfgspectator::disk::SpectatingPlayer,viewing))r_.Put(fixtureSpectator+0x3F8,viewing);}
  }
  if(f==functions[cfgtrap::disk::K2_TeleportTo.cache]){
   struct Pose {cfgspectator::Vector position;cfgspectator::Rotation rotation;std::uint8_t result,pad[3];};
   auto& pose=*static_cast<Pose*>(p);pose.result=teleportResult;
   if(pose.result==1)cameraPawnPosition=pose.position;++teleports;
   if(teleportMutation==1)r_.Put(plan.controller+cfgprobe::ControllerPawn,r_.bot);
   if(teleportMutation==2)r_.Put(plan.world+cfgprobe::WorldNetDriver,plan.controller);
   if(teleportMutation==3)Serial(fixtureSpectator,9);
   if(teleportMutation==4)r_.Put(plan.controller+cfgspectator::disk::SpectatingPlayer,r_.bot);
  }
  if(f==functions[cfgspectator::disk::SetViewTargetWithBlend.cache]){
   if(lookMutation==1)r_.Put(plan.controller+cfgprobe::ControllerPawn,r_.bot);
   if(lookMutation==2)r_.Put(plan.world+cfgprobe::WorldNetDriver,plan.controller);
   if(lookMutation==3)Serial(fixtureSpectator,9);
   if(lookMutation==4)r_.Put(plan.controller+cfgspectator::disk::SpectatingPlayer,r_.bot);
  }
  lastFunction=f;
  return true;
 }
 // Redirect only the OS CAS primitive to the synthetic bytes, retaining the
 // production thread check, expected selection and publication ordering.
 void* InterlockedCompareExchangePointer(void* volatile* address,void* value,void* expected){
  std::uintptr_t old=0;const auto at=reinterpret_cast<std::uintptr_t>(address);
  if(!r_.Read(at,old))return nullptr;
  if(old==reinterpret_cast<std::uintptr_t>(expected)){r_.Put(at,reinterpret_cast<std::uintptr_t>(value));++selections;}
  return reinterpret_cast<void*>(old);
 }
#include "SpectatorRuntimeMembers.hpp"
 void Serial(std::uintptr_t object,int value=1){int index=0;r_.Read(object+12,index);r_.Put(r_.objects.data+24*static_cast<std::uintptr_t>(index)+16,value);}
 void CameraView(std::uintptr_t actor){
  if(!fixtureCamera){r_.cap=100000;const auto cameraType=r_.Object();fixtureCamera=r_.Object(cameraType);
   r_.Put(plan.module+cfgcamera::disk::CameraClassCache,cameraType);r_.Put(plan.module+cfgcamera::disk::CameraConstructorCache,cameraType);
   r_.Put(plan.module+cfgspectatorview::disk::Getter,cfgspectatorview::disk::GetterCode);
   r_.Put(plan.controller+cfgspectatorview::disk::ControllerCamera,fixtureCamera);
   r_.Put(fixtureCamera+cfgspectatorview::disk::PCOwner,plan.controller);r_.cap=1536;
  }
  r_.Put(fixtureCamera+cfgspectatorview::disk::Active,actor);r_.Put(fixtureCamera+cfgspectatorview::disk::Pending,std::uintptr_t{});
 }
 void Binding(const cfgvehicle::disk::Binding& b,std::uintptr_t executor){const auto f=r_.Object();functions[b.cache]=f;r_.Put(plan.module+b.cache,f);r_.Put(f+0xB0,plan.module+executor);
  r_.Put(plan.module+b.ownerReflection,controllerType);r_.Put(plan.module+b.ownerConstructor,controllerType);r_.Put(plan.module+b.factory,b.code);
  r_.Put(f+0x20,controllerType);
  // Independent logical extent from the pinned five native fields: last
  // bool is at20/size1; the UHT/C++ allocation is24, NOT UFunction.ParmsSize.
  const unsigned extent=b.cache==cfgspectator::disk::SetViewTargetWithBlend.cache?21:b.nativeBytes;
  r_.Put(f+cfgreflection::arraydisk::FunctionParameterBytes,static_cast<std::uint16_t>(extent));
 }
 SpectatorFixture(){
  using namespace cfgprobe;
  r_.cap=100000;
  // The actual native pause method calls this local pure function. Its code
  // pin remains in sparse memory and its entire before/after proof executes.
  plan.module=reinterpret_cast<std::uintptr_t>(&PauseStub)-cfgspectator::pausedisk::Query;
  plan.bytes=sizeof(plan);plan.threadId=GetCurrentThreadId();plan.world=r_.Object();plan.mode=r_.Object();
  controllerType=r_.Object();plan.controller=r_.Object(controllerType);plan.pawn=r_.Object(r_.botClass);
  plan.engine=r_.Object();plan.viewport=r_.Object();plan.player=r_.Object();plan.gameInstance=r_.Object();plan.localPlayerData=0x780000;
  plan.objectArray=r_.objects;plan.dispatch.flag=1;plan.dispatch.threadId=plan.threadId;
  plan.dispatch.vtable=0x790000;plan.dispatch.target=plan.module+dispatch::NativeActorDispatcherRva;
  for(auto object:{plan.world,plan.mode,plan.controller,plan.pawn,r_.pawn,r_.bot,r_.botController})Serial(object);
  const std::array<std::uintptr_t,4> objects{plan.world,plan.mode,plan.controller,plan.pawn};
  for(unsigned i=0;i<4;++i)cfgspawn::ObjectSlot(r_,r_.objects,objects[i],plan.objects[i]);
  context=plan;bundleHumanTarget=plan.objects[3];
  r_.Put(plan.module+lifetime::ObjectArrayRva+lifetime::ArrayDataOffset,r_.objects.data);
  r_.Put(plan.module+lifetime::ObjectArrayRva+lifetime::ArrayCountOffset,r_.objects.count);
  r_.Put(plan.module+EngineGlobalCandidate,plan.engine);r_.Put(plan.engine+EngineGameViewport,plan.viewport);
  r_.Put(plan.viewport+ViewportWorld,plan.world);r_.Put(plan.world+WorldNetDriver,std::uintptr_t{});
  r_.Put(plan.world+WorldGameMode,plan.mode);r_.Put(plan.world+WorldGameInstance,plan.gameInstance);
  r_.Put(plan.module+dispatch::ThreadFlagRva,std::uint8_t{1});r_.Put(plan.module+dispatch::ThreadIdRva,plan.threadId);
  r_.Put(plan.module+cfgprimary::SerializationFlag,std::uint8_t{});r_.Put(plan.module+cfgprimary::GarbageCollectionCounter,std::uint32_t{});
  r_.Put(plan.gameInstance+GameInstanceLocalPlayers,cfgobserver::LocalPlayers{plan.localPlayerData,1,1});
  r_.Put(plan.localPlayerData,plan.player);r_.Put(plan.player+PlayerController,plan.controller);r_.Put(plan.controller+ControllerPlayer,plan.player);
  spawnAssets.gameState=r_.state;spawnAssets.resources.classes=r_.classes;spawnAssets.killerBase=r_.killerBase;bundleCounselorClass=r_.counselorBase;
  r_.Put(plan.world+WorldGameState,r_.state);
  spectatorType=r_.Object();fixtureSpectator=r_.Object(spectatorType);Serial(fixtureSpectator);
  r_.Put(plan.module+cfgspectator::disk::PawnClassCache,spectatorType);r_.Put(plan.module+cfgspectator::disk::PawnConstructorCache,spectatorType);
  r_.Put(fixtureSpectator+cfgescape::disk::Controller,plan.controller);r_.Put(plan.controller+ControllerPawn,fixtureSpectator);r_.Put(plan.controller+ControllerSpectatorPawn,fixtureSpectator);
  localState=r_.Object(r_.classes[cfgspawn::PlayerState]);r_.Put(localState+cfgescape::disk::bDead,std::uint8_t{1});r_.Put(plan.controller+cfgescape::disk::PlayerState,localState);
  r_.Put(plan.module+cfgescape::disk::SCPlayerStateReflection,r_.classes[cfgspawn::PlayerState]);r_.Put(plan.module+cfgescape::disk::SCPlayerStateConstructor,r_.classes[cfgspawn::PlayerState]);
  r_.Put(plan.controller+cfgspectator::disk::SpectatingPlayer,r_.bot);
  r_.Put(r_.bot+cfghunter::disk::Health,100.f);
  killerController=r_.Object(r_.classes[cfgspawn::KillerAI]);Serial(killerController);killerState=r_.Object(r_.classes[cfgspawn::PlayerState]);
  r_.Put(r_.pawn+cfgescape::disk::Controller,killerController);r_.Put(r_.pawn+cfgescape::disk::PlayerState,killerState);
  r_.Put(killerController+ControllerPawn,r_.pawn);r_.Put(killerController+cfgescape::disk::PlayerState,killerState);
  r_.Put(r_.state+cfgprimary::CurrentKillerState,killerState);cfgspawn::ObjectSlot(r_,r_.objects,r_.pawn,bundleJason.pawn);
  r_.Put(r_.state+cfgprimary::CurrentKiller,r_.pawn);cfgspawn::ObjectSlot(r_,r_.objects,r_.state,spawnAssets.primary.stateSlot);
  cfgspawn::ObjectSlot(r_,r_.objects,killerController,bundleJason.killerController);
  spawnAssets.classes[3]=r_.Object();const auto path=r_.Object(spawnAssets.classes[3]);cfgspawn::ObjectSlot(r_,r_.objects,path,bundleJason.path);
  r_.Put(r_.pawn+cfgspawn::PawnAIClass,r_.classes[cfgspawn::KillerAI]);r_.Put(killerController+0x410,path);
  r_.Put(path+0x20,killerController);r_.Put(path+cfgjason::disk::ComponentOwner,killerController);
  spawnAssets.classes[2]=r_.Object();spawnAssets.functions[2]=r_.Object();
  cfgspawn::ObjectSlot(r_,r_.objects,spawnAssets.functions[2],spawnAssets.functionSlots[2]);
  r_.Put(plan.module+cfgjason::disk::Functions[2].cache,spawnAssets.functions[2]);r_.Put(spawnAssets.functions[2]+0x20,spawnAssets.classes[2]);
  r_.Put(r_.state+cfgescape::disk::PlayerCharacterList,cfgspawn::PlayerArray{0x7A0000,1,1});r_.Put(0x7A0000,r_.bot);
  r_.Put(r_.pawn+0x1800,cfgspawn::Vec3{100,100,100});
  r_.Put(plan.mode+cfgescape::disk::MatchState,cfgnames::Words{0,0});
  r_.Put(plan.module+cfgnames::disk::Global,std::uintptr_t{0x800000});r_.Put(0x800000+cfgnames::disk::ElementCount,std::array<int,2>{1,1});
  r_.Put(0x800000,std::uintptr_t{0x810000});r_.Put(0x810000,std::uintptr_t{0x820000});r_.Put(0x820000,std::uint32_t{});
  r_.Put(0x820000+cfgnames::disk::EntryText,std::array<char,16>{'I','n','P','r','o','g','r','e','s','s',0});
  r_.Put(plan.module+cfgspectator::pausedisk::Engine,plan.engine);r_.Put(plan.module+cfgspectator::pausedisk::Query,cfgspectator::pausedisk::QueryCode);
  Binding(cfgspectator::disk::GetControlRotation,cfgspectator::disk::GetControlRotationExecutor);
  Binding(cfgspectator::disk::ClientSetSpectatorCamera,cfgspectator::disk::ClientSetSpectatorCameraExecutor);
  Binding(cfgtrap::disk::K2_TeleportTo,cfgtrap::disk::K2_TeleportToExecutor);
  const auto actorType=r_.Object();r_.Put(controllerType+0x30,actorType);r_.Put(spectatorType+0x30,actorType);
  r_.Put(plan.module+cfgtrap::disk::K2_TeleportTo.ownerReflection,actorType);
  r_.Put(plan.module+cfgtrap::disk::K2_TeleportTo.ownerConstructor,actorType);
  r_.Put(functions[cfgtrap::disk::K2_TeleportTo.cache]+0x20,actorType);
  r_.Put(plan.module+cfgtrap::disk::K2_TeleportToExecutor,cfgtrap::disk::K2_TeleportToCode);
  Binding(cfgspectator::disk::SetViewTargetWithBlend,cfgspectator::disk::SetViewTargetWithBlendExecutor);
  Binding(cfgtrap::disk::SetControlRotation,cfgtrap::disk::SetControlRotationExecutor);
  r_.Put(plan.module+cfgspectator::disk::GetControlRotationExecutor,cfgspectator::disk::GetControlRotationExecutorCode);
  r_.Put(plan.module+cfgspectator::disk::ClientSetSpectatorCameraExecutor,cfgspectator::disk::ClientSetSpectatorCameraExecutorCode);
  r_.Put(plan.module+cfgspectator::disk::SetViewTargetWithBlendExecutor,cfgspectator::disk::SetViewTargetWithBlendExecutorCode);
  r_.Put(plan.module+cfgtrap::disk::SetControlRotationExecutor,cfgtrap::disk::SetControlRotationCode);
  r_.reads=0;r_.cap=cfgbundle::ReadCap;
 }
 void Step(std::uint64_t now,bool key){
  r_.reads=0;calls_=0;spectatorSafetyProved_=false;spectatorTerminalProved_=false;
  pauseReader=&r_;pauseMutationByte=pauseMutation==1;
  pauseMutationAt=pauseMutation==1?localState+cfgescape::disk::bDead:
   pauseMutation==2?plan.controller+cfgprobe::ControllerPlayer:0;
  bundleSpectator.Step(*this,now,key);pauseReader=nullptr;pauseMutationAt=0;
  receipt.Capture(now,bundleSpectator.report.refusals,bundleReport.spectatorFailure,bundleSpectator.report.decision,
   bundleReport.spectatorLinkFailure,r_.reads,calls_,bundleReport.spectatorRosterCount,
   bundleReport.spectatorResourceBinding,bundleReport.spectatorResourceFailure,bundleReport.spectatorNativeBytes);
 }
 void Roster(unsigned count,bool ancestry=false){
  r_.cap=100000;r_.reads=0;
  if(ancestry)for(auto root:{r_.classes[cfgspawn::PlayerState],r_.counselorBase,r_.killerBase,controllerType}){
   std::uintptr_t originalParent=0;r_.Read(root+0x30,originalParent);
   auto current=root;for(unsigned depth=0;depth<6;++depth){const auto parent=r_.Object();r_.Put(current+0x30,parent);current=parent;}
   r_.Put(current+0x30,originalParent);
  }
  for(unsigned i=1;i<count;++i){const auto pawn=r_.Object(r_.botClass),controller=r_.Object(r_.classes[cfgspawn::CounselorAI]),state=r_.Object(r_.classes[cfgspawn::PlayerState]);
   Serial(pawn);Serial(controller);r_.Put(pawn+cfgescape::disk::Controller,controller);r_.Put(pawn+cfgescape::disk::PlayerState,state);
   r_.Put(controller+cfgprobe::ControllerPawn,pawn);r_.Put(controller+cfgescape::disk::PlayerState,state);r_.Put(pawn+cfghunter::disk::Health,100.f);
   r_.Put(0x7A0000+8*i,pawn);
  }
  r_.Put(r_.state+cfgescape::disk::PlayerCharacterList,cfgspawn::PlayerArray{0x7A0000,static_cast<int>(count),static_cast<int>(count)});r_.cap=1536;r_.reads=0;
 }
};
#ifndef CFG_SPECTATOR_MEMBER_FIXTURE_ONLY
int main(){
 // Independent migration regression: the 3.4 address and old helper-call
 // displacement must never satisfy the production getter's full code pin.
 static_assert(cfgspectatorview::disk::Getter==0x168E6B0);
 static_assert(cfgspectator::disk::StatusFunctionCache==0x304D138);
 for(unsigned stale:{0u,1u}){SpectatorFixture drift;drift.CameraView(drift.r_.bot);
  if(!stale){drift.r_.Put(drift.plan.module+0x168D020,cfgspectatorview::disk::GetterCode);
   drift.r_.Put(drift.plan.module+cfgspectatorview::disk::Getter,std::uint8_t{});}
  else drift.r_.Put(drift.plan.module+cfgspectatorview::disk::Getter+49,std::uint8_t{0x1D});
  cfgspectator::Snapshot snapshot{};CHECK(drift.Observe(snapshot));
  CHECK(!drift.SpectatorViewObservation(snapshot));
  CHECK(!drift.selections&&!drift.blends);
 }
 for(unsigned extent:{0u,20u,22u,24u}){SpectatorFixture wrong;
  wrong.r_.Put(wrong.functions[cfgspectator::disk::SetViewTargetWithBlend.cache]+cfgreflection::arraydisk::FunctionParameterBytes,static_cast<std::uint16_t>(extent));
  wrong.Step(100,true);CHECK(!wrong.publications&&!wrong.selections&&!wrong.bundleSpectator.Following());
  CHECK(wrong.bundleReport.spectatorResourceBinding==3&&wrong.bundleReport.spectatorResourceFailure==2&&wrong.bundleReport.spectatorNativeBytes==extent);
 }
 SpectatorFixture f;f.CameraView(f.r_.bot);f.Step(100,true);
 std::printf("native follow: following=%d RPCs=%u teleports=%u selections=%u reads=%u calls=%u failure=%u\n",f.bundleSpectator.Following(),f.publications,f.teleports,f.selections,f.r_.reads,f.calls_,f.bundleReport.spectatorFailure);
 CHECK(f.bundleSpectator.Following()&&f.selections==1&&f.blends==1&&!f.publications&&!f.teleports);
 CHECK(f.nativeView==f.fixtureSpectator&&f.r_.reads<=1536&&f.calls_<=12);
 for(unsigned i=1;i<=120;++i){f.Step(100+20*i,false);
  CHECK(f.bundleSpectator.Following()&&!f.publications&&!f.teleports&&f.blends==1&&f.selections==1);
  CHECK(f.r_.reads<=1536&&f.calls_<=12);}
 {SpectatorFixture native;native.CameraView(native.r_.bot);native.Step(100,true);
  native.CameraView(native.plan.world);native.Step(150,false);
  CHECK(!native.bundleSpectator.Following()&&native.blends==1&&!native.teleports&&!native.publications);}
 {SpectatorFixture cycle;cycle.Roster(2);cycle.CameraView(cycle.r_.bot);
  cycle.r_.Put(cycle.plan.controller+cfgspectator::disk::SpectatingPlayer,std::uintptr_t{});
  cycle.Step(100,false);CHECK(!cycle.selections);
  std::uintptr_t second=0;CHECK(cycle.r_.Read(0x7A0008,second));cycle.CameraView(second);cycle.Step(150,false);CHECK(!cycle.selections);
  cycle.CameraView(cycle.r_.bot);cycle.Step(200,false);CHECK(cycle.bundleSpectator.Following()&&cycle.selections==1);
  cycle.CameraView(second);cycle.Step(250,false);CHECK(!cycle.bundleSpectator.Following()&&!cycle.teleports);}
 {SpectatorFixture unpossessed;unpossessed.CameraView(unpossessed.r_.bot);
  unpossessed.r_.Put(unpossessed.plan.controller+cfgprobe::ControllerPawn,std::uintptr_t{});
  unpossessed.Step(100,true);CHECK(unpossessed.bundleSpectator.Following()&&unpossessed.selections==1);
  CHECK(!unpossessed.teleports&&!unpossessed.publications);}
 {SpectatorFixture cycle;cycle.CameraView(cycle.r_.bot);cycle.Step(100,false);
  ++cycle.bundleSpectatorCycleEvents;cycle.Step(150,false);
  CHECK(cycle.bundleSpectator.Following()&&cycle.selections==1);cycle.Step(180,true);
  CHECK(!cycle.bundleSpectator.Following()&&cycle.blends==2&&!cycle.teleports&&!cycle.publications);
  std::uintptr_t selected=0;CHECK(cycle.r_.Read(cycle.plan.controller+cfgspectator::disk::SpectatingPlayer,selected)&&selected==cycle.r_.bot);}
 for(unsigned count:{2u,7u,16u}){SpectatorFixture many;many.Roster(count,true);many.CameraView(many.r_.bot);
  for(unsigned i=0;i<(count+1)/2;++i)many.Step(100+20*i,false);
  many.Step(500,true);
  std::printf("roster=%u following=%d reads=%u calls=%u failure=%u\n",count,many.bundleSpectator.Following(),many.r_.reads,many.calls_,many.bundleReport.spectatorFailure);
  CHECK(many.r_.reads<=1536&&many.calls_<=12&&!many.teleports&&!many.publications);
  if(count<=7)CHECK(many.bundleSpectator.Following()&&many.selections==1);
 }
 for(unsigned fault=0;fault<8;++fault){SpectatorFixture bad;bad.CameraView(bad.r_.bot);
  switch(fault){
   case 0:bad.r_.Put(bad.plan.controller+cfgprobe::ControllerPlayer,std::uintptr_t{});break;
   case 1:bad.Serial(bad.fixtureSpectator,0);break;
   case 2:bad.r_.Put(bad.localState+cfgescape::disk::bDead,std::uint8_t{});break;
   case 3:bad.r_.Put(bad.r_.bot+cfghunter::disk::Health,0.f);break;
   case 4:bad.r_.Put(bad.r_.botController+cfgescape::disk::PlayerState,bad.killerState);break;
   case 5:bad.r_.Put(bad.plan.world+cfgprobe::WorldNetDriver,bad.plan.controller);break;
   case 6:bad.r_.trigger=bad.plan.controller+cfgprobe::ControllerPlayer;bad.r_.triggerAt=2;bad.r_.mutation=bad.r_.trigger;bad.r_.newPointer=0;break;
   case 7:bad.r_.trigger=0x7A0000;bad.r_.triggerAt=2;bad.r_.mutation=bad.r_.trigger;bad.r_.newPointer=bad.r_.pawn;break;
  }
  bad.Step(100,true);CHECK(!bad.publications&&!bad.teleports&&!bad.selections&&!bad.bundleSpectator.Following());
 }
 fixturePaused=true;SpectatorFixture paused;paused.Step(100,true);CHECK(!paused.selections);fixturePaused=false;
 for(unsigned mutation=1;mutation<=6;++mutation){SpectatorFixture changed;changed.CameraView(changed.r_.bot);
  changed.poseMutation=mutation;changed.Step(100,true);CHECK(!changed.selections&&!changed.bundleSpectator.Following());}
 for(unsigned mutation=1;mutation<=2;++mutation){SpectatorFixture changed;changed.CameraView(changed.r_.bot);
  changed.pauseMutation=mutation;changed.Step(100,true);CHECK(!changed.selections&&!changed.bundleSpectator.Following());}
 for(unsigned mutation=1;mutation<=4;++mutation){SpectatorFixture changed;changed.CameraView(changed.r_.bot);
  changed.lookMutation=mutation;changed.Step(100,true);
  CHECK(!changed.bundleSpectator.Following()&&changed.blends==1&&!changed.teleports&&!changed.publications);}
 for(unsigned fault=0;fault<6;++fault){SpectatorFixture bad;bad.CameraView(bad.r_.bot);
  switch(fault){
   case 0:bad.r_.Put(bad.bundleJason.path.object+cfgjason::disk::ComponentOwner,bad.plan.controller);break;
   case 1:bad.r_.Put(bad.killerController+0x410,std::uintptr_t{});break;
   case 2:bad.r_.Put(bad.plan.module+cfgspectator::disk::GetControlRotation.ownerConstructor,bad.r_.pawnClass);break;
   case 3:bad.r_.Put(bad.functions[cfgspectator::disk::SetViewTargetWithBlend.cache]+cfgreflection::arraydisk::FunctionParameterBytes,std::uint16_t{});break;
   case 4:bad.Serial(bad.spawnAssets.functions[2],9);break;
   case 5:bad.r_.Put(bad.plan.module+cfgspectator::disk::SetViewTargetWithBlend.factory,std::uint8_t{});break;
  }
  bad.Step(100,true);CHECK(!bad.selections&&!bad.bundleSpectator.Following());}
 {SpectatorFixture respawn;respawn.CameraView(respawn.r_.bot);respawn.Step(100,true);CHECK(respawn.bundleSpectator.Following());
  respawn.r_.Put(respawn.plan.controller+cfgprobe::ControllerPawn,respawn.r_.bot);
  respawn.Step(150,false);CHECK(!respawn.bundleSpectator.Following()&&respawn.blends==1&&!respawn.teleports);}
 {SpectatorFixture lag;lag.Roster(7,true);lag.CameraView(lag.r_.bot);
  lag.r_.Put(lag.localState+cfgescape::disk::bDead,std::uint8_t{});
  lag.r_.Put(lag.plan.pawn+cfgescape::disk::PlayerState,lag.localState);lag.r_.Put(lag.plan.pawn+cfghunter::disk::Health,0.f);
  for(unsigned i=0;i<4;++i)lag.Step(100+20*i,false);lag.Step(500,true);
  std::printf("lagged terminal native follow reads=%u calls=%u failure=%u link=%u decision=%u\n",lag.r_.reads,lag.calls_,lag.bundleReport.spectatorFailure,lag.bundleReport.spectatorLinkFailure,lag.bundleSpectator.report.decision);
  CHECK(lag.bundleSpectator.Following()&&lag.selections==1&&!lag.teleports&&!lag.publications);
  CHECK(lag.r_.reads<=1536&&lag.calls_<=12);}
 std::printf("native stock follow composed adapter: %d failures\n",failures);return failures?1:0;
}
#endif
