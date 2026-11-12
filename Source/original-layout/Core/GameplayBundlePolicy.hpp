#pragma once
#include <array>
#include "H248SweaterTarget.hpp"
#ifndef CFG_H250_ARRAY_GROWTH
#define CFG_H250_ARRAY_GROWTH 1 // hosted BundleAccess refreshes the GUObjectArray count (growth only)
#endif
#include <cstdint>
#include <cmath>
#include "HelperSpawnPolicy.hpp"
#include "SpectatorRefusalPolicy.hpp"
#include "J21WorkPolicy.hpp"
#include "J21cFeatureFlags.hpp"
#include "J23GrabTiming.hpp"
#include "ConvergenceArrivalReport.hpp"
#include "J21FrameBudget.hpp"
#ifndef CFG_H245_EMOTE
#define CFG_H245_EMOTE 1 // hosted emote hook proofs use the shared 4096-read observation budget (offline stays 1536)
#endif
#ifndef CFG_H243_ROSTER
#define CFG_H243_ROSTER 1 // report-only Tommy-time roster counters (no behaviour change)
#endif
#include "LeanConvergencePolicy.hpp"
#include "CounselorPracticePolicy.hpp"
#include "HostedObservationBudget.hpp"
#include "DonorJasonBusyCause.hpp"
namespace cfgbundle {
constexpr unsigned MaximumBots=cfglimits::MaximumHelpers,InitialBots=3,ReadCap=1536,CallCap=12;
#ifdef CFG_H1_JASON
constexpr unsigned ReportReadCap=cfghostedbudget::EmoteReads;
#else
constexpr unsigned ReportReadCap=ReadCap;
#endif
// One positively identified offline session remains bounded, but normal play
// must not silently lose AI at the old five-minute/1000-phase prototype limit.
constexpr std::uint64_t MaximumElapsedMs=5400000,MaximumPhases=60000;
enum class Lane : unsigned { None, Release, Spawn, Select, Hunt, Vehicle, Convergence, Settings, Loadout };
enum class HostedPresentationDispatch : unsigned { None, Primary, Extra };
constexpr HostedPresentationDispatch PresentationDispatch(bool adopted,bool nested,bool primaryAttempted,
    bool extraPending,bool extraAttempted)noexcept{
    if(!adopted||nested)return HostedPresentationDispatch::None;
    if(!primaryAttempted)return HostedPresentationDispatch::Primary;
    if(extraPending&&!extraAttempted)return HostedPresentationDispatch::Extra;
    return HostedPresentationDispatch::None;
}
constexpr bool AdditionalJasonLaneEnabled(bool offlineScheduleActive,bool hostedAdoption)noexcept{
    return offlineScheduleActive||hostedAdoption;
}
// Bounded subreasons in the existing knifeFailure capture field. Values 0-7
// remain the original aggregate failures; this range names KnifeOwner clauses.
enum class KnifeOwnerFailure : unsigned {
    Context=100, Jason=101, PlayerState=102, PlayerStateSlot=103, PlayerStateClass=104,
    PawnClass=105, ControllerClass=106, PathClass=107, ThreadOrOwnerState=108,
    LookupSafe=109, GameplayWorld=110, GameState=111, FreshOwnerLinks=112,
    AliveProof=113, NotLiving=114, AuthorityRole=115, VTableRead=116,
    ReleaseBinding=117, RefreshBinding=118, FinalizeBinding=119
};
constexpr bool ValidKnifeFailure(unsigned value) noexcept {
    return value<=7||(value>=static_cast<unsigned>(KnifeOwnerFailure::Context)&&
        value<=static_cast<unsigned>(KnifeOwnerFailure::FinalizeBinding));
}
// The host's terminal signal may drive label updates, but native hosted
// spectating owns the camera. Never activate the offline camera driver online.
constexpr bool OfflineSpectatorCameraAllowed(bool hosted,unsigned terminalMask)noexcept{
    return !hosted&&(terminalMask&1u)!=0;
}
// Cheap dispatch routing only, never native-call or lifetime authorization.
inline bool OwnedCallback(std::uintptr_t actor,std::uintptr_t human,std::uintptr_t jason,bool started,
    std::uintptr_t localController=0,bool humanTerminal=false){
    // The original counselor can stop dispatching after native escape/death.
    // Use only its saved LOCAL controller as a terminal-session transport;
    // Context() must still prove its positive slot before any native action.
    return actor&&(actor==human||(started&&jason&&actor==jason)||
        (started&&humanTerminal&&localController&&actor==localController));
}
class TargetChoice {
    // "Better killer" scoring over the sweep's existing observations (no new
    // reads): an isolated counselor (nobody else within 12 m) is a quicker
    // kill and scores as if 20% closer; a chase that has been kited without
    // closing for a long time stops holding the six-second lock and scores
    // 60% farther, so Jason drops a looped target for a reachable one.
    struct Entry {bool valid=false,positioned=false,retained=false;float squared=0,x=0,y=0;};
    std::array<Entry,MaximumBots+2> entries_{};
    static constexpr float Isolation=1200.f,IsolatedFactor=.64f /* .8 squared */,FatiguedFactor=2.56f /* 1.6 squared */;
    bool Isolated(unsigned i)const{
        if(!entries_[i].positioned)return false;
        for(unsigned j=0;j<entries_.size();++j){if(j==i||!entries_[j].valid)continue;
            if(!entries_[j].positioned)return false; // unknown neighbour: no bonus
            const float dx=entries_[i].x-entries_[j].x,dy=entries_[i].y-entries_[j].y;
            if(dx*dx+dy*dy<Isolation*Isolation)return false;}
        return true;
    }
    float Score(unsigned i,unsigned fatigued)const{
        const float s=entries_[i].squared*(Isolated(i)?IsolatedFactor:1.f);
        return i==fatigued?s*FatiguedFactor:s;
    }
public:
    void Begin(){entries_={};}
    void Consider(unsigned index,float squared,bool retained=false){Consider(index,squared,retained,nullptr);}
    // position: horizontal x/y of the candidate from the same Distance() read.
    void Consider(unsigned index,float squared,bool retained,const float* position){
        // The last logical selection slot is reserved for the positively
        // observed native counselor (for example Tommy), beyond F3 helpers.
        if(index>MaximumBots+1||!std::isfinite(squared)||squared<0)return;
        auto& e=entries_[index];e={};e.valid=true;e.retained=retained;e.squared=squared;
        if(position&&std::isfinite(position[0])&&std::isfinite(position[1])){e.positioned=true;e.x=position[0];e.y=position[1];}
    }
    bool Found()const{for(const auto& e:entries_)if(e.valid)return true;return false;}
    // Donor SelectStockPursuitTarget: retain a freshly observed living target
    // for six seconds unless another scores substantially better (squared * .45).
    // fatigued: logical slot of a counselor who kited Jason (penalized, and
    // never held by the lock); ~0u = none.
    unsigned Index(bool locked=false,unsigned fatigued=~0u)const{
        unsigned best=0,retained=0;bool found=false,held=false;float bestScore=0,retainedScore=0;
        for(unsigned i=0;i<entries_.size();++i){if(!entries_[i].valid)continue;const float s=Score(i,fatigued);
            if(!found||s<bestScore){found=true;best=i;bestScore=s;}
            if(entries_[i].retained){held=true;retained=i;retainedScore=s;}}
        return locked&&held&&retained!=fatigued&&bestScore>=retainedScore*.45f?retained:best;
    }
};
struct Schedule {
    bool active=false,pressed=false,helpersActive=true,settingsActive=false,loadoutActive=false,backgroundLoadout=false;
    unsigned spawned=0;
    unsigned requestedBots=InitialBots;
    std::uint64_t next=0,releaseAt=0,spawnAt=0,selectAt=0,huntAt=0,vehicleAt=0,convergenceAt=0,attackAt=0,expires=0,settingsAt=0,loadoutAt=0,meleeUntil=0,meleePriorityAt=0;
    bool Start(std::uint64_t now,unsigned bots=InitialBots,unsigned seconds=0) {
        if(bots>MaximumBots||pressed||active||now>UINT64_MAX-MaximumElapsedMs||
            (seconds&&seconds!=1800&&seconds!=3600&&seconds!=5400))return false;
        *this={};requestedBots=bots;active=true;expires=now+(seconds?static_cast<std::uint64_t>(seconds)*1000:MaximumElapsedMs);next=now+100;
        spawnAt=now+1500;selectAt=now+300;huntAt=now+700;vehicleAt=now+1100;convergenceAt=now+500;return true;
    }
    Lane Due(std::uint64_t now) {
        if((!active&&!pressed) || now<next)return Lane::None;
        const bool close=now<meleeUntil;
        next=now+(close?25:100);
        // Releases are obligations, never suppressed by stun/cooldown/new-target work.
        if(pressed && now>=releaseAt)return Lane::Release;
        if(pressed)return Lane::None;
        if(!active)return Lane::None;
        if(settingsActive&&now>=settingsAt){settingsAt=now+500;return Lane::Settings;}
        if(!settingsActive&&helpersActive&&spawned<requestedBots && now>=spawnAt){spawnAt=now+1500;return Lane::Spawn;}
        if(loadoutActive&&!backgroundLoadout&&now>=loadoutAt){loadoutAt=now+1000;return Lane::Loadout;}
        // A ready close-range swing should not wait behind every overdue
        // maintenance lane. Bound this priority to once/200ms so vehicle and
        // selection work still get turns; native readiness remains mandatory.
        if(close&&now>=huntAt&&now>=meleePriorityAt){
            meleePriorityAt=now+200;huntAt=now+25;return Lane::Hunt;
        }
        if(now>=vehicleAt){vehicleAt=now+1100;return Lane::Vehicle;}
        if(now>=selectAt){selectAt=now+1900;return Lane::Select;}
        if(now>=huntAt){huntAt=now+(close?25:700);return Lane::Hunt;}
        if(now>=convergenceAt){convergenceAt=now+250;return Lane::Convergence;}
        if(loadoutActive&&backgroundLoadout&&now>=loadoutAt){loadoutAt=now+100;return Lane::Loadout;}
        return Lane::None;
    }
    void ContinueSelection(std::uint64_t now,bool complete){selectAt=now+(complete?1900:200);}
    bool AdditionalHelper(std::uint64_t now){
        if(!active||spawned!=requestedBots||requestedBots>=MaximumBots)return false;
        ++requestedBots;helpersActive=true;spawnAt=now+100;return true;
    }
    bool AdoptNativeHelpers(unsigned born){
        if(!active||pressed||spawned||born>requestedBots)return false;
        spawned=born;helpersActive=spawned<requestedBots;return true;
    }
    // Close-range hints change polling ONLY, never native attack admission.
    // CanAttack/Attacking/target/range/stun locks still gate every native press.
    void CloseCombat(std::uint64_t now,float squared){
        if(std::isfinite(squared)&&squared>=0&&squared<=260.f*260.f){meleeUntil=now+1000;huntAt=now+25;}
        else meleeUntil=0;
    }
    void Pressed(std::uint64_t now) { pressed=true;releaseAt=now+150;attackAt=now; }
    void Released() { pressed=false;releaseAt=0; }
    bool AttackDue(std::uint64_t now,float squared) const {
        return active&&!pressed&&now>=attackAt&&std::isfinite(squared)&&squared>=0&&squared<=260.f*260.f;
    }
};
struct CameraTurn {bool camera=false;Lane gameplay=Lane::None;};
inline CameraTurn SelectCameraTurn(Schedule& schedule,std::uint64_t now,bool cameraDue,bool previousCamera){
    cameraDue=cameraDue&&schedule.active&&!schedule.pressed;
    CameraTurn turn;turn.camera=cameraDue&&!previousCamera;
    if(!turn.camera)turn.gameplay=schedule.Due(now);
    if(turn.gameplay==Lane::None&&cameraDue&&!schedule.pressed)turn.camera=true;
    return turn;
}
inline CameraTurn SelectActionTurn(Schedule& schedule,std::uint64_t now,bool cameraDue,bool previousCamera,bool knifeDue,bool trapDue=false){
    // An already-owned native throw must not wait for an unrelated AI lane.
    // Owed melee release still wins; no maintenance due timestamps consumed.
    if(knifeDue&&schedule.active&&!schedule.pressed)return {false,Lane::Hunt};
    auto turn=SelectCameraTurn(schedule,now,cameraDue,previousCamera);
    // Due native opening-trap work gets unused owner-thread callbacks. A
    //700ms hunt/door probe must not repeatedly consume the only travel slice.
    // Ordinary vehicle/settings/helper lanes and owed release still win.
    if(trapDue&&!turn.camera&&turn.gameplay==Lane::None&&schedule.active&&!schedule.pressed)turn.gameplay=Lane::Hunt;
    return turn;
}
constexpr std::uint32_t ReportMagic=0x43464742,ReportVersion=87;
enum class State : unsigned { Dormant, Active, Stopped };
enum class Stop : unsigned { None, Context, JasonIdentity, Resources, NativeException, Spawn, Input, NoTargets, Limit };
enum class InitializationStep : unsigned {
    None,Configuration,Context,SpawnState,Pawn,PawnSerial,Controller,ControllerSerial,Path,Jason,
    ManagerClass,ManagerPair,ManagerIdentity,CounselorClass,TreeIdentity,Tree,NativeBindings,Manager,
    NativeCohort,HelperClass,HelperSpawn,HelperPlayerState,HelperPawnIdentity,HelperControllerIdentity,
    HelperDuplicate,HelperAdoption,Schedule,Duration,NativeSchedule,Ready
};
struct KnifeBoundarySample {
    std::uintptr_t returnPc=0,subject=0;
    std::uint32_t kind=0,flags=0,knifeStage=0,count=0;
};
struct Bot {
    std::uintptr_t pawn=0,controller=0;
    std::int32_t pawnSerial=0,controllerSerial=0;
};
template<class Observe> bool TrackedDriver(std::uintptr_t driver,unsigned bots,
    const std::array<Bot,MaximumBots>& roster,Observe observe){
    if(!driver||bots>MaximumBots)return false;
    for(unsigned i=0;i<=bots;++i){
        // Human observation must follow native possession (including Tommy),
        // not a cached starting-pawn value. Helpers are directly prefiltered.
        if(i&&roster[i-1].pawn!=driver)continue;
        std::uintptr_t candidate=0;
        if(observe(i,candidate)&&candidate==driver)return true;
    }
    return false;
}
struct Report {
    std::uint32_t magic=ReportMagic,version=ReportVersion,bytes=sizeof(Report),generation=0;
    std::uint32_t state=0,stop=0,bots=0,spawnAttempts=0,spawned=0,phase=0,maxReads=0,maxCalls=0;
    std::uint32_t pressReturns=0,releases=0,moves=0,heldSkips=0,stunSkips=0,notReady=0,attackAnimations=0;
    std::uint64_t phases=0,reads=0,calls=0,maxMicroseconds=0;
    std::uintptr_t jason=0,target=0;
    std::uint32_t error=0,releasePending=0,releaseAbandoned=0;
    std::uint32_t carChecks=0,carMoves=0,morphAttempts=0,morphReturns=0,carUnavailable=0;
    std::uint32_t carHoodAttempts=0,carSeatAttempts=0,carActionRefusals=0,carSpeedWrites=0,carSpeedRestores=0,carSpeedActive=0;
    std::uint32_t carStage=0,carDriverChecks=0,carDriverLiving=0;
    std::uint32_t carPositionSamples=0,carVelocityFallbacks=0,carStationary=0,carHeld=0,carBusy=0,carMoveAttempts=0;
    std::uint32_t knifeStage=0,knifeFailure=0,knifePresses=0,knifeReleases=0,knifeLaunches=0,knifeFinalizes=0,knifeRefusals=0,knifeTimeouts=0;
    std::uint32_t knifeFlightSamples=0,knifeFlightsMoved=0,knifeFlightsStationary=0,knifeFlightUnavailable=0,knifeFlightActive=0;
    cfgspawn::Vec3 knifeFlightPosition{};float knifeFlightSpeed=0;std::uintptr_t knifeFlightUpdated=0;
    std::uint32_t requestedBots=0,selectedJasonIndex=0xFFFFFFFF;
    std::uint64_t submission=0;
    std::uint32_t convergenceProbes=0,convergenceActive=0,convergenceKeys=0,convergencePublished=0;
    std::uint32_t convergenceCleared=0,convergenceUnarmed=0,convergenceUnavailable=0,convergenceReadbackRefused=0;
    std::uint32_t convergenceFearWrites=0,convergenceFearRefused=0;
    std::uint32_t convergenceSchemaMask=0,convergenceSchemaBot=0,convergenceSchemaFailure=0;
    std::uint32_t travelAcquires=0,travelMoves=0,travelResumes=0,travelLost=0;
    std::uint32_t travelIdentityRefused=0,travelTickRefused=0,travelAlreadyDisabled=0,travelMoveRefused=0;
    std::uintptr_t knifeAimTarget=0;
    cfgspawn::Vec3 knifeAimPoint{},knifeLaunchPoint{},knifeLaunchVelocity{},knifeActualLaunch{},knifeActualVelocity{};
    std::uint32_t knifeLaunchObserved=0;
    cfgspawn::Vec3 knifeNativeLaunchPoint{},knifeNativeLaunchVelocity{};
    std::uint32_t knifeNativeLaunchSamples=0,knifeNativeLaunchRefused=0;
    std::uintptr_t knifeUpdatedBefore=0;
    std::uint32_t knifeCollisionBefore=0,knifeCollisionAfter=0;
    std::uint32_t knifeAcquiredCollision=0,knifeReleaseCollisionBefore=0,knifeReleaseCollisionAfter=0;
    std::uint32_t knifeAllOwnedStops=0,knifeStopStage=0,knifeNotifyEntries=0,knifeNotifyCorrections=0,knifeNotifyRefused=0,knifeNotifyStops=0;
    std::uint32_t knifeNotifyAdmissionRefused=0,knifeNotifyMaxReads=0;
    std::uintptr_t knifeLifecycleUpdated=0,knifeLifecycleThrownBy=0,knifeActualStopTarget=0;
    std::uint32_t knifeHeldStops=0,knifeHeldStopRefused=0,knifeStopMaxReads=0,emoteMovementStops=0,emoteBrainResumes=0;
    std::uint32_t radioObjectiveStage=0,radioObjectiveFailure=0,radioObjectiveSlices=0,radioObjectiveRetries=0;
    std::uint32_t radioObjectiveAttempts=0,radioObjectiveGetterReturns=0,radioObjectiveConfirmed=0;
    std::uintptr_t radioObjectiveActor=0,radioObjectiveGetterValue=0;
    std::uintptr_t radioObjectiveCandidate=0;std::int32_t radioObjectiveCandidateSerial=0;
    std::uint32_t radioObjectiveTypedStage=0;
    std::uint32_t swimChecks=0,swimActions=0,swimRefused=0,swimFailure=0,swimGuard=0,knifeNativeForwards=0;
    std::int32_t swimNativeAdmission=0;
    std::uint32_t runStateChecks=0,runStateAttempts=0,runStateReturns=0,runStateFailure=0;
    std::uint32_t convergenceFailureKey=9,convergenceExpectedBool=0,convergenceActualBool=0;
    std::uintptr_t convergenceExpectedObject=0,convergenceActualObject=0;
    std::uint32_t helperStage=0,helperFailure=0,helperDispatched=0,helperNulls=0,helperCandidate=0,helperReads=0;
    std::int32_t helperClassSerial=0,helperDefaultSerial=0;
    std::uint32_t helperStartState=0,helperStartCount=0,helperCatalogueCursor=0,helperSelectedEntry=0,helperPreparation=0;
    std::uint32_t helperResolveCalls=0,helperColdClasses=0,helperInvalidPackets=0,helperLoadRequests=0,helperLoadTimeouts=0;
    std::uintptr_t helperSelectedClass=0;
    std::uint32_t lastMoveResult=255,moveRefusals=0,resourceFailure=0;
    std::uint32_t doorChecks=0,doorAttempts=0,doorNativeReturns=0,doorUnavailable=0;
    std::uint32_t doorBreakRequests=0,doorOpenRequests=0,doorBreakIntent=0;
    std::uint32_t targetSweeps=0,pathPreserved=0,helperPositionSamples=0;
    cfgspawn::Vec3 helperProjected{},helperRequested{},helperActual{};
    std::uint32_t pursuitChecks=0,pursuitProjections=0,pursuitAttempts=0,pursuitReturns=0,pursuitUnavailable=0;
    std::uint32_t cabinBoundsStage=0,cabinBoundsFailure=0,cabinBoundsCount=0,cabinBoundsCaptured=0,cabinBoundsCalls=0;
    std::uint32_t additionalHelperRequests=0,additionalHelperRefusals=0;
    std::uint32_t sweaterChecks=0,sweaterBound=0,sweaterGrants=0,sweaterUses=0,sweaterRecharges=0;
    std::uint32_t sweaterWrites=0,sweaterUnavailable=0,sweaterStage=0;
    std::uint32_t sweaterFailure=0,sweaterHudAttempts=0,sweaterHudReturns=0,runWrites=0,runRefusals=0;
    std::uint32_t sweaterPickupStage=0,sweaterPickupInstalled=0,sweaterPickupCalls=0,sweaterPickupFailure=0;
    std::uint32_t sweaterPickupRefused=0,sweaterPickupBasePassed=0,sweaterPickupBaseDenied=0;
    std::uintptr_t sweaterPickupInteractor=0;
    std::uint32_t sweaterReadDeferrals=0,sweaterArmPending=0,sweaterArmAttempts=0,sweaterArms=0;
    std::uint32_t sweaterArmDeferrals=0,sweaterArmRejections=0,sweaterArmFailure=0;
    std::uint32_t durationState=0,skillState=0,settingsWrites=0;
    std::uint32_t loadoutState=0,loadoutCalls=0;
    std::uint32_t loadoutRequested=0,loadoutReady=0,loadoutFailed=0;
    std::uint32_t loadoutSlot=0,loadoutChoice=0,loadoutPhase=0,loadoutFailure=0,loadoutNativeStage=0;
    std::uint32_t loadoutSlices=0,loadoutVisited=0,loadoutCursor=0,loadoutTotal=0,loadoutReadDeferrals=0;
    std::uint32_t loadoutRestarts=0,loadoutLastReads=0,loadoutPreparations=0,loadoutGrantAttempts=0,loadoutVerifications=0;
    std::uintptr_t loadoutType=0;
    std::uint32_t convergenceAxeStage=0,convergenceAxeScanned=0,convergenceAxeAttempts=0,convergenceAxeGrants=0,convergenceAxeRefused=0;
    std::uint32_t objectiveScanned=0,objectiveProcessed=0,objectiveChanged=0,objectiveAlreadyBase=0,objectiveRefused=0;
    std::uint32_t objectiveRestored=0,objectiveRestoreRefused=0,objectiveDeferred=0,objectivePeakReads=0,objectiveFlags=0;
    std::uint32_t objectiveProcessedMask=0,objectiveChangedMask=0,objectiveBaseMask=0,objectiveRefusedMask=0;
    std::uint32_t objectiveRepairStage=0,objectiveRepairFailure=0,objectiveRepairHostFailure=0,objectiveRepairTarget=0;
    std::uint32_t objectiveRepairAttempts=0,objectiveRepairConfirmed=0,objectiveRepairPreserved=0,objectiveRepairDeferred=0;
    std::uint32_t objectiveRepairAttemptedMask=0,objectiveRepairConfirmedMask=0,objectiveRepairPreservedMask=0,objectiveRepairRefusedMask=0;
    std::uint32_t objectiveInstances=0,objectiveCandidate=0;
    std::uint32_t killAcceptedHits=0,killStances=0,killInteractions=0,killUnavailable=0,killStage=0,killEligibilityInstalled=0;
    std::uint32_t killDamageGate=0;
    std::uint32_t killGateFailedPhase=0,killGateFields=0,killGateOwners=0;
    std::uint64_t deathConfirmedAt=0,deathEndDue=0;
    std::uintptr_t initialJasonPawnState=0,initialJasonControllerState=0,initialRegisteredKillerState=0;
    std::uint32_t initialKillerStateReadMask=0;
    std::uint32_t defeatedOutroFlags=0,defeatedOutroFinished=0;
    std::uint32_t trapStage=0,trapFailure=0,trapObjectives=0,trapObjective=0,trapAttempts=0,trapConsumed=0,trapSkipped=0,trapProbes=0;
    std::uint32_t trapMapFamily=0;
    std::uint32_t trapShackStage=0,trapShackFailure=0,trapShackSlices=0,trapShackReady=0;
    std::uint32_t trapShackRestarts=0,trapCurrentKind=0;std::array<unsigned,4> trapOutcomes{};cfgspawn::Vec3 trapLastImpact{};
    std::array<unsigned,4> trapLastStages{},trapObjectiveProbes{};
    std::array<cfgspawn::Vec3,4> trapLastPawns{},trapLastStands{};
    std::array<unsigned,4> trapTravelFailures{},trapLastMoves{},trapLastMorphs{};
    std::array<unsigned,4> trapMorphProof{},trapProjectionCandidates{};
    std::array<cfgspawn::Vec3,4> trapMorphDestination{},trapMorphActual{};
    std::uintptr_t sweaterOwner=0;
    std::array<unsigned,4> trapKinds{},trapNativePlacement{},trapLegalPlacement{},trapStops{};
    std::array<cfgspawn::Vec3,4> trapImpacts{};
    unsigned objectiveStackChecks=0,objectiveStackVisits=0,objectiveStackAdmissions=0;
    unsigned objectiveCacheChecks=0,objectiveCacheMask=0,objectiveCacheAbsentMask=0,objectiveCacheRefusedMask=0;
    unsigned objectiveCachePublicationAttempts=0,objectiveCacheAcquisitionDeferrals=0;
    unsigned objectiveFinderGate=0,objectiveFinderParms=0,objectiveFinderBytes=0;
    unsigned outroRecoveryChecks=0,outroRecoveryStage=0,outroRecoveryFailure=0,outroRecoveryAttempts=0,outroRecoveryReturns=0;
    std::uint64_t outroObservedAt=0,outroElapsedMs=0;
    unsigned outroRecoveryDue=0;
    unsigned outroInactiveProof=0;
    unsigned outroFactoryAttempts=0,outroFactoryReturns=0;
    std::array<char,160> trapMapPackage{};
    std::array<Bot,MaximumBots> roster{};
    std::uintptr_t currentHumanTarget=0;
    std::uint32_t terminalCounselorMask=0,nativeTerminalChecks=0,nativeTerminalPasses=0;
    std::uint32_t endMatchAttempts=0,endMatchReturns=0,targetRetireStops=0;
    std::uint32_t postMatchChecks=0,postMatchObserved=0,postMatchFailure=0;
    std::array<char,33> postMatchPhase{};
    std::uint32_t nativeTerminalFailure=0,nativeTerminalIndex=0;std::uintptr_t nativeTerminalPawn=0,nativeTerminalPending=0;
    std::int32_t nativeTerminalCount=0,nativeTerminalHunter=0;
    std::uint32_t endingMetadataStage=0;
    std::uint32_t endingRepairStage=0,endingRepairFailure=0,endingRepairCalls=0;
    std::uint32_t botNameAttempts=0,botNameConfirmed=0,botNameSkipped=0,botNameFailure=0;
    std::uintptr_t endingNativeKiller=0,endingNativeKillerState=0,endingJasonState=0,endingSkin=0;
    std::uint32_t grabWarm=0,grabAttempts=0,grabReturns=0,grabHolds=0,grabKillAttempts=0,grabKillReturns=0,grabUnavailable=0,grabBusySkips=0;
    std::uint32_t killFailure=0,killResourceFailure=0;
    std::uint32_t killInteractionDeferrals=0,killInteractionAdmission=0,killInteractionReads=0,killInteractionCalls=0;
    std::uint32_t killRecoveryAttempts=0,killRecoveryReturns=0,killRecoveryFailure=0;
    float killInteractionDistanceSquared=0;
    std::uint32_t sweaterRetentionChecks=0,sweaterRetentionRestored=0,sweaterRetentionRefused=0,sweaterRetentionFailure=0,sweaterRetentionMaxReads=0;
    std::uint32_t emotePrepared=0,emoteStarts=0,emoteQueued=0,emoteAttempts=0,emoteReturns=0,emoteStarted=0;
    std::uint32_t emoteSkipped=0,emoteCancels=0,emoteRefused=0,emoteFailure=0;
    std::uint32_t emoteCancelAttempts=0,emoteCancelReturns=0,emoteStopped=0,emoteOwnershipLost=0;
    std::uint32_t rbPhase=0,rbShows=0,rbHides=0,rbRefused=0,rbAbandoned=0;
    std::uint32_t rbCapturedEdges=0,rbBlockedEdges=0,rbExpiredEdges=0;
    std::uint32_t rbProviderReady=0,rbPolls=0,rbForegroundPolls=0,rbConnectedPolls=0,rbHeldPolls=0;
    std::uint32_t soundBlipAttempts=0,soundBlipApplied=0,soundBlipRestores=0,soundBlipRestored=0,soundBlipRefused=0,soundBlipLost=0;
    std::uint32_t spectatorState=0,spectatorSelections=0,spectatorCameras=0,spectatorRestores=0,spectatorRefusals=0,spectatorFailure=0;
    std::uint32_t spectatorLinkFailure=0,spectatorLinkReads=0;
    std::uint32_t spectatorResourceBinding=0,spectatorResourceFailure=0,spectatorNativeBytes=0;
    cfgspectator::RefusalReceipt spectatorLastRefusal{};
    std::uint32_t spectatorAdmission=0;std::int32_t spectatorAdmissionSerial=0;
    std::uintptr_t spectatorPossessedPawn=0,spectatorStockPawn=0,spectatorCandidatePawn=0,spectatorAdmissionType=0,spectatorAdmissionOwner=0;
    std::uint32_t roadStage=0,roadFailure=0,roadPoints=0,roadSlices=0,roadLastReads=0,roadIntercepts=0;
    std::uintptr_t sweaterFreshClass=0;
    std::uint32_t sweaterFreshAttempts=0,sweaterFreshCreated=0,sweaterFreshFailure=0;
    std::uint32_t sweaterFreshAdmission=0,sweaterFreshAdmissionReads=0,sweaterFreshAdmissionCalls=0;
    std::uint32_t difficulty=1,difficultyHookReady=0,difficultyScaled=0,difficultyMaxReads=0;
    std::uint32_t soundEmitterReady=0,soundEmitterSuppressed=0,doorDecisionPhase=0;
    std::uint32_t trapPriorityHookReady=0,trapPriorityEvents=0,trapPriorityAccepted=0,trapPriorityRefused=0,trapPriorityActive=0,trapPriorityMaxReads=0;
    std::uint32_t sweaterClassAttempts=0,sweaterClassFailure=0;
    std::int32_t sweaterClassSerial=0,sweaterClassCdoSerial=0;
    std::uint32_t sweaterClassFlags=0,sweaterClassCdoFlags=0;
    std::uint32_t sweaterClassAnchor=0;
    std::uint32_t sweaterTemplateStage=0;
    std::uintptr_t sweaterTemplateSourceClass=0,sweaterTemplateSourceDefault=0,sweaterTemplateObject=0,sweaterTemplateClass=0;
    std::int32_t sweaterTemplateSourceSerial=0,sweaterTemplateSerial=0;
    std::uint32_t travelMoveFailure=0,travelMoveNativeResult=255,roadLandingObserved=0;
    cfgspawn::Vec3 roadRequested{},roadActual{},roadCar{},roadDirection{};
    float roadSourceAlignment=0,roadSourceLane=0,roadLaneCorrection=0;
    std::uint32_t roadTraceState=0,roadTraceSegments=0;
    float roadTraceNearest=0,roadTraceLateral=0;
    std::uint32_t completionProbes=0,completionFailure=0,trapPrepared=0;
    std::uint64_t sweaterHudQueuedAt=0,sweaterHudFirstServiceAt=0;
    std::uint32_t sweaterHudProofFailure=0;
    std::uint32_t doorBreakStarts=0,doorBreakObservations=0,doorBreakEnds=0,doorForeignLockWaits=0;
    std::uintptr_t doorBreakTarget=0,doorBreakOwner=0;
    std::uint64_t spectatorCycleEvents=0;
    std::uint32_t spectatorNativeCycles=0;
    std::uint32_t nativePursuitAcquires=0,nativePursuitRefusals=0;
    std::uint32_t spectatorNativeEntries=0,spectatorCycleRejection=0,spectatorDecision=0,spectatorRosterCount=0;
    std::int32_t spectatorCycleDirection=0;
    std::uintptr_t spectatorCycleController=0,spectatorObservedState=0,spectatorStatusFunctionHint=0,spectatorStatusWidgetHint=0;
    std::array<std::uintptr_t,16> spectatorRosterStates{};
    std::uint32_t hidingPhase=0,hidingAttempts=0,hidingTimeouts=0,hidingRecoveries=0,hidingRefusals=0;
    std::uintptr_t hidingCounselor=0,hidingSpot=0,hidingComponent=0;
    std::uint32_t carSlamObserved=0,carRouteCommitted=0,carSeatBatch=0;
    std::uint32_t spectatorStatusEvents=0,spectatorStatusRefusals=0,spectatorStatusJasonView=0;
    std::uint32_t spectatorDispatcherReady=0;
    std::uint32_t spectatorLabelOverrides=0;
    std::uintptr_t spectatorStatusWidget=0;
    std::array<char16_t,96> spectatorStatusText{};
    std::array<char16_t,64> spectatorStatusNextMode{};
    std::uintptr_t objectiveOwnerWidget=0,objectiveOwnerResult=0,objectiveOwnerExpected=0;
    std::uint32_t objectiveOwnerFailure=0;
    std::uint32_t objectivePanelStage=0,objectivePanelChild=0,objectivePanelFields=0;
    std::int32_t objectivePanelOffset=0,objectivePanelSerial=0,objectiveSlotSerial=0,objectiveChildSerial=0;
    std::uintptr_t objectivePanelPointer=0,objectivePanelSlot=0,objectivePanelContent=0,spectatorNativePawn=0;
    std::uint32_t knifeTrailCount=0;
    std::array<std::uint32_t,8> knifeTrailMs{};
    std::array<cfgspawn::Vec3,8> knifeTrailPosition{},knifeTrailTarget{},knifeTrailVelocity{};
    std::array<char,33> completionPhase{};
    std::uint32_t sweaterDiscoveryCandidates=0,sweaterDiscoveryResult=0;
    std::uintptr_t sweaterDiscoverySource=0,sweaterDiscoveryAbility=0,sweaterDiscoveryWearer=0,sweaterDiscoveryOwner=0;
    std::uint32_t footBurstStage=0,footBurstMask=0,footBurstStarts=0,footBurstRestores=0,footBurstRefused=0;
    std::uint64_t footBurstUntil=0,footBurstReadyAt=0;
    std::uint32_t initializationStage=0;
    std::uint32_t rootOwners=0,rootClaims=0,rootPoisoned=0,rootLastFailure=0,rootWindowStage=0,rootWindowGate=0;
    std::uint64_t rootCandidates=0,rootEntered=0,rootRefusals=0,rootWindowAttempts=0,rootLockRetries=0,rootWindowRefusals=0;
    std::uint32_t rootReferencerFailure=0,rootStrandedClaims=0;
    std::uint64_t rootEmptyRetirements=0;
    std::array<KnifeBoundarySample,8> knifeBoundaries{};
    std::uint32_t knifeBoundaryCount=0,knifeBoundaryOverflow=0,actorRetentionPeakReads=0;
    std::uint32_t knifeAdapterStage=0,knifeAdapterFailure=0,knifeAdapterReads=0,knifeAdapterCalls=0,knifeAdapterEntryReads=0;
    std::uint32_t rootDeferredClaims=0;
    std::uint64_t rootDeferredRetirements=0,rootDrainedRetirements=0;
    cfgspawn::Vec3 knifeNativeOrigin{},knifeNativePost{};
    float knifeNativePostSpeed=0;std::uint32_t knifeNativeOriginSamples=0;
    std::uintptr_t knifeNativePostUpdated=0;
    std::uint64_t f3Polls=0,f3Foreground=0,f3DownSamples=0,f3ShortSamples=0;
    std::uint32_t sweaterUseEdgeSource=0,sweaterOwnershipUses=0,sweaterAbilityUses=0,sweaterRetiredAbilities=0;
    std::uint64_t zeroOutputCompletions=0,zeroOutputContendedCompletions=0,zeroOutputProbeUnknown=0;
    struct DonorRosterRow {
        std::uintptr_t pawn=0;std::uint64_t at=0,liveObservations=0;
        std::uint32_t kind=0,result=0,blocked=0,escaped=0,live=0;
    };
    std::array<DonorRosterRow,64> donorRoster{};
    std::uint32_t donorRosterRows=0,donorCensusComplete=0,donorRegistered=0,donorLive=0;
    std::uint64_t donorCensusCells=0,donorTrapActiveMs=0;
    std::uintptr_t donorTrapVictim=0;
    struct DonorSelection {std::uint64_t at=0;std::uintptr_t pawn=0;float distance=0;std::uint32_t reason=0;};
    struct DonorDriverRows {
        std::array<DonorSelection,16> selected{};std::uint32_t count=0,cursor=0;
        std::array<std::uint64_t,16> lapses{},resumes{};
    };
    std::array<DonorDriverRows,2> donorDrivers{};
    struct SweaterMatch {std::uintptr_t world=0;std::uint64_t startedAt=0,lastUseAt=0,lastRechargeAt=0;
        std::uint32_t uses=0,recharges=0;};
    std::array<SweaterMatch,8> sweaterMatches{};std::uint32_t sweaterMatchCount=0;
    struct J2HeldRow {
        std::uintptr_t jason=0,victim=0;
        std::uint32_t stage=0,queried=0,available=0,before=0,after=0,slot=7,scans=0,pickedCount=0;
        std::uint64_t attempts=0,returns=0,uncertain=0,probes=0,holds=0;
        float travelled=0;
        std::array<std::uintptr_t,7> actors{},classes{},picked{};
        std::array<std::array<wchar_t,192>,7> classPaths{},pickedPaths{};
    };
    std::array<J2HeldRow,2> j2Held{};
    std::uint64_t j2ConvergenceTurns=0,j2ConvergenceGrants=0,j2ConvergenceAmbiguous=0,j2ConvergenceDeferrals=0;
    std::uint64_t j2ConvergencePublications=0,j2ConvergenceBrainResumes=0,j2ConvergenceBrainLost=0;
    std::uint64_t j2ConvergenceUncertainPublications=0;
    std::uint32_t j2ConvergenceActive=0,j2ConvergenceTravel=0;
    std::uint32_t j2EmoteDeduplicated=0,j2EmoteExpiries=0,j2EmoteWorldCancels=0;
    std::uint32_t j2EmoteBudgetDeferrals=0;
    std::array<std::uint32_t,8> j2EmoteRefusals{};
    std::array<std::uintptr_t,4> j2EmoteAssets{};
    std::array<std::array<wchar_t,192>,4> j2EmotePaths{};
    std::array<cfgj21::Cost,cfgj21::StageCount> j21Costs{};
    std::uint32_t j21ConvergenceEnabled=cfgj21::ConvergenceEnabled?1u:0u,j21EmotesEnabled=cfgj21::EmotesEnabled?1u:0u;
    std::uint32_t j21EmoteProofStage=0,j21EmoteProofDifference=0;
    std::uint32_t j21EmoteProofReads=0,j21EmoteProofError=0;
    std::uint64_t j21ObjectiveTurns=0,j21RosterFailures=0;
    cfgj21::EpochBudget j21Epoch{}; // value-only diagnostics, no runtime authority on the reader
    std::uint32_t j21LeanEnabled=cfgj21::LeanEnabled?1u:0u;
    cfglean::Counters j21Lean{};
    std::uint32_t j21LeanBot=cfgconvergence::MaximumBots,j21LeanPhase=0,j21LeanKey=0;
    std::array<cfgj21c::HeldDiagnostics,2> j21cHeld{};
    std::array<std::uint64_t,2> j3HardMovementWrites{};
    std::uint64_t j3HardMovementRefused=0,j3EasyDamageScaled=0,j3HardDamageScaled=0;
    std::uintptr_t j3BotSweaterOwner=0;
    std::uint64_t j3LateTargetRetries=0;
    std::array<cfggrabtiming::Report,2> j23GrabTiming{};
    std::uint64_t j23EmoteReadDeferrals=0,j23EmoteBudgetDropped=0;
    std::uint64_t j23NearFollowMoves=0,j23NearFollowDisabled=0,j23NearFollowRefused=0;
    std::array<cfglean::ArrivalReport,cfgconvergence::MaximumBots> j23Arrival{};
    cfgpractice::Report j215Practice{}; // append only: preserve the complete schema72 prefix
#if CFG_J217_FINISHING
    cfgpractice::HudDiagnostics j217Hud{};
#endif
    // Count only a native extraction that was observed active, then ended with
    // its proven victim out of the seat and terminal/dead.
    std::uint32_t carExtractionCompleted=0;
    std::array<std::uint32_t,2> grabFinished{};
    std::uint32_t carExtractionCommitted=0;
    // H2.28a diagnostic append only; preserve all prior report offsets.
    struct SweaterOwnerChange {std::uint64_t tick=0;std::uintptr_t previous=0,current=0;};
    std::uint32_t sweaterFirstRefusalRecorded=0,sweaterFirstRefusalCaller=0,sweaterFirstRefusalFailure=0,sweaterFirstRefusalStage=0;
    std::uint32_t sweaterFirstRefusalFreshFailure=0;
    std::uint64_t sweaterFirstRefusalTick=0;std::uintptr_t sweaterFirstRefusalOwner=0;
    std::uint32_t sweaterOwnerChangeCount=0,sweaterOwnerChangeOverflow=0;
    std::array<SweaterOwnerChange,8> sweaterOwnerChanges{};
    // H2.29 append-only hosted fields preserve every schema72 prefix offset.
    // Last native held weapon observed for each hosted counselor: 0 unknown,
    // 1 axe, 2 machete, 3 other, 4 native query confirmed empty.
    std::array<std::uint32_t,MaximumBots> convergenceWeaponResult{};
    std::uint32_t emoteStartedUnchanged=0,emoteStartedOtherMontage=0,emoteStartedReadRefused=0;
    std::uint32_t hidingSelectionChecks=0,hidingSelectionStage=0;
    std::uint32_t hidingPhaseStartStage=0,hidingSafeFailure=0;
    std::uintptr_t hidingCandidate=0,hidingCandidateSpot=0;
    std::uint32_t hidingCandidatesSeen=0,hidingCandidatesEligible=0,hidingChosenStage=0,hidingRosterCursor=0;
    std::uintptr_t hidingChosenPawn=0;
    struct DonorMovementRows {
        std::array<std::uint64_t,4> nearestDistanceBuckets{}; // <20m,20-50m,50-90m,>=90m
        std::array<std::uint64_t,8> distanceTeleportRefusals{};
        std::array<std::uint64_t,8> fiftyMorphRefusals{};
        std::array<std::uint64_t,static_cast<unsigned>(cfgdonorhunt::BusyCause::Count)> busyCauseHits{};
        std::array<std::uint64_t,4> navProjectOutcomes{},navTeleportOutcomes{}; // NotEntered, false, true, uncertain
        std::uint64_t runningMs=0,distanceTeleportAttempts=0,distanceTeleportConfirmed=0;
        std::uint64_t fiftyMorphAttempts=0,fiftyMorphConfirmed=0;
        std::uint32_t navStage=0,navCandidate=0,navPurpose=0;
    };
    std::array<DonorMovementRows,2> donorMovement{};
    std::uint32_t hostedEndSamples=0,hostedEndPhaseChanges=0,hostedEndPawnChanges=0,hostedEndReadFailure=0;
    std::uintptr_t hostedEndPawn=0;std::array<char,33> hostedEndPhase{};
    // H2.40 read attribution is append-only to preserve published payload offsets.
    std::uint32_t sweaterTurnOwnerReads=0,sweaterTurnContextReads=0,sweaterTurnPamelaReads=0,sweaterTurnClassReads=0;
    std::uint32_t sweaterTurnDiscoveryReads=0,sweaterTurnHostedReads=0,sweaterTurnPreviousReads=0,sweaterTurnAdmissionReads=0;
    std::uint32_t emoteBindClause=0,emoteBindReads=0;
    std::array<std::array<std::uint64_t,4>,2> j2HeldStageReached{}; // append-only: Probing,Scanning,Walking,Committed
    // H2.43 roster counters (report only, append-only): follow of the local controller's
    // current pawn (Tommy after return) into the single donor roster, and why it was rejected.
    std::uint32_t donorLocalPawnChanges=0,donorLocalAdmitted=0,donorLocalRejected=0,donorLocalLastResult=0,
        donorLocalLastRegistered=0,donorNativeListRefusals=0,donorNativeListLastCount=0,donorLocalRowDropped=0;
    std::uintptr_t donorLocalLastPawn=0;
    // H2.44 (report only, append-only): why each donorRoster row is not Live.
    std::array<std::uint8_t,64> donorRosterLiveWhy{};
    // H2.45 emote hosted read-budget counters (report only, append-only).
    std::uint32_t emoteFailSite=0,emoteFailClause=0,emoteFailBindReads=0,emoteHookPeakReads=0;
    // H2.46 (report only, append-only): donor nav-ring truth. Attempts in donorMovement count a ring
    // START (also pending); these count how each ring ENDED and why candidates were rejected.
    struct H246NavRows {
        std::array<std::uint32_t,6> ringBegun{},ringSucceeded{},ringFailed{}; // by NavRingPurpose
        std::uint32_t dzRejects=0,horizontalRejects=0,roofChecks=0,roofRejects=0;
        std::uint32_t probeStarved=0,probeStarvedRetried=0,probeStarvedGaveUp=0;
        std::uint32_t stuckDoorCalls=0,stuckFailsafeCalls=0; // offline-inclusive (H234Recovery StuckAttempts is hosted-only)
    };
    std::array<H246NavRows,2> h246Nav{};
    // H2.47 (report only, append-only): held grab-kill wait split (CurrentSeat vs extraction) and stale-seat proof.
    struct H247HeldRows {
        std::uint64_t seatWaitsSeat=0,seatWaitsExtracting=0,staleSeatIgnored=0,seatProofFailures=0,scanSkippedStruggling=0;
        std::uint64_t commitGuardWaits=0,postCommitReleases=0,postCommitKills=0,offlineF1Releases=0; // H2.80/H2.82, appended after the schema72 prefix
    };
    std::array<H247HeldRows,2> h247Held{};
    // H2.81 (report only, appended after the schema72 prefix): water/frozen unstick teleports per Jason role.
    struct H281UnstickRows {std::uint32_t waterEscapes=0,waterRefused=0,frozenEscapes=0,frozenRefused=0,lastReason=0;};
    std::array<H281UnstickRows,2> h281Unstick{};
    // H2.47 mob counters (report only, append-only). Stage: 1=bot 2=ambiguous/turn/postgrab 3=class 4=observe 5=not-entered 6=entered
    struct H247MobRows { std::uint64_t grantRefused=0,mobHeld=0,mobReleased=0;std::array<std::uint64_t,8> grantStage{}; };
    H247MobRows h247Mob{};
    // H2.48 (report only, append-only): why ObserveDonorCounselor refuses (failure==1 sub-clause, 1..10, see
    // DonorCounselorObservation.hpp). Per roster row: last first-failing clause, raw context-kill/bHasEscaped bytes
    // at that failure (0xFF = unread). observeBudgetFailures counts failed observations that hit the read cap.
    std::array<std::uint32_t,12> h248ObserveClause{};
    std::array<std::uint8_t,64> h248RosterClause{},h248RosterCtxRaw{},h248RosterEscRaw{};
    std::uint32_t h248ObserveBudgetFailures=0;
    // H2.48 Tommy slot (report only, append-only): clause-4 reason histogram (index = slotWhy), per-row last reason,
    // last item flags / expected+fresh serials at a clause-4 refusal, and the failure-22 sweater HUD proof
    // sub-clause histogram (1=code pin 2=controller->pawn 3=pawn->controller 4=ability field 5=ability owner
    // 6=Used==0 7=lookup 8=call cap).
    std::array<std::uint32_t,8> h248SlotWhy{};std::array<std::uint8_t,64> h248RosterSlotWhy{};
    std::uint32_t h248SlotFlags=0;std::int32_t h248SlotExpectedSerial=-1,h248SlotFreshSerial=-1;
    std::array<std::uint32_t,10> h248HudClause{};
    std::uint32_t knifeAbortStages[10]={},knifeAbortOwnerObserve=0,knifeFirstAbortStage=0,knifeFirstAbortFailure=0,knifeFirstAbortReads=0,knifeFirstAbortCalls=0,knifeFirstAbortRecorded=0; // H2.77
    std::uint32_t soundInstall=0,soundEntries=0,soundHostedCalls=0,soundGate=0;std::int32_t soundMh=0;std::uintptr_t soundLastKiller=0; // H2.77 diagnostics
    std::uint32_t vetoCount=0,vetoTolerated=0,vetoStage=0,vetoSerializing=0,vetoCollecting=0,vetoScoped=0;std::uintptr_t vetoFunction=0; // H2.77 post-spawn lookup veto
    // H2.83 (report only, appended after the schema72 prefix): held grab-kill completion evidence per Jason role (0 primary, 1 F1).
    // Hits/kills: counselor TakeDamage with this Jason as causer (kill = incoming>=1000, native ForceKillCharacter's MaxHealth^2).
    // Gates: native USCContextKillComponent::EndAction (0x35B520) victim gates, read once per committed hold at commit+1.5 s. 255=unread.
    struct H283KillRows {
        std::uint64_t hits=0,hitsZero=0,kills=0,killsApplied=0,killsZero=0,snapshots=0,releasedAlive=0,releasedDead=0,snapCommit=0;
        float lastKillIncoming=0,lastKillResult=0,lastKillHealthBefore=-1,lastKillHealthAfter=-1,victimHealth=-1,releaseHealth=-1;
        std::uint32_t lastKillOwnInstigator=255,clause=0,ownerRole=255,compKiller=255,compVictim=255,perWeapon=255,weapon=255,mapEntries=255,
            matchedEntry=255,moveKillFlag=255,victimContext=255,victimInvulnerable=255,releaseContext=255;
    };
    std::array<H283KillRows,2> h283Kill{};
    struct H284CutoffRows {std::uint64_t pursuitMorphs=0,led=0,noHeading=0,landedClose=0,landedRing=0;};
    std::array<H284CutoffRows,2> h284Cutoff{};
    std::array<std::uint32_t,2> h285Starts{},h285ChaseStarts{};std::uint64_t h285BurstRequests=0,h285BeginBlocked=0;
    std::uint64_t h286F1NewMatchResets=0;
    // H2.91 timing only: held-lane native query invokes / GrabKillInput calls, and the callback GC root windows (copied from bundleRootDiagnostics).
    struct H291Cost {std::uint64_t count=0,us=0,maxUs=0,overFrame=0;};
    H291Cost h291HeldQuery{},h291HeldInput{},h291HeldDrive{};
    std::uint64_t h291RootWindows=0,h291RootWindowUs=0,h291RootWindowMaxUs=0,h291RootSleeps=0,h291RootSleepUs=0,h291RootBeginUs=0,h291RootOperationUs=0,h291RootEndUs=0,h291RootRetries=0,h292RootProvenSkips=0;
    std::array<std::uint32_t,2> h288FailedRings{},h288OtherTargets{}; // unstick: rings with no walkable cell, escapes redirected to another counselor
    std::array<std::uint64_t,4> h286NearestEmpty{};std::array<std::uint64_t,5> h286NearestSkips{};std::uint64_t h286NearestChosen=0; // decision-frame target choice (copied from cfgdonorframe::Frame::Stats) // previous match's F1 state cleared on the first F1 press in a new world
    struct H286LeadRows {std::uint64_t led=0,unled=0;};std::array<H286LeadRows,2> h286Lead{}; // burst moves aimed 2 s ahead / left on the counselor
    std::array<std::uint64_t,8> h285Veto{}; // 0=knife probe pending but allowed(chase) 1=car 2=carSpeed 3=traps 4=hiding 5=door 6=knife throw 7=knife probe(>30 m) // primary burst requests from the hunt (CFG_H285_CHASE_BURST); starts/chase starts come from the policies // pursuit Morph cut-off (CFG_H284_MORPH_CUTOFF), role 0 primary 1 F1
    std::uint64_t h283ClassAttempts=0,h283ClassApplied=0,h283ClassAlready=0;std::uint32_t h283ClassRefusal=0; // offline F1 ActiveCharacterClass (CFG_H283_F1_KILLER_CLASS)
};
inline void RecordSweaterFirstRefusal(Report& r,unsigned caller,unsigned failure,unsigned stage,
    std::uintptr_t owner,std::uint64_t tick,unsigned freshFailure=0)noexcept{
    if(r.sweaterFirstRefusalRecorded)return;
    r.sweaterFirstRefusalRecorded=1;r.sweaterFirstRefusalCaller=caller;
    r.sweaterFirstRefusalFailure=failure;r.sweaterFirstRefusalStage=stage;
    r.sweaterFirstRefusalOwner=owner;r.sweaterFirstRefusalTick=tick;r.sweaterFirstRefusalFreshFailure=freshFailure;
}
inline void RecordSweaterOwnerChange(Report& r,std::uintptr_t previous,std::uintptr_t current,
    std::uint64_t tick)noexcept{
    if(previous==current)return;
    const auto n=r.sweaterOwnerChangeCount;
    if(n<r.sweaterOwnerChanges.size())r.sweaterOwnerChanges[r.sweaterOwnerChangeCount++]={tick,previous,current};
    else if(r.sweaterOwnerChangeOverflow<UINT32_MAX)++r.sweaterOwnerChangeOverflow;
}
static_assert(offsetof(Report,j215Practice)==28160,"J215 preserves every schema72 payload offset");
inline bool Valid(const Report& r) {
#if CFG_J217_FINISHING
    if(!r.j217Hud.Valid())return false;
#endif
    for(const auto finished:r.grabFinished)if(finished>64)return false;
    if(r.carExtractionCommitted>64)return false;
    if(r.sweaterFirstRefusalRecorded>1||r.sweaterFirstRefusalFreshFailure>4||r.sweaterOwnerChangeCount>r.sweaterOwnerChanges.size()||
       (r.sweaterFirstRefusalRecorded&&(!r.sweaterFirstRefusalCaller||!r.sweaterFirstRefusalStage)))return false;
    if((r.j215Practice.options&~cfgpractice::KnownMask)||r.j215Practice.voiceHookReady>1||r.j215Practice.voiceHookFailure>3||
       r.j215Practice.peakReads>8192||r.j215Practice.hudStage>2||r.j215Practice.hudNodes>192)return false;
    for(const auto& arrival:r.j23Arrival)if(!cfglean::ValidArrival(arrival))return false;
    for(const auto& timing:r.j23GrabTiming)if(!cfggrabtiming::Valid(timing))return false;
    for(const auto& d:r.j21cHeld)if(d.observeStage>8||d.queryStage>3||d.inputStage>6)return false;
    if(r.j21ConvergenceEnabled>1||r.j21EmotesEnabled>1||r.j21LeanEnabled>1)return false;
    if(r.j21LeanBot>cfgconvergence::MaximumBots||r.j21LeanPhase>(CFG_J21C_LEAN_SLICES?5u:4u)||r.j21LeanKey>cfgconvergence::KeyCount)return false;
    for(const auto& cost:r.j21Costs)if(cost.over4ms>cost.count||cost.overFrame>cost.over4ms||cost.maximum>cost.total)return false;
    if(r.j21EmoteProofStage>6||r.j21EmoteProofDifference>2||r.j21Epoch.source>2||
       !std::isfinite(r.j21Epoch.primarySeconds)||!std::isfinite(r.j21Epoch.extraSeconds)||
       r.j21Epoch.primarySeconds<0||r.j21Epoch.extraSeconds<0)return false;
    if(r.j2ConvergenceActive>1||r.j2ConvergenceTravel>64)return false;
    for(const auto& path:r.j2EmotePaths)if(path.back())return false;
    for(const auto& held:r.j2Held){
        if(held.stage>6||held.queried>127||held.available>127||held.before>127||held.after>127||held.slot>7||held.scans>1||
            held.pickedCount>64||!std::isfinite(held.travelled)||held.travelled<0)return false;
        for(const auto& path:held.classPaths)if(path.back())return false;
        for(const auto& path:held.pickedPaths)if(path.back())return false;
    }
    if(r.donorRosterRows>64||r.donorCensusComplete>1||r.donorRegistered>64||r.donorLive>64||r.sweaterMatchCount>8)return false;
    for(const auto& driver:r.donorDrivers)if(driver.count>16||driver.cursor>=16)return false;
    if(r.sweaterUseEdgeSource>2||r.sweaterOwnershipUses>r.sweaterUses||r.sweaterAbilityUses>r.sweaterUses)return false;
    if(r.f3Foreground>r.f3Polls||r.f3DownSamples>r.f3Foreground||r.f3ShortSamples>r.f3Foreground)return false;
    if(!cfgspawn::Finite(r.knifeNativeOrigin)||!cfgspawn::Finite(r.knifeNativePost)||
       !std::isfinite(r.knifeNativePostSpeed)||r.knifeNativePostSpeed<0)return false;
    if(r.rootDeferredClaims>1024||r.rootDrainedRetirements>r.rootDeferredRetirements)return false;
    if(r.knifeAdapterStage>8||r.knifeAdapterFailure>6||r.knifeAdapterReads>ReadCap||r.knifeAdapterCalls>CallCap||r.knifeAdapterEntryReads>ReadCap)return false;
    if(r.knifeBoundaryCount>r.knifeBoundaries.size()||r.actorRetentionPeakReads>512u*CallCap)return false;
    for(unsigned i=0;i<r.knifeBoundaryCount;++i){const auto& sample=r.knifeBoundaries[i];
        if(!sample.returnPc||!sample.subject||sample.kind<1||sample.kind>2||sample.flags>7||sample.knifeStage>9||!sample.count)return false;}
    return r.magic==ReportMagic&&r.version==ReportVersion&&r.bytes==sizeof(Report)&&!(r.generation&1)&&r.carExtractionCompleted<=64&&r.sweaterDiscoveryResult<=4&&
        r.initializationStage<=static_cast<unsigned>(InitializationStep::Ready)&&r.rootOwners<=32&&r.rootClaims<=1024&&r.rootPoisoned<=1&&r.rootLastFailure<=6&&
        r.rootReferencerFailure<=4&&r.rootStrandedClaims<=1024&&
        r.footBurstStage<=3&&r.footBurstMask<=15&&r.footBurstRestores<=r.footBurstStarts&&
        r.spectatorCycleRejection<=8&&r.spectatorDecision<=9&&r.spectatorRosterCount<=16&&r.knifeTrailCount<=8&&
        r.difficulty<=2&&r.difficultyHookReady<=1&&r.difficultyMaxReads<=ReadCap&&r.soundEmitterReady<=1&&r.doorDecisionPhase<=2&&
        r.completionFailure<=7&&r.objectiveOwnerFailure<=5&&r.objectivePanelStage<=12&&r.objectivePanelChild<64&&r.completionPhase.back()==0&&r.trapPrepared<=12&&
        r.postMatchFailure<=2&&r.postMatchPhase.back()==0&&r.rbProviderReady<=1&&r.sweaterTemplateStage<=5&&
        r.sweaterClassAttempts<=8&&(r.sweaterClassFailure<=5||(r.sweaterClassFailure>=30&&r.sweaterClassFailure<=33))&&
        r.sweaterClassSerial>=0&&r.sweaterClassCdoSerial>=0&&r.sweaterClassAnchor<=9&&r.travelMoveFailure<=4&&
        r.travelMoveNativeResult<=255&&r.roadLandingObserved<=1&&
        cfgspawn::Finite(r.roadRequested)&&cfgspawn::Finite(r.roadActual)&&cfgspawn::Finite(r.roadCar)&&cfgspawn::Finite(r.roadDirection)&&
        std::isfinite(r.roadSourceAlignment)&&std::isfinite(r.roadSourceLane)&&std::isfinite(r.roadLaneCorrection)&&
        r.roadTraceState<=4&&std::isfinite(r.roadTraceNearest)&&r.roadTraceNearest>=0&&std::isfinite(r.roadTraceLateral)&&
        r.trapPriorityHookReady<=1&&r.trapPriorityActive<=1&&r.trapPriorityMaxReads<=ReadCap&&r.trapPriorityAccepted<=r.trapPriorityEvents&&
        r.state<=static_cast<unsigned>(State::Stopped)&&r.stop<=static_cast<unsigned>(Stop::Limit)&&r.releaseAbandoned<=1&&
        r.bots<=MaximumBots&&r.spawned<=MaximumBots&&r.spawnAttempts<=MaximumHelperAttempts&&r.releasePending<=1&&
        r.carStage<=15&&r.carDriverLiving<=r.carDriverChecks&&
        r.roadStage<=4&&r.roadFailure<=13&&r.roadPoints<=512&&r.roadSlices<=1025&&r.roadLastReads<=ReadCap&&
        r.sweaterFreshCreated<=r.sweaterFreshAttempts&&r.sweaterFreshFailure<=4&&
        r.cabinBoundsStage<=4&&r.cabinBoundsFailure<=12&&r.cabinBoundsCount<=128&&r.cabinBoundsCaptured<=r.cabinBoundsCount&&r.cabinBoundsCalls<=384&&
        r.botNameAttempts<=MaximumBots&&r.botNameConfirmed<=r.botNameAttempts&&r.botNameSkipped<=MaximumBots&&r.botNameFailure<=9&&
        r.objectiveProcessed<=25&&r.objectiveChanged+r.objectiveAlreadyBase<=25&&r.objectiveRestored<=r.objectiveChanged&&
        r.objectivePeakReads<=ReadCap&&r.objectiveFlags<=31&&r.objectiveProcessedMask<0x2000000&&
        r.objectiveChangedMask<0x2000000&&r.objectiveBaseMask<0x2000000&&r.objectiveRefusedMask<0x2000000&&
        r.objectiveRepairStage<=7&&r.objectiveRepairFailure<=8&&r.objectiveRepairHostFailure<=8&&r.objectiveRepairTarget<=5&&
        r.objectiveRepairAttempts<=5&&r.objectiveRepairConfirmed<=r.objectiveRepairAttempts&&r.objectiveRepairPreserved<=5&&
        r.objectiveRepairAttemptedMask<=31&&r.objectiveRepairConfirmedMask<=31&&r.objectiveRepairPreservedMask<=31&&r.objectiveRepairRefusedMask<=31&&
        r.objectiveInstances<=8&&r.objectiveCandidate<=8&&
        r.objectiveCacheChecks<=42&&r.objectiveCacheMask<=3&&r.objectiveCacheAbsentMask<=3&&r.objectiveCacheRefusedMask<=3&&
        r.objectiveCachePublicationAttempts<=3&&r.objectiveCacheAcquisitionDeferrals<=80&&r.outroRecoveryDue<=1&&r.outroInactiveProof<=1&&r.sweaterFreshAdmission<=4&&
        r.objectiveFinderGate<=3&&r.objectiveFinderParms<=255&&r.objectiveFinderBytes<=65535&&
        r.outroRecoveryStage<=2&&r.outroRecoveryFailure<=5&&r.outroRecoveryAttempts<=2&&r.outroRecoveryReturns<=r.outroRecoveryAttempts&&
        r.outroFactoryAttempts<=3&&r.outroFactoryReturns<=3&&(r.outroFactoryReturns&r.outroFactoryAttempts)==r.outroFactoryReturns&&
        r.spectatorAdmission<=9&&r.spectatorAdmissionSerial>=0&&
        r.knifeStage<=9&&ValidKnifeFailure(r.knifeFailure)&&r.endingRepairStage<=9&&r.endingRepairFailure<=27&&r.endingRepairCalls<=4&&
        r.terminalCounselorMask<cfglimits::TerminalMaskLimit&&r.endMatchAttempts<=1&&r.endMatchReturns<=r.endMatchAttempts&&r.grabWarm<=6&&
        r.grabReturns<=r.grabAttempts&&r.grabKillReturns<=r.grabKillAttempts&&
        r.spectatorState<=4&&r.spectatorFailure<=34&&r.spectatorLinkFailure<=12&&r.spectatorLinkReads<=ReadCap&&
        r.spectatorLastRefusal.count<=r.spectatorRefusals&&r.spectatorLastRefusal.failure<=34&&
        r.spectatorLastRefusal.decision<=9&&r.spectatorLastRefusal.link<=12&&
        r.spectatorLastRefusal.reads<=ReadCap&&r.spectatorLastRefusal.calls<=CallCap&&r.spectatorLastRefusal.roster<=16&&
        // Preserve even a rejected uint16 native frame in the report. This
        // diagnostic range never authorizes a call; NativeFrame stays exact.
        r.spectatorResourceBinding<=4&&r.spectatorResourceFailure<=5&&r.spectatorNativeBytes<=65535&&
        r.spectatorLastRefusal.binding<=4&&r.spectatorLastRefusal.resource<=5&&r.spectatorLastRefusal.nativeBytes<=65535&&
        r.emotePrepared<=2&&r.emoteFailure<=6&&r.emoteReturns<=r.emoteAttempts&&r.emoteStarted<=r.emoteReturns&&
        r.emoteStartedUnchanged<=r.emoteReturns&&r.emoteStartedOtherMontage<=r.emoteReturns&&r.emoteStartedReadRefused<=r.emoteReturns&&
        r.emoteCancelReturns<=r.emoteCancelAttempts&&r.emoteStopped<=r.emoteCancelReturns&&r.rbPhase<=10&&
        r.soundBlipAttempts<=1&&r.soundBlipApplied<=1&&r.soundBlipRestores<=1&&r.soundBlipRestored<=r.soundBlipRestores&&r.soundBlipLost<=1&&
        r.helperStage<=static_cast<unsigned>(HelperStage::Indeterminate)&&
        r.helperFailure<=static_cast<unsigned>(HelperFailure::Slots)&&r.helperDispatched<=1&&
        r.helperNulls<=r.spawnAttempts&&r.helperCandidate<=HelperCandidates&&r.helperReads<=ReadCap&&
        r.helperClassSerial>=0&&r.helperDefaultSerial>=0&&
        r.helperStartState<=3&&r.helperStartCount<=64&&r.helperCatalogueCursor<=64&&r.helperSelectedEntry<64&&r.helperPreparation<=(CFG_J21C_READER_COUNTERS?12u:9u)&&
        (r.lastMoveResult<=2||r.lastMoveResult==255)&&r.resourceFailure<=10&&
        cfgspawn::Finite(r.helperProjected)&&cfgspawn::Finite(r.helperRequested)&&cfgspawn::Finite(r.helperActual)&&
        r.requestedBots<=MaximumBots&&(r.selectedJasonIndex<64||r.selectedJasonIndex==0xFFFFFFFF)&&
        r.convergenceActive<=1&&r.convergenceKeys<=9&&r.convergenceSchemaMask<512&&r.convergenceSchemaBot<MaximumBots&&
        r.convergenceSchemaFailure<=9&&r.convergenceFailureKey<=9&&r.convergenceExpectedBool<=1&&r.convergenceActualBool<=255&&
        r.trapMapFamily<=3&&r.trapMapPackage.back()==0&&r.sweaterBound<=1&&r.sweaterStage<=5&&
        (r.sweaterFailure<=103||(r.sweaterFailure>=111&&r.sweaterFailure<=114))&&r.sweaterHudReturns<=r.sweaterHudAttempts&&
        r.sweaterPickupStage<=3&&r.sweaterPickupInstalled<=1&&r.sweaterPickupFailure<=11&&
        r.trapStage<=13&&r.trapFailure<=8&&r.trapObjectives<=4&&r.trapObjective<=5&&r.trapConsumed<=r.trapAttempts&&
        r.trapShackStage<=3&&r.trapShackFailure<=10&&r.trapShackSlices<=513&&r.trapShackReady<=1&&r.trapShackRestarts<=513&&r.trapCurrentKind<=3&&
        r.sweaterArmPending<=1&&r.sweaterArmFailure<=6&&r.sweaterArms<=r.sweaterArmAttempts&&
        r.defeatedOutroFlags<=2&&r.defeatedOutroFinished<=2&&r.carSpeedActive<=1&&r.knifeFlightActive<=1&&
        r.knifeCollisionBefore<=2&&r.knifeCollisionAfter<=2&&r.knifeAcquiredCollision<=2&&
        r.knifeReleaseCollisionBefore<=2&&r.knifeReleaseCollisionAfter<=2&&r.knifeStopStage<=10&&r.doorBreakIntent<=1&&
        r.knifeStopMaxReads<=ReadCap&&r.knifeNotifyMaxReads<=ReadCap&&r.swimFailure<=4&&r.swimGuard<=21&&
        r.radioObjectiveStage<=4&&r.radioObjectiveFailure<=8&&r.radioObjectiveSlices<=129&&r.radioObjectiveRetries<=32&&
        r.radioObjectiveAttempts<=1&&r.radioObjectiveConfirmed<=1&&r.radioObjectiveTypedStage<=8&&
        r.swimNativeAdmission>=0&&r.swimNativeAdmission<=1&&r.runStateFailure<=4&&r.runStateReturns<=r.runStateAttempts&&
        cfgspawn::Finite(r.knifeFlightPosition)&&std::isfinite(r.knifeFlightSpeed)&&r.knifeFlightSpeed>=0&&
        r.knifeLaunchObserved<=1&&cfgspawn::Finite(r.knifeNativeLaunchPoint)&&cfgspawn::Finite(r.knifeNativeLaunchVelocity)&&
        cfgspawn::Finite(r.knifeAimPoint)&&cfgspawn::Finite(r.knifeLaunchPoint)&&
        cfgspawn::Finite(r.knifeLaunchVelocity)&&cfgspawn::Finite(r.knifeActualLaunch)&&cfgspawn::Finite(r.knifeActualVelocity)&&
        r.loadoutRequested<=1&&r.loadoutReady<=1&&r.loadoutFailed<=1&&r.killGateFailedPhase<=6&&r.killGateFields<=4096&&r.killGateOwners<=32&&
        r.killInteractionAdmission<=9&&r.killInteractionReads<=ReadCap&&r.killInteractionCalls<=CallCap&&
        r.killRecoveryReturns<=r.killRecoveryAttempts&&r.killRecoveryFailure<=4&&
        r.hidingPhase<=13&&r.hidingSelectionStage<=9&&r.hidingPhaseStartStage<=13&&r.hidingSafeFailure<=6&&r.hidingRecoveries<=r.hidingTimeouts&&r.carSlamObserved<=1&&r.carRouteCommitted<=1&&r.carSeatBatch<=3&&
        r.spectatorStatusJasonView<=1&&r.spectatorStatusText.back()==0&&r.spectatorStatusNextMode.back()==0&&
        r.sweaterRetentionRestored<=r.sweaterRetentionChecks&&r.sweaterRetentionRefused<=r.sweaterRetentionChecks&&r.sweaterRetentionFailure<=9&&r.sweaterRetentionMaxReads<=ReadCap&&
        r.killDamageGate<=6&&r.killFailure<=9&&r.killResourceFailure<=15&&r.loadoutState<=6&&r.killStage<=6&&(CFG_J21C_READER_COUNTERS||r.killEligibilityInstalled<=1)&&r.durationState<=6&&r.skillState<=6&&r.phase<=static_cast<unsigned>(Lane::Loadout)&&r.maxReads<=ReportReadCap&&r.maxCalls<=CallCap;
}
}
