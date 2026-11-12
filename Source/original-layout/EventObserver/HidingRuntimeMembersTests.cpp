#define CFG_SWIM_MEMBER_FIXTURE_ONLY
namespace cfgh1 { enum class Lane:unsigned {Hiding,Swim}; }
inline bool HostedLaneRefuse(cfgh1::Lane,unsigned){return false;}
inline void HostedLaneEnter(cfgh1::Lane){}
#include "SwimRuntimeMembersTests.cpp"
#include "../Core/HidingInteractionPolicy.hpp"
#include "../Core/DonorHidingTargetPolicy.hpp"
#include "../RuntimeProbe/AiActionBindings.generated.hpp"
#include "../RuntimeProbe/VehicleInteractionBindings.generated.hpp"
#include "../RuntimeProbe/OfflineRecoveryBindings.generated.hpp"
#include "../RuntimeProbe/PositionBindings.generated.hpp"
#include "../Core/VehicleActionPolicy.hpp"
#include "../Core/CounselorPursuitPolicy.hpp"
#include "KillRuntime.hpp"
#include "TrapNativeParameters.hpp"
#include "DonorCounselorObservation.hpp"
inline bool hostedHidingTestActive=false;
inline bool HostedRoutineActive()noexcept{return hostedHidingTestActive;}
#define CFG_H1_JASON 1
struct HidingFixture:SwimFixture {
 cfghiding::State bundleHiding;unsigned bundleHidingRefused=0;
 struct {struct {bool interactionAttempted=false,nativeActiveObserved=false;} policy;} bundleKill;
 struct Bot {std::uintptr_t pawn=0;};
 struct {unsigned bots=0,hidingSelectionChecks=0,hidingSelectionStage=0,hidingPhaseStartStage=0,hidingSafeFailure=0;std::uintptr_t hidingCandidate=0,hidingCandidateSpot=0;std::array<Bot,1> roster{};} bundleReport;
 struct {std::uint64_t selectAt=0;} bundleSchedule;
 bool dying_=false,activeDeath=false,deathReadable=true,deathScopeReadable=true;std::uintptr_t deathActor=99;
 bool hostedContextCurrent=true;
 bool HostedContext(bool){return hostedContextCurrent;}
 bool DonorConvergenceRead(std::uintptr_t,cfgdonorhunt::CounselorObservation&){return false;}
 unsigned actions=0;std::uintptr_t spot_=0,component_=0,manager_=0,scene_=0;
 bool DeathScope(bool& out){out=dying_;return deathScopeReadable;}
 bool DeathContext(cfgkill::Context& out){out={deathActor,0,0,activeDeath};return deathReadable;}
 bool Held(std::uintptr_t& out){out=0;return true;}
 bool Manager(){return true;}
 template<class B,std::size_t N>bool CarCall(const B&,std::uintptr_t executor,const std::array<unsigned char,N>&,std::uintptr_t,void* packet){
  ++calls_;if(executor==cfgtrap::disk::K2_SetActorRotationExecutor)static_cast<cfgtrap::RotationParameters*>(packet)->result=1;
  if(executor==cfgvehicleinteraction::disk::Executor)++actions;return true;
 }
 bool TeleportFunction(std::uintptr_t& out){out=1;return true;}
#include "HidingRuntimeMembers.hpp"
 HidingFixture(){r_.cap=100000;
  const auto type=r_.Object(),interact=r_.Object();scene_=r_.Object();spot_=r_.Object(type);component_=r_.Object(interact);manager_=r_.Object();
  r_.Put(interact+cfgprobe::ClassSuper,scene_);
  r_.Put(plan.module+cfgaction::disk::HidingReflection,type);r_.Put(plan.module+cfgaction::disk::HidingConstructor,type);
  r_.Put(plan.module+cfgvehicleinteraction::disk::ILLInteractableComponent,interact);
  r_.Put(plan.module+cfgvehicleinteraction::disk::ILLInteractableComponentConstructor,interact);
  r_.Put(plan.module+cfgposition::disk::SceneClassCache,scene_);r_.Put(plan.module+cfgposition::disk::SceneConstructorCache,scene_);
  r_.Put(r_.bot+cfgaction::disk::CurrentHidingSpot,spot_);r_.Put(spot_+cfgaction::disk::HidingCounselor,r_.bot);
  r_.Put(bundleJason.pawn.object+cfghunter::disk::Manager,manager_);
  r_.Put(manager_+cfgvehicleinteraction::disk::LockedComponent,std::uintptr_t{});r_.Put(manager_+0x210,component_);
  r_.Put(component_+0xE0,spot_);r_.Put(component_+0x20,spot_);r_.Put(spot_+cfgaction::disk::KillerHidingComponent,component_);
  r_.Put(component_+cfgposition::disk::SceneWorldTranslation,cfgspawn::Vec3{100,200,300});
  std::int32_t index=0;r_.Read(spot_+0xC,index);r_.Put(r_.objects.data+static_cast<std::uintptr_t>(index)*24+16,std::int32_t{123});
  r_.reads=0;r_.cap=1536;calls_=0;
 }
};
int main(){
 CHECK(cfgh241hiding::HidingTarget(true,0xBEEF,1000,1200,true,0xCAFE)==0xBEEF);
 CHECK(cfgh241hiding::HidingTarget(true,0xBEEF,1000,1600,true,0xCAFE)==0xCAFE);
 CHECK(cfgh241hiding::HidingTarget(true,0xBEEF,1000,1200,false,0xCAFE)==0xCAFE);
 CHECK(cfgh241hiding::HidingTarget(false,0xBEEF,1000,1200,true,0xCAFE)==0xCAFE);
 HidingFixture f;CHECK(f.HidingSafe()); // inactive, initialized context allowed
 hostedHidingTestActive=true;f.deathScopeReadable=false;CHECK(f.HidingSafe()&&!f.bundleReport.hidingSafeFailure);
 f.r_.Put(f.bundleJason.pawn.object+cfgescape::disk::DyingFlags,cfgescape::disk::DyingMask);
 CHECK(!f.HidingSafe()&&f.bundleReport.hidingSafeFailure==2);
 f.r_.Put(f.bundleJason.pawn.object+cfgescape::disk::DyingFlags,std::uint8_t{});f.hostedContextCurrent=false;
 CHECK(!f.HidingSafe()&&f.bundleReport.hidingSafeFailure==1);
 f.hostedContextCurrent=true;f.deathScopeReadable=true;hostedHidingTestActive=false;
 f.activeDeath=true;CHECK(!f.HidingSafe());f.activeDeath=false;
 f.deathReadable=false;CHECK(!f.HidingSafe()&&f.bundleReport.hidingSafeFailure==3);f.deathReadable=true;
 f.bundleKill.policy.interactionAttempted=true;CHECK(!f.HidingSafe());f.bundleKill.policy.interactionAttempted=false;
 std::uintptr_t spot=0,component=0,lock=0;cfgspawn::Vec3 point;
 const bool selected=f.HidingSelection(f.r_.bot,spot,component,lock,point);
 if(!selected)std::printf("Hiding fixture diagnostic spot=%llX component=%llX lock=%llX reads=%u\n",spot,component,lock,f.r_.reads);
 CHECK(selected&&spot==f.spot_&&component==f.component_);
 CHECK(f.bundleReport.hidingSelectionStage==9&&f.bundleReport.hidingCandidate==f.r_.bot&&
       f.bundleReport.hidingCandidateSpot==f.spot_&&f.bundleReport.hidingSelectionChecks==1);
 CHECK(f.r_.reads<1536);
 f.bundleHiding.Begin(f.r_.bot,f.spot_,f.component_,100);f.bundleHiding.phase=cfghiding::Phase::Attempt;
 f.r_.reads=0;CHECK(f.HidingPhase(f.r_.bot,100)&&f.actions==1&&f.r_.reads<1536&&f.bundleReport.hidingPhaseStartStage==1);
 auto foreign=HidingFixture{};foreign.r_.Put(foreign.component_+0x20,foreign.r_.bot);
 CHECK(!foreign.HidingSelection(foreign.r_.bot,spot,component,lock,point));
 std::printf("Included hiding native adapter: %s (%d failures); inactive context/zero-serial owned component/final facing tested; native calls mocked.\n",failures?"FAIL":"PASS",failures);
 return failures?1:0;
}
