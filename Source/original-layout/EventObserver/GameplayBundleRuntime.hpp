#include "HostedH1Report.hpp"
#include "../Core/HostedKillerPresentation.hpp"
#include "../Core/HostedPresentationCopy.hpp"
#if defined(CFG_H1_JASON) && defined(CFG_H1_OBSERVE)
extern cfgh1::Report hostedH1Report;
#endif
#include "../Core/HostedMaskTriggerPolicy.hpp"
#include "../Core/HostedClassRestorePolicy.hpp"
#include "../RuntimeProbe/HostedCounselorBrainBindings.generated.hpp"
// Separate bundle only. Uses the proven primary spawn, never a player handoff.
#include "CounselorTerminalObservation.hpp"
#include "TommyEndRaceObservation.hpp"
#include "../Core/DonorReinforcementQueue.hpp"
#include "../Core/DonorMoveRetentionPolicy.hpp"
#include "../Core/DonorHidingTargetPolicy.hpp"
#include "../Core/HostedMaskTriggerMemo.hpp"
#include "../Core/HostedSweaterClassProofMemo.hpp"
#include "../Core/HostedSpectatorHintPolicy.hpp"
#include "../Core/AdditionalJasonPolicy.hpp"
#include "../Core/VehicleProgressPolicy.hpp"
#include "../Core/HeldRecoveryPolicy.hpp"
#include "../Core/ChaseRecoveryPolicy.hpp"
#include "../Core/JasonHuntPolicy.hpp"
#include "PoliceExitObservation.hpp"
#include "../Core/OnFootCarObstaclePolicy.hpp"
#include "../Core/HuntProbeAdmission.hpp"
#include "../Core/EndingSummaryPolicy.hpp"
#include "../Core/KillHitDiagnostic.hpp"
#include "../Core/PamelaContextAdmission.hpp"
#include "../RuntimeProbe/Compatibility/CfgSweaterPickup35.generated.hpp"
#include "../Core/VehicleContinuationPolicy.hpp"
#include "../RuntimeProbe/CounselorLoadBindings.generated.hpp"
#include "../Core/StuckOccupiedCarPolicy.hpp"
#include "../Core/AdditionalJasonJournal.hpp"
#include "AdditionalConstructionTrace.hpp"
#include "AdditionalRegistryWrite.hpp"
#include "../RuntimeProbe/AdditionalLifecycleBindings.generated.hpp"
#include "NativeSummaryBotRoster.hpp"
#include "OwnedNativeDetour.hpp"
#include "NativeJasonControllerTickHook.hpp"
#include "DonorJasonDecisionFrame.hpp"
#include "DonorJasonRuntimeState.hpp"
#include "../RuntimeProbe/NativePopulationBindings.generated.hpp"
#include "../RuntimeProbe/SummaryFilterBindings.generated.hpp"
#include "../Core/JasonRageDifficultyPolicy.hpp"
#include "NativeSummaryObservation.hpp"
#include "EndingSummaryOpportunity.hpp"
#include "../RuntimeProbe/AdditionalJasonBindings.generated.hpp"
#include "../SourceBaseline/gameplay-backend/port/src/Game/Setup/JasonDoorApproachPolicy.hpp"
#include "../Core/DeadSpectatorRecoveryPolicy.hpp"
#include "PostMatchRecoveryObservation.hpp"
#include "NativePursuitTargetObservation.hpp"
#include "../Core/H246BurstPolicy.hpp"
#include "../Core/H247CombatPolicy.hpp"
#include "../Core/JasonDeathCompletion.hpp"
#include "DefeatedOutroProperty.hpp"
#include "../Core/SweaterRetention.hpp"
#include "../Core/SweaterOwnerRefreshPolicy.hpp"
#include "../RuntimeProbe/SweaterRetentionBindings.generated.hpp"
#include "../RuntimeProbe/OfflineRecoveryBindings.generated.hpp"
#include "../Core/GrabCombatPolicy.hpp"
#include "../Core/JasonRunPolicy.hpp"
#include "../RuntimeProbe/RunningStateBindings.hpp"
#include "../RuntimeProbe/SummaryRowReadyBindings.hpp"
#include "../RuntimeProbe/SummaryPresentationBindings.hpp"
#include "../Core/SummaryPresentationPolicy.hpp"
#include "../Core/ConvergenceTravelPolicy.hpp"
#include "RetiredOwnedAbility.hpp"
#include "ConvergenceTravelParameters.hpp"
#include "ConvergenceBrainObservation.hpp"
#include "../RuntimeProbe/ConvergenceTravelBindings.generated.hpp"
#include "../RuntimeProbe/GrabBindings.generated.hpp"
#include "NativeEmoteMirror.hpp"
#include "DonorCounselorEmotes.hpp"
#include "../Core/DonorHeldGrabKill.hpp"
#include "../Core/DonorNativeTurn.hpp"
#include "../RuntimeProbe/ObjectPathObservation.hpp"
#include "../RuntimeProbe/DonorHeldBindings.generated.hpp"
#include "EmotePlaybackAccess.hpp"
#include "HelperCatalogueObservation.hpp"
#include "SoftClassFrameObservation.hpp"
#include "EmoteObservationAccess.hpp"
#include "RBEmoteRuntime.hpp"
void BundleRBConfirmationFailed()noexcept;
bool BundleRBCancelCurrent(std::uintptr_t pawn)noexcept;
#include "NativeSoundBlipSuppression.hpp"
#include "TrapPriorityObservation.hpp"
#include "../RuntimeProbe/SwimActionBindings.hpp"
#include "CounselorStartObservation.hpp"
#include "PendingCounselorBirth.hpp"
#include "../Core/RandomCounselorPolicy.hpp"
#include "../Core/ResidentCounselorSelection.hpp"
#include "../Core/OptionalWorkAdmission.hpp"
#include "ResolvedCounselorTemplate.hpp"
#include "../RuntimeProbe/NativeClassPathChoice.hpp"
#include "../RuntimeProbe/SpectatorRuntime.hpp"
#include "SpectatorCycleObservation.hpp"
#include "../Core/HostedWidgetDispatchPolicy.hpp"
#include "OfflineDiagnosticTrace.hpp"
#include "../RuntimeProbe/SpectatorNativeBindings.generated.hpp"
#include "../RuntimeProbe/HostLocalView35.generated.hpp"
#include "HostedLocalView.hpp"
#include "../RuntimeProbe/SpectatorViewBindings.hpp"
#include "RadioObjectiveSource.hpp"
void DeadSpectatorRecoveryTrace(std::uintptr_t before,std::uintptr_t after,bool returned){
    cfgclosedtrace::Mark(returned?"spectator-dead-target-restored":"spectator-dead-target-refused",before,after);
}
template<std::size_t N>struct KnifeHookLease {
    std::uintptr_t target=0;void* original=nullptr;std::array<unsigned char,N> installed{};bool enabled=false;
    template<class R>bool Current(R& r)const{
        std::array<unsigned char,N> code{};return enabled&&original&&r.Read(target,code)&&code==installed;
    }
};
KnifeHookLease<cfgknife::disk::ThrowKnifeCode.size()> bundleKnifeNotifyHook;
KnifeHookLease<cfgknife::disk::MulticastUseCode.size()> bundleKnifeMulticastHook;
KnifeHookLease<cfgknife::disk::LaunchCode.size()> bundleKnifeLaunchHook;
KnifeHookLease<cfgknife::disk::OnStopCode.size()> bundleKnifeStopLease;
thread_local cfgknife::NotifyWindow* bundleKnifeNotifyWindow=nullptr;
thread_local bool bundleKnifeNotifyBusy=false;
bool bundleKnifeNotifyEnabled=false;
bool bundleKnifeBoundaryObserveEnabled=false;
bool bundleKnifeReleaseAdapterEnabled=false;
thread_local cfgknife::ReleaseFrame* bundleKnifeReleaseFrame=nullptr;
cfgknife::ReleaseDiagnostics bundleKnifeReleaseDiagnostics;
unsigned bundleKnifeReleaseDepth=0;
unsigned bundleKnifeNotifyEntries=0,bundleKnifeNotifyCorrections=0,bundleKnifeNotifyRefused=0,bundleKnifeNotifyStops=0;
unsigned bundleKnifeNotifyAdmissionRefused=0,bundleKnifeNotifyMaxReads=0;
void __fastcall BundleNativeThrowKnife(void*);
void __fastcall BundleNativeKnifeMulticast(void*,void*,const cfgspawn::Vec3*,const cfgspawn::Vec3*);
void __fastcall BundleNativeKnifeLaunch(void*,const cfgspawn::Vec3*);
template<std::size_t N>bool PrepareKnifeWitness(std::uintptr_t module,std::uintptr_t rva,
    const std::array<unsigned char,N>& original,void* detour,KnifeHookLease<N>& lease){
    if(lease.target)return lease.enabled&&lease.target==module+rva;
    Reader r;r.cap=16;if(!cfgbundle::Code(r,module,rva,original))return false;
    lease.target=module+rva;
    if(MH_CreateHook(reinterpret_cast<void*>(lease.target),detour,&lease.original)!=MH_OK)return false;
    if(MH_EnableHook(reinterpret_cast<void*>(lease.target))!=MH_OK)return false;
    // Preserve and subsequently prove our exact resident detour image. Never
    // ignore a code mismatch just because an original body has been hooked.
    if(!r.Read(lease.target,lease.installed))return false;
    lease.enabled=true;return true;
}
using KnifeStopOriginal=void(__fastcall*)(void*,const void*);
KnifeStopOriginal bundleKnifeStopOriginal=nullptr;
std::uintptr_t bundleKnifeStopTarget=0;bool bundleKnifeStopEnabled=false;
unsigned bundleKnifeHeldStops=0,bundleKnifeHeldRefused=0,bundleKnifeStopMaxReads=0;
void __fastcall BundleNativeKnifeStop(void*,const void*);
bool PrepareBundleKnifeStop(std::uintptr_t module){
    if(bundleKnifeStopTarget)return bundleKnifeStopEnabled&&bundleKnifeStopTarget==module+cfgknife::disk::OnStop;
    Reader r;r.cap=128;
    if(!cfgbundle::Code(r,module,cfgknife::disk::OnStop,cfgknife::disk::OnStopCode))return false;
    void* original=nullptr;const auto target=module+cfgknife::disk::OnStop;
    if(MH_CreateHook(reinterpret_cast<void*>(target),reinterpret_cast<void*>(&BundleNativeKnifeStop),&original)!=MH_OK)return false;
    bundleKnifeStopOriginal=reinterpret_cast<KnifeStopOriginal>(original);bundleKnifeStopTarget=target;MemoryBarrier();
    if(MH_EnableHook(reinterpret_cast<void*>(target))==MH_OK){
        bundleKnifeStopEnabled=true;bundleKnifeStopLease.target=target;bundleKnifeStopLease.original=original;
        if(!r.Read(target,bundleKnifeStopLease.installed))return false;
        bundleKnifeStopLease.enabled=true;return true;
    }
    MH_DisableHook(reinterpret_cast<void*>(target));return false;
}
bool PrepareBundleKnifeNotify(std::uintptr_t module){
    using namespace cfgknife::disk;
    if(!PrepareBundleKnifeStop(module)||
       !PrepareKnifeWitness(module,MulticastUse,MulticastUseCode,reinterpret_cast<void*>(&BundleNativeKnifeMulticast),bundleKnifeMulticastHook)||
       !PrepareKnifeWitness(module,Launch,LaunchCode,reinterpret_cast<void*>(&BundleNativeKnifeLaunch),bundleKnifeLaunchHook)||
       !PrepareKnifeWitness(module,ThrowKnife,ThrowKnifeCode,reinterpret_cast<void*>(&BundleNativeThrowKnife),bundleKnifeNotifyHook))return false;
    MemoryBarrier();bundleKnifeNotifyEnabled=true;return true;
}
bool PrepareBundleKnifeBoundaryObservation(std::uintptr_t module){
    // Existing entry hooks, pass-through only. Installation grants no native
    // correction/suppression authority to the animation callback.
    if(!PrepareBundleKnifeStop(module)||!PrepareKnifeWitness(module,cfgknife::disk::ThrowKnife,cfgknife::disk::ThrowKnifeCode,
        reinterpret_cast<void*>(&BundleNativeThrowKnife),bundleKnifeNotifyHook))return false;
    MemoryBarrier();bundleKnifeBoundaryObserveEnabled=true;return true;
}
bool PrepareBundleKnifeReleaseAdapter(std::uintptr_t module){
    if(!PrepareBundleKnifeNotify(module))return false;
    MemoryBarrier();bundleKnifeBoundaryObserveEnabled=true;bundleKnifeReleaseAdapterEnabled=true;return true;
}
cfgspectator::Runtime bundleSpectator;
std::uint64_t hostedEndWatchAt=0;
std::uint64_t bundleSpectatorAt=0;
std::uint64_t bundleSpectatorLabelAt=0;
unsigned bundleSpectatorLabelAttempts=0;
bool bundleSpectatorLabelPending=false;
bool bundleJasonLabelOwned=false;
cfgspectatorrecovery::State bundleDeadSpectatorRecovery;
std::uint64_t bundleObjectiveStackAt=0;
cfgmenu::active::StackArray bundleObjectiveStackLast{};
std::uintptr_t bundleObjectiveStackTop=0;
unsigned bundleObjectiveStackVisits=0;
cfgobjective::CacheAdmission bundleObjectiveCarCache;
cfgobjective::PublicationWindow bundleObjectiveCarPublication;
cfgvehicle::RosterProof bundleObjectiveCarProof;
cfgradioobjective::Survey bundleRadioObjective;
std::uint64_t bundleRadioObjectiveSince=0,bundleRadioObjectiveNext=0;
bool bundleRadioObjectiveBegun=false,bundleRadioObjectiveAttempted=false,bundleRadioObjectiveFinished=false;
bool bundleSpectatorLast=false;
cfgspectator::Identity bundleSpectatorInstalled;
cfgspawn::PlayerArray bundleSpectatorList{};
std::array<cfgspectator::Target,cfgspectator::MaximumCounselors> bundleSpectatorRoster{};
std::array<std::uintptr_t,cfgspectator::MaximumCounselors> bundleSpectatorPawnClasses{},bundleSpectatorStateClasses{};
unsigned bundleSpectatorCursor=0;
bool bundleSpectatorRosterReady=false;
std::uint64_t bundleSpectatorCycleEvents=0;
using SpectatorViewOriginal=void(__fastcall*)(void*,std::int32_t);
SpectatorViewOriginal bundleSpectatorViewOriginal=nullptr;
std::uintptr_t bundleSpectatorViewTarget=0;
bool bundleSpectatorViewEnabled=false,bundleSpectatorViewBusy=false;
cfgspectator::CycleHookRetry bundleSpectatorCycleHook;
unsigned bundleSpectatorNativeCycles=0;
volatile LONG bundleSpectatorNativeEntries=0;
unsigned bundleSpectatorCycleRejection=0;
std::int32_t bundleSpectatorCycleDirection=0;
std::uintptr_t bundleSpectatorCycleController=0;
void __fastcall BundleNativeViewAPlayer(void*,std::int32_t);
bool PrepareBundleSpectatorCycle(std::uintptr_t module){
    using namespace cfgofflinerecovery::disk;
    const auto target=module+ViewAPlayer;
    if(bundleSpectatorViewTarget){
        // CreateHook succeeded previously, but EnableHook may have transiently
        // failed. Reuse only the exact process-owned target/original pair and
        // retry enabling it; never create a second hook or change camera code.
        if(bundleSpectatorViewTarget!=target||!bundleSpectatorViewOriginal)return false;
        if(bundleSpectatorViewEnabled)return true;
        if(MH_EnableHook(reinterpret_cast<void*>(target))==MH_OK){bundleSpectatorViewEnabled=true;return true;}
        return false;
    }
    Reader r;r.cap=64;
    if(!cfgbundle::Code(r,module,ViewAPlayer,ViewAPlayerCode))return false;
    void* original=nullptr;
    if(MH_CreateHook(reinterpret_cast<void*>(target),reinterpret_cast<void*>(&BundleNativeViewAPlayer),&original)!=MH_OK)return false;
    bundleSpectatorViewOriginal=reinterpret_cast<SpectatorViewOriginal>(original);bundleSpectatorViewTarget=target;MemoryBarrier();
    if(MH_EnableHook(reinterpret_cast<void*>(target))==MH_OK){bundleSpectatorViewEnabled=true;return true;}
    MH_DisableHook(reinterpret_cast<void*>(target));return false;
}
std::array<std::uintptr_t,32> bundleSpectatorOtherFunctions{};
unsigned bundleSpectatorOtherCursor=0;
unsigned bundleSpectatorAlternateTraceCount=0; // process-wide bounded diagnostics, never camera authority
// PID15068: one refused Context() (likely the door phase's spent read budget
// or a GC/serialization veto) permanently stopped Jason. Consecutive transient
// refusals now skip the phase; nothing acts without the next full re-proof.
#ifdef CFG_H1_JASON
inline bool HostedRoutineActive()noexcept;
#else
inline constexpr bool HostedRoutineActive()noexcept;
#endif
constexpr unsigned BundleContextDeferralLimit=64;
unsigned bundleContextDeferrals=0,bundleContextDeferralsTotal=0,bundleContextTraces=0;
constexpr unsigned BundleManagerRetryLimit=64;
constexpr std::uint64_t BundleManagerRetryDelayMs=250;
constexpr std::uint64_t BundleManagerRetryCooldownMs=5000;
struct BundleManagerRetryState {std::uintptr_t world=0;unsigned failures=0;std::uint64_t retryAt=0;bool exhausted=false;};
BundleManagerRetryState bundleManagerRetry;
struct BundleManagerFirstFailure {std::uintptr_t world=0;unsigned stage=0,reads=0,cap=0;bool pending=false,emitted=false;};
BundleManagerFirstFailure bundleManagerFirstFailure;
unsigned bundleManagerLastClause=0,bundleManagerLastReads=0,bundleManagerLastCap=0;
struct BundleKnifeFirstAbortTrace {std::uintptr_t world=0;unsigned stage=0,failure=0,reads=0,calls=0;bool pending=false,emitted=false;};
BundleKnifeFirstAbortTrace bundleKnifeFirstAbortTrace;
void RecordBundleManagerFailure(unsigned stage,unsigned reads,unsigned cap){
    if(!HostedRoutineActive())return;
    bundleManagerLastClause=stage;bundleManagerLastReads=reads;bundleManagerLastCap=cap;
    if(bundleManagerFirstFailure.world!=plan.world)bundleManagerFirstFailure={plan.world,0,0,0,false,false};
    if(!bundleManagerFirstFailure.pending&&!bundleManagerFirstFailure.emitted){
        bundleManagerFirstFailure.stage=stage;bundleManagerFirstFailure.reads=reads;bundleManagerFirstFailure.cap=cap;bundleManagerFirstFailure.pending=true;}
}
void FlushBundleFirstFailureTrace(){
    bool rootsIdle=true;
#ifdef CFG_RETAINED_GAMEPLAY_BUNDLE
    rootsIdle=!bundleRootLease&&!bundleRootWitness.lease;
#endif
    if(bundleManagerFirstFailure.pending&&!bundleManagerFirstFailure.emitted&&
       rootsIdle&&!NativeJasonControllerTickInFlight()&&
       GetCurrentThreadId()==plan.threadId){
        cfgclosedtrace::Mark("bundle-manager-refusal-stage",bundleManagerFirstFailure.stage,
            (static_cast<std::uint64_t>(bundleManagerFirstFailure.reads)<<32)|bundleManagerFirstFailure.cap);
        bundleManagerFirstFailure.pending=false;bundleManagerFirstFailure.emitted=true;
    }
    if(bundleKnifeFirstAbortTrace.pending&&!bundleKnifeFirstAbortTrace.emitted&&rootsIdle&&
       !NativeJasonControllerTickInFlight()&&GetCurrentThreadId()==plan.threadId){
        cfgclosedtrace::Mark("bundle-knife-first-abort",bundleKnifeFirstAbortTrace.stage,bundleKnifeFirstAbortTrace.failure);
        cfgclosedtrace::Mark("bundle-knife-first-abort-work",bundleKnifeFirstAbortTrace.reads,bundleKnifeFirstAbortTrace.calls);
        bundleKnifeFirstAbortTrace.pending=false;bundleKnifeFirstAbortTrace.emitted=true;
    }
}
template<class Observe>bool BundleManagerActionAllowed(std::uint64_t now,bool hosted,Observe&& observe){
    if(!hosted)return observe();
    if(bundleManagerRetry.world!=plan.world)bundleManagerRetry={plan.world,0,0,false};
    if(now<bundleManagerRetry.retryAt)return false;
    if(bundleManagerRetry.exhausted){bundleManagerRetry.failures=0;bundleManagerRetry.exhausted=false;}
    if(observe()){bundleManagerRetry={plan.world,0,0,false};return true;}
    if(bundleManagerRetry.failures<BundleManagerRetryLimit)++bundleManagerRetry.failures;
    const auto delay=bundleManagerRetry.failures>=BundleManagerRetryLimit?BundleManagerRetryCooldownMs:BundleManagerRetryDelayMs;
    bundleManagerRetry.retryAt=now>UINT64_MAX-delay?UINT64_MAX:now+delay;
    if(bundleManagerRetry.failures>=BundleManagerRetryLimit)bundleManagerRetry.exhausted=true;
    return false;
}
enum class BundleManagerGateResult:unsigned{Ready,Deferred,Failed};
template<class Observe>BundleManagerGateResult BundleManagerGate(std::uint64_t now,bool hosted,Observe&& observe){
    if(BundleManagerActionAllowed(now,hosted,observe))return BundleManagerGateResult::Ready;
    return hosted?BundleManagerGateResult::Deferred:BundleManagerGateResult::Failed;
}
void ResetBundleManagerRetry(){bundleManagerRetry={};bundleManagerFirstFailure={};}
cfghiding::State bundleHiding;
unsigned bundleHidingRefused=0;
std::uintptr_t bundleSpectatorStatusFunction=0;
std::uintptr_t bundleSpectatorStatusWidgetHint=0;
cfgnames::Words bundleSpectatorStatusName{};
bool bundleSpectatorStatusNameReady=false;
std::uint64_t bundleSpectatorStatusAt=0;
#if CFG_J21_EMOTE_MIRROR
cfgemotemirror::DonorRuntime bundleEmotes;
#else
cfgemotemirror::Runtime bundleEmotes; // exact accepted J1a.2 queue, NOT the new C implementation
#endif
cfgrbemote::Runtime bundleRBEmote;
bool bundleRBLastTurn=false;
bool bundleJasonLabelLastTurn=false;
bool bundleRBDispatch=false;
cfginput::SpectatorIntent bundleSpectatorInput;bool bundleSpectatorInputPending=false;std::uint64_t bundleSpectatorInputPendingAt=0;
cfgsoundblip::Runtime bundleSoundBlips;
cfgsoundblip::Scope bundleSoundScope;
cfgtrappriority::Policy bundleTrapPriority;
cfgtrappriority::Slot bundleTrapVictimSlot;
cfgtrappriority::Slot bundleTriggeredTrapSlot;
std::uint64_t bundleSwimNext=0;
struct {std::uint64_t until=0,next=0,blockedUntil=0;unsigned failures=0,marks=0,stalls=0;float lastDistance=-1.f;bool forceSearch=false;} bundleHiderApproach; // H2.66
using TrapSetterOriginal=int(__fastcall*)(void*,void*,cfgspawn::Vec3*);
TrapSetterOriginal bundleTrapSetterOriginal=nullptr;
std::uintptr_t bundleTrapHookTarget=0;
bool bundleTrapHookEnabled=false;
unsigned bundleTrapEvents=0,bundleTrapAccepted=0,bundleTrapRefused=0,bundleTrapMaxReads=0;
void BundleTrapEventAfter(std::uintptr_t victim,std::uintptr_t trap);
int __fastcall BundleNativeTrappedState(void* victim,void* trap,cfgspawn::Vec3* output){
    if(CFG_J214_TRAP_PRIORITY)
        return cfgj214trap::Forward(victim,trap,output,bundleTrapSetterOriginal,
            GetCurrentThreadId()==plan.threadId,[](void* v,void* t){
                BundleTrapEventAfter(reinterpret_cast<std::uintptr_t>(v),reinterpret_cast<std::uintptr_t>(t));});
    // Exactly one original call, preserving native foot selection/return value.
    const int result=bundleTrapSetterOriginal?bundleTrapSetterOriginal(victim,trap,output):-1;
    if(result>=0&&GetCurrentThreadId()==plan.threadId)
        BundleTrapEventAfter(reinterpret_cast<std::uintptr_t>(victim),reinterpret_cast<std::uintptr_t>(trap));
    return result;
}
bool PrepareBundleTrapHook(std::uintptr_t module){
    if(!cfgtrappriority::disk::NativeHookBindingVerified)return false;
    if(bundleTrapHookTarget)return bundleTrapHookEnabled&&bundleTrapHookTarget==module+cfgtrappriority::disk::Setter;
    Reader r;r.cap=64;
    if(!cfgbundle::Code(r,module,cfgtrappriority::disk::Setter,cfgtrappriority::disk::SetterCode))return false;
    void* original=nullptr;const auto target=module+cfgtrappriority::disk::Setter;
    if(MH_CreateHook(reinterpret_cast<void*>(target),reinterpret_cast<void*>(&BundleNativeTrappedState),&original)!=MH_OK)return false;
    bundleTrapSetterOriginal=reinterpret_cast<TrapSetterOriginal>(original);bundleTrapHookTarget=target;MemoryBarrier();
    if(MH_EnableHook(reinterpret_cast<void*>(target))==MH_OK){bundleTrapHookEnabled=true;return true;}
    MH_DisableHook(reinterpret_cast<void*>(target));return false;
}
using SoundBlipOriginal=void(__fastcall*)(void*,float);
SoundBlipOriginal bundleSoundOriginal=nullptr;
std::uintptr_t bundleSoundHookTarget=0;
bool bundleSoundHookEnabled=false;
unsigned bundleSoundSuppressed=0;
#if CFG_H276_SOUND_EMITTER
// H2.77 diagnostics: install result (1 pin, 2 create, 3 enable, 4 installed, 5 already), last MH status, detour entries, entries that reached the hosted
// test, highest hosted gate reached (1 thread, 2 routine, 3 identity, 4 lookup, 5 world, 6 slot, 7 suppress), last killer seen.
unsigned bundleSoundInstall=0,bundleSoundEntries=0,bundleSoundHostedCalls=0,bundleSoundGate=0;int bundleSoundMh=0;std::uintptr_t bundleSoundLastKiller=0;
#endif
bool BundleAdditionalSoundSuppressed(std::uintptr_t killer);
#if CFG_H276_SOUND_EMITTER
bool BundleHostedSoundSuppressed(std::uintptr_t killer);
#endif
void __fastcall BundleNativeSoundBlips(void* killer,float delta){
    // Donor suppresses the emitter, not merely existing blips. Fast reject
    // unrelated worlds/roles; bounded direct identity reads, never a census.
#if CFG_H276_SOUND_EMITTER
    ++bundleSoundEntries;bundleSoundLastKiller=reinterpret_cast<std::uintptr_t>(killer);
#endif
    if(GetCurrentThreadId()==plan.threadId&&bundleSoundScope.active&&
       reinterpret_cast<std::uintptr_t>(killer)==bundleSoundScope.killer.object){
        Reader r;r.cap=128;
        if(cfgprimary::LookupSafe(r,plan.module)&&cfgsoundblip::Suppress(r,bundleSoundScope,
            reinterpret_cast<std::uintptr_t>(killer),true)){++bundleSoundSuppressed;return;}
    }
#if CFG_H276_SOUND_EMITTER
    if(BundleAdditionalSoundSuppressed(reinterpret_cast<std::uintptr_t>(killer))||BundleHostedSoundSuppressed(reinterpret_cast<std::uintptr_t>(killer))){++bundleSoundSuppressed;return;}
#else
    if(BundleAdditionalSoundSuppressed(reinterpret_cast<std::uintptr_t>(killer)))return;
#endif
    if(bundleSoundOriginal)bundleSoundOriginal(killer,delta);
}
bool PrepareBundleSoundHook(std::uintptr_t module){
#if CFG_H276_SOUND_EMITTER
    if(bundleSoundHookTarget){bundleSoundInstall=5;return bundleSoundHookEnabled&&bundleSoundHookTarget==module+cfgsoundblip::disk::UpdateSoundBlips;}
#else
    if(bundleSoundHookTarget)return bundleSoundHookEnabled&&bundleSoundHookTarget==module+cfgsoundblip::disk::UpdateSoundBlips;
#endif
    Reader r;r.cap=64;
    if(!cfgbundle::Code(r,module,cfgsoundblip::disk::UpdateSoundBlips,cfgsoundblip::disk::UpdateSoundBlipsCode)){
#if CFG_H276_SOUND_EMITTER
        bundleSoundInstall=1;
#endif
        return false;}
    void* original=nullptr;const auto target=module+cfgsoundblip::disk::UpdateSoundBlips;
#if CFG_H276_SOUND_EMITTER
    const auto created=MH_CreateHook(reinterpret_cast<void*>(target),reinterpret_cast<void*>(&BundleNativeSoundBlips),&original);bundleSoundMh=static_cast<int>(created);
    if(created!=MH_OK){bundleSoundInstall=2;return false;}
#else
    if(MH_CreateHook(reinterpret_cast<void*>(target),reinterpret_cast<void*>(&BundleNativeSoundBlips),&original)!=MH_OK)return false;
#endif
    bundleSoundOriginal=reinterpret_cast<SoundBlipOriginal>(original);bundleSoundHookTarget=target;MemoryBarrier();
#if CFG_H276_SOUND_EMITTER
    const auto enabled=MH_EnableHook(reinterpret_cast<void*>(target));bundleSoundMh=static_cast<int>(enabled);
    if(enabled==MH_OK){bundleSoundHookEnabled=true;bundleSoundInstall=4;return true;}
    bundleSoundInstall=3;
#else
    if(MH_EnableHook(reinterpret_cast<void*>(target))==MH_OK){bundleSoundHookEnabled=true;return true;}
#endif
    // Resident original remains valid even if enable is ambiguous. No gameplay
    // scope is published on preparation failure; forwarding remains unchanged.
    MH_DisableHook(reinterpret_cast<void*>(target));return false;
}
cfgescape::Completion bundleCompletion;
cfgescape::PostMatchWatch bundlePostMatchWatch;
cfgoutro::Recovery bundlePostMatchRecovery;
bool bundleCompletionReady=false;
cfgdeathend::Policy bundleDeathEnd;
std::array<cfgdeathend::OutroProperty,2> bundleDefeatedOutro{};
unsigned bundleDefeatedOutroCursor=0,bundleDefeatedOutroFlags=0;
cfgprobe::lifetime::SlotObservation bundleHumanTarget;
constexpr unsigned BundleNativeTargetIndex=cfgbundle::MaximumBots+1;
cfgnativepursuit::Proof bundleNativeTarget{};
std::uintptr_t bundleNativeTargetHint=0;
unsigned bundleNativeTargetHintIndex=0;
cfggrab::Policy bundleGrab;
cfgholdrecovery::Policy bundleHeldRecovery;
unsigned bundleGrabWarm=0;
bool bundleGrabUnavailable=false;
cfgbundle::Report bundleReport;
// Per-world history survives menu return. Missing prior instrumentation is not
// evidence of zero uses; these counters begin with this report version only.
unsigned EnsureBundleSweaterMatch(std::uint64_t now){
    auto& count=bundleReport.sweaterMatchCount;
    if(!count||bundleReport.sweaterMatches[count-1].world!=plan.world){
        if(count==bundleReport.sweaterMatches.size()){
            for(unsigned i=1;i<count;++i)bundleReport.sweaterMatches[i-1]=bundleReport.sweaterMatches[i];
            --count;
        }
        bundleReport.sweaterMatches[count++]={plan.world,now,0,0,0,0};
    }
    return count-1;
}
void RecordBundleSweaterEvent(bool use,std::uint64_t now){
    auto& match=bundleReport.sweaterMatches[EnsureBundleSweaterMatch(now)];
    if(use){++match.uses;match.lastUseAt=now;}else{++match.recharges;match.lastRechargeAt=now;}
    cfgclosedtrace::Mark(use?"sweater-use-observed":"sweater-recharge-observed",plan.world,now);
}
#ifdef CFG_RETAINED_GAMEPLAY_BUNDLE
void PublishBundleRootState(unsigned failure,bool candidate,bool entered)noexcept{
    // Own diagnostics only. Preserve an existing outer mutation generation.
    const bool own=(bundleReport.generation&1u)==0;
    if(own)InterlockedIncrement(reinterpret_cast<volatile LONG*>(&bundleReport.generation));
    if(candidate)++bundleReport.rootCandidates;if(entered)++bundleReport.rootEntered;
    if(failure){++bundleReport.rootRefusals;bundleReport.rootLastFailure=failure;}
    bundleReport.rootOwners=bundleRootBank.Owners();bundleReport.rootClaims=bundleRootLease?bundleRootLease->Claims():0;
    bundleReport.rootPoisoned=bundleRootBank.Poisoned()?1u:0u;
    bundleReport.rootWindowStage=bundleRootDiagnostics.windowStage;bundleReport.rootWindowGate=bundleRootDiagnostics.windowGate;
    bundleReport.rootWindowAttempts=bundleRootDiagnostics.attempts;bundleReport.rootLockRetries=bundleRootDiagnostics.retries;
    bundleReport.rootWindowRefusals=bundleRootDiagnostics.refusals;
    bundleReport.rootReferencerFailure=bundleRootDiagnostics.referencerFailure;
    bundleReport.rootStrandedClaims=bundleRootDiagnostics.strandedClaims;
    bundleReport.rootEmptyRetirements=bundleRootDiagnostics.emptyRetirements;
    bundleReport.rootDeferredClaims=bundleDeferredRetirement.Claims();
    bundleReport.rootDeferredRetirements=bundleRootDiagnostics.deferredRetirements;
    bundleReport.rootDrainedRetirements=bundleRootDiagnostics.drainedRetirements;
    bundleReport.zeroOutputCompletions=bundleRootDiagnostics.zeroOutputCompletions;
    bundleReport.zeroOutputContendedCompletions=bundleRootDiagnostics.zeroOutputContendedCompletions;
    bundleReport.zeroOutputProbeUnknown=bundleRootDiagnostics.zeroOutputProbeUnknown;
    if(own){MemoryBarrier();InterlockedIncrement(reinterpret_cast<volatile LONG*>(&bundleReport.generation));}
}
#endif
cfgbundle::Schedule bundleSchedule;
cfgbundle::HelperStartup bundleHelpers;
cfghelperbirth::PendingCounselorBirth bundleHelperBirth;
cfgresidentcounselor::Pass bundleHelperResidentPass;
cfgcounselorstart::Observation bundleCounselorStarts;
cfgkillerstart::Bootstrap bundleCounselorStartWarm;
cfgbundle::RandomCounselors bundleRandomCounselors;
cfgcounselorload::Pending bundleCounselorLoad;
cfgcounselorload::Preload bundleCounselorPreload;
cfgspawn::PlayerArray bundleCounselorCatalogue{};
std::array<cfgprimary::SoftClassBytes,64> bundleCounselorEntries{};
cfgspawn::TemplateObservation bundleHelperTemplate;
cfgprimary::SoftClassBytes bundleHelperPacket{};
unsigned bundleHelperEntry=0,bundleHelperStart=0,bundleHelperLocationAttempts=0,bundleHelperStaleRetries=0;
bool bundleHelperSelected=false;
bool bundleStarted=false;
#ifdef CFG_H1_JASON
bool hostedExtraPresentationPending=false;
bool hostedJasonExtraCountConsumed=false;
void HostedF1CaptureRequest(bool accepted)noexcept;
void HostedH1ServiceAdditionalRequests(std::uint64_t now)noexcept;
void HostedH1ServiceAdditionalPresentation()noexcept;
void HostedH1ServicePrimaryPresentation(std::uint64_t now)noexcept; // H2.43: post-adoption killer presentation copy
#endif
#ifdef CFG_H1_JASON
inline bool HostedRoutineActive()noexcept;
#else
inline constexpr bool HostedRoutineActive()noexcept;
#endif
inline bool BundleGameplayStarted()noexcept{return bundleStarted||HostedRoutineActive();}
#ifdef CFG_H1_JASON
inline bool HostedSummaryWindow()noexcept;
#else
inline constexpr bool HostedSummaryWindow()noexcept;
#endif
// Summary presentation only: also true in the hosted end-frozen window (never gameplay scheduling).
inline bool BundleSummaryStarted()noexcept{return BundleGameplayStarted()||HostedSummaryWindow();}
bool bundleF3Down=false;
cfgcutoff::Tracker bundleCutoff; // H2.84 counselor heading samples for the cut-off Morph (positions the donor lane already proved)
std::uint64_t bundleF3Polls=0,bundleF3Foreground=0,bundleF3DownSamples=0,bundleF3ShortSamples=0;
struct AdditionalJasonState {
    cfgadditional::Policy policy;cfgjason::NewActorEvidence actor;
    cfgspawn::TemplateObservation selected{},sameClassFallback{};cfgadditionaljournal::Journal journal;
    unsigned sameClassFallbackIndex=~0u; // H2.42: used only when no distinct Jason class exists
    bool hostedOwnClass=false; // H2.51: hosted F1 selected the adopted Jason's own class (no captured catalogue)
    bool hostedRandom=false,hostedGateWait=false;unsigned hostedGateRetries=0,hostedRandomTries=0,hostedRandomSeed=0,hostedOwnerMarks=0,hostedTargetMarks=0,burstMarks=0;std::uintptr_t hostedLastTarget=0;cfgfootburst::Policy burst;cfgprobe::lifetime::SlotObservation burstMovement{};std::uint64_t burstCheckAt=0;std::array<unsigned char,40> hostedRandomSoft{}; // H2.61 hosted random Jason class (resident only)
    unsigned selection=0,selectedIndex=~0u,selectionRefusal=0,selectionConversionFailures=0,selectionTemplateFailures=0;
    cfgrun::Policy run;cfgadditional::Targets targets;
    cfgbundle::HelperStartup placement;cfgspawn::Vec3 position{};
    cfgbundle::ActorFollowPath path;cfgbundle::MoveCooldown move;cfggrab::Policy grab;
    std::uintptr_t rawPawn=0,rawController=0,rawPath=0,primaryState=0,world=0;bool killerClass=false;float burstSquared=-1.f; // H2.83 offline F1 PlayerState carries a Jason ActiveCharacterClass
    cfgsoundblip::Scope soundScope;cfgsoundblip::Runtime sound;
    cfgragethreshold::Rage rage;
    unsigned cursor=0,driveRefusals=0,rollback=0;bool allocationAttempted=false,stopped=false,lastTurn=false,genericDestroyAttempted=false;
} bundleAdditional;
using AdditionalPostSpawnOriginal=void(__fastcall*)(void*,const void*,void*,void*,unsigned char,unsigned char,unsigned char);
AdditionalPostSpawnOriginal bundleAdditionalPostSpawnOriginal=nullptr;
std::uintptr_t bundleAdditionalPostSpawnTarget=0;
bool bundleAdditionalPostSpawnEnabled=false;
using AdditionalSingleOriginal=void(__fastcall*)(void*);
AdditionalSingleOriginal bundleAdditionalBeginOriginal=nullptr,bundleAdditionalIteratorOriginal=nullptr;
std::uintptr_t bundleAdditionalBeginTarget=0,bundleAdditionalIteratorTarget=0;
bool bundleAdditionalBeginEnabled=false,bundleAdditionalIteratorEnabled=false;
thread_local cfgadditionalconstruction::Scope bundleAdditionalConstructionScope;
bool PrepareAdditionalConstructionTrace(std::uintptr_t module);
bool BundleAdditionalSoundSuppressed(std::uintptr_t killer){
    const auto& scope=bundleAdditional.soundScope;
    if(GetCurrentThreadId()!=plan.threadId||!bundleSchedule.active||!scope.active||killer!=scope.killer.object)return false;
    Reader r;r.cap=128;
    // H2.86: the game calls UpdateSoundBlips outside any mod scoped-lookup bundle, so cfgprimary::LookupSafe is always false
    // here (live: every hosted entry stopped at Gate=3). Suppression is read-only (skip the original, no native call by us):
    // use the read-only GC/serialization veto, as ChallengeLookupSafe does.
    return (CFG_H286_BLIP_VETO?cfgprimary::LookupVetoClearForObservation(r,plan.module):cfgprimary::LookupSafe(r,plan.module))&&
        cfgsoundblip::Suppress(r,scope,killer,true);
}
cfgreinforcement::Counselors bundleF3Queue;
unsigned bundleHelperCatalogueFailures=0;
cfginput::CounselorInputConsumer bundleF3WindowInput;
bool bundleHelperReinforcement=false;
bool bundleLateMaskRetryUsed=false;
std::uint64_t bundleInputAt=0;
cfgjason::NewActorEvidence bundleJason;
#if CFG_H276_SOUND_EMITTER
// Donor IsCounselorRouteSoundBlipEmitter: while the AI Jason route is active in the current world its emitter makes no blips.
// Hosted only: owner thread, pointer compare to the adopted primary / F1 Jason, one bounded object-slot proof. A human Jason never matches.
bool BundleHostedSoundSuppressed(std::uintptr_t killer){
    ++bundleSoundHostedCalls;bundleSoundGate=(std::max)(bundleSoundGate,1u);
    if(!killer||GetCurrentThreadId()!=plan.threadId||!HostedRoutineActive())return false;
    bundleSoundGate=(std::max)(bundleSoundGate,2u);
    const cfgprobe::lifetime::SlotObservation* expected=nullptr;
    if(killer==bundleJason.pawn.object)expected=&bundleJason.pawn;
    else if(killer==bundleAdditional.actor.pawn.object)expected=&bundleAdditional.actor.pawn; // H2.78: live 14641 entries all refused at identity with the F1 pawn as killer; the slot proof below still binds the object
    if(!expected)return false;
    bundleSoundGate=(std::max)(bundleSoundGate,3u);
    Reader r;r.cap=64;std::uintptr_t world=0;cfgprobe::lifetime::ArrayHeader array;cfgprobe::lifetime::SlotObservation slot;
    if(!(CFG_H286_BLIP_VETO?cfgprimary::LookupVetoClearForObservation(r,plan.module):cfgprimary::LookupSafe(r,plan.module)))return false; // H2.86 read-only veto (see above)
    bundleSoundGate=(std::max)(bundleSoundGate,4u);
    if(!(plan.world&&r.Read(plan.viewport+cfgprobe::ViewportWorld,world)&&world==plan.world))return false;
    bundleSoundGate=(std::max)(bundleSoundGate,5u);
    if(!(cfgprobe::lifetime::ReadArrayHeader(r,plan.module+cfgprobe::lifetime::ObjectArrayRva,array)&&
        cfgspawn::ObjectSlot(r,array,killer,slot)&&slot==*expected))return false;
    bundleSoundGate=(std::max)(bundleSoundGate,6u);return true;
}
#endif
std::array<cfgprobe::lifetime::SlotObservation,cfgbundle::MaximumBots> bundlePawnSlots{},bundleControllerSlots{};
cfgprobe::lifetime::SlotObservation bundleTreeSlot{},bundleManagerClassSlot{};
std::uintptr_t bundleManagerClass=0,bundleCounselorClass=0;
unsigned bundleSelectIndex=0,bundleTargetIndex=0;
cfgbundle::TargetChoice bundleChoice;
std::uint64_t bundleMoveAt=0;
std::uint64_t bundleTargetLockUntil=0;
cfgbundle::MoveCooldown bundlePathCooldown;
cfgbundle::ActorFollowPath bundleFollowPath;
cfgpursuit::Progress bundlePursuitProgress;
cfgchaserecovery::Policy bundleChaseRecovery;
// Intercept/loop counter-play: fed only by the hunt lane's existing distance
// observation; moves reuse the verified MoveToLocation binding (door approach).
cfghunt::TargetTracker bundleHuntTracker;cfghunt::ChaseProgress bundleHuntProgress;
cfgpoliceexit::Runtime bundleEscapeGuard;
cfgworlddiag::Previous bundlePreviousWorld;
#if defined(CFG_RETAINED_GAMEPLAY_BUNDLE) && defined(CFG_FRONTEND_BUNDLE)
void ObservePreviousWorldAtMenu(const cfgmenu::active::Context& menu)noexcept;
#endif
std::uint64_t bundleInterceptAt=0;
cfghunt::ChaseFatigue bundleChaseFatigue;unsigned bundleFatigueTraces=0,bundleEscalationTraces=0;
unsigned bundleIntercepts=0,bundleInterceptRefusals=0,bundleInterceptUnavailable=0,bundleInterceptTraces=0;
std::array<unsigned,9> bundleConvergenceRefusals{};unsigned bundleConvergenceTraces=0;
cfgsummary::Runtime bundleSummary;
cfgsummarybots::Roster bundleSummaryBots;
cfgsummarybots::Roster bundleStatusBots;
cfgsummary::Runtime bundleStatusRows;
cfgstatussurface::Runtime bundleStatusSurface;
unsigned bundleSummarySurfaceTurn=0;
bool bundleSummaryLastTurn=false;
using SummaryFilterOriginal=bool(__fastcall*)(void*,void*);
SummaryFilterOriginal bundleSummaryFilterOriginal=nullptr;
std::uintptr_t bundleSummaryFilterTarget=0;
bool bundleSummaryFilterEnabled=false;
using SummaryRowReadyOriginal=bool(__fastcall*)(void*);
SummaryRowReadyOriginal bundleSummaryRowReadyOriginal=nullptr;
std::uintptr_t bundleSummaryRowReadyTarget=0;
bool bundleSummaryRowReadyEnabled=false;
using SummaryTextOriginal=void*(__fastcall*)(void*,void*);
using SummaryPanelOriginal=unsigned char(__fastcall*)(void*);
using SummaryStateReadyOriginal=bool(__fastcall*)(void*,bool);
SummaryTextOriginal bundleSummaryTextOriginal=nullptr;
SummaryPanelOriginal bundleSummaryPanelOriginal=nullptr,bundleSummaryCharacterOriginal=nullptr;
SummaryStateReadyOriginal bundleSummaryStateReadyOriginal=nullptr;
thread_local std::uintptr_t bundleSummaryPresentationRow=0;
bool bundleSummaryPresentationEnabled=false;
bool PrepareBundleSummaryPresentation(std::uintptr_t);
bool PrepareBundleSummaryRowReady(std::uintptr_t);
bool __fastcall BundleNativeSummaryFilter(void*,void*);
bool PrepareBundleSummaryFilter(std::uintptr_t);
cfgragethreshold::Rage bundleRageDifficulty;
cfgdoor::Recovery bundleDoorRecovery;
cfgdoor::Selected bundleDoorBreakSelection;
std::uint64_t bundleTrapDoorAt=0;
cfgvehicle::Cooldown bundleCarMorph;
cfgvehicle::TailIntercept bundleCarIntercept;
cfgvehicle::Pursuit bundleCarPursuit;
cfgvehicleprogress::Runtime bundleVehicleProgress;
cfgroad::Runtime bundleRoads;
cfgroad::PathTrace bundleRoadTrace;
struct RoadMorphPending {unsigned stage=0;std::uintptr_t car=0,driver=0,source=0;cfgspawn::Vec3 position{},velocity{},projected{};cfgroad::Candidate candidate{};bool prolongedTail=false,exitPriority=false;};
RoadMorphPending bundleRoadMorph;
std::uint64_t bundleRoadAttemptAt=0;
cfgvehicleaction::State bundleVehicleActions;
std::uint32_t bundleCarRefuseLine=0; // H2.55c: last CarInteraction refusal line (diagnostics only)
cfgdrivernav::Cache bundleDriverNavigation;
cfgvehicleaction::Obstacle bundleCarObstacle;
cfgfootcar::Obstacle bundleFootObstacle;
cfgfootcar::RouteHistory bundleFootObstacleHistory;
cfgstuckcar::Runtime bundleStuckCars;
cfgprobe::lifetime::SlotObservation bundleFootObstacleCarSlot{};
std::uintptr_t bundleFootObstacleTarget=0;
std::uint64_t bundleFootObstacleCheckAt=0;
std::uintptr_t bundleCounselorMorphTarget=0;
std::uint64_t bundleCounselorMorphCheckAt=0;
cfghuntprobe::Policy bundleHuntProbe;
cfgsummaryending::Window bundleEndingSummary;
bool bundleSummaryStackBound=false;
bool bundleFootObstacleAlternate=false;
struct CarSpeedState {cfgprobe::lifetime::SlotObservation slot{};std::array<float,4> original{},boosted{};bool active=false;};
CarSpeedState bundleCarSpeed;
cfgrun::Policy bundleRun;
struct CarActionPending {bool active=false,moving=false;std::uintptr_t source=0,car=0,driver=0;
    cfgprobe::lifetime::SlotObservation carSlot{},driverSlot{};cfgspawn::Vec3 position{},velocity{};bool speedChange=false;};
CarActionPending bundleCarActionPending;
std::uint64_t bundleCarSpeedRetryAt=0;
cfgknife::Runtime bundleKnife;
void LatchBundleKnifeFirstAbort(){
    if(!bundleKnife.firstAbortRecorded)return;
    if(bundleKnifeFirstAbortTrace.world!=plan.world)bundleKnifeFirstAbortTrace={plan.world,0,0,0,0,false,false};
    if(!bundleKnifeFirstAbortTrace.pending&&!bundleKnifeFirstAbortTrace.emitted){
        bundleKnifeFirstAbortTrace.stage=static_cast<unsigned>(bundleKnife.firstAbortStage);
        bundleKnifeFirstAbortTrace.failure=bundleKnife.firstAbortFailure;
        bundleKnifeFirstAbortTrace.reads=bundleKnife.firstAbortReads;
        bundleKnifeFirstAbortTrace.calls=bundleKnife.firstAbortCalls;bundleKnifeFirstAbortTrace.pending=true;}
}
cfgknife::NearestRefresh bundleKnifeRefresh;
cfgending::Transaction bundleEnding;
cfgending::skin::Transfer bundleEndingSkin;
cfgfootburst::Policy bundleFootBurst;
cfgprobe::lifetime::SlotObservation bundleFootMovement;
std::uint64_t bundleFootBurstRequestAt=0;float bundleFootBurstSquared=-1.f; // H2.85 target distance at the burst request
cfgbotname::Queue bundleBotNames;
bool bundleKnifeProbePending=false;
unsigned bundleKnifeProbeIndex=0;
std::uint64_t bundleKnifeProbeAt=0;
cfgpursuit::Ring bundlePursuit;
cfgcabin::Runtime bundleCabins;
std::uint64_t bundleCabinsNext=0;
constexpr cfgvehicle::disk::Binding BundleTeleport{
    cfgteleport::disk::FunctionFactory,cfgteleport::disk::FunctionCache,cfgteleport::disk::ActorClass,
    cfgteleport::disk::ActorConstructor,cfgteleport::disk::ParameterBytes,cfgteleport::disk::StorageBytes,cfgteleport::disk::FactoryCode};
unsigned bundleCarIndex=0;
cfgconvergence::Policy bundleConvergence;
cfgconvergence::AxeRuntime bundleConvergenceAxes;
unsigned bundleConvergenceFearPending=0,bundleConvergenceFearWrites=0,bundleConvergenceFearRefused=0;
cfgobjective::Runtime bundleObjectives;
bool bundleObjectiveRetiring=false;
std::uint64_t bundleObjectiveNext=0,bundleObjectiveCheck=0;
std::uintptr_t bundleObjectiveModule=0,bundleObjectiveWorld=0,bundleObjectiveViewport=0;
DWORD bundleObjectiveThread=0;
cfgobjective::repair::Runtime bundleObjectiveRepair;
cfgobjectivepanel::Binding bundleObjectivePanelBinding;
cfgprobe::lifetime::SlotObservation bundleObjectiveWidget;
unsigned bundleObjectiveCandidate=0,bundleObjectiveRepairFailure=0;
bool bundleObjectiveRepairTurn=false;
struct ConvergenceTravelState {cfgconvergencetravel::Policy policy;cfgprobe::lifetime::SlotObservation brain{},controller{};};
std::array<ConvergenceTravelState,cfgbundle::MaximumBots> bundleConvergenceTravel{};
struct EmoteBrainHold {cfgemotemirror::Slot pawn{},controller{},brain{};unsigned phase=0;bool changed=false;
    std::uintptr_t montage=0;std::uint64_t until=0;};
std::array<EmoteBrainHold,cfgbundle::MaximumBots> bundleEmoteBrains{};
std::uint64_t bundleEmoteMaintainAt=0;unsigned bundleEmoteMaintainCursor=0,bundleEmoteStops=0,bundleEmoteResumes=0;
bool BundleEmoteHeld(unsigned index){return index<bundleEmoteBrains.size()&&bundleEmoteBrains[index].phase!=0;}
bool BundleEmoteMaintenanceDue(std::uint64_t now){
    if(now<bundleEmoteMaintainAt)return false;
    for(const auto& hold:bundleEmoteBrains)if(hold.phase&&(hold.phase>=4||now>=hold.until||
        !bundleEmotes.InterruptPending({hold.pawn,hold.controller})))return true;
    return false;
}
unsigned bundleTravelCursor=0,bundleTravelAcquires=0,bundleTravelMoves=0,bundleTravelResumes=0,bundleTravelLost=0;
std::uint64_t bundleTravelAt=0;
bool bundleTravelTurn=false;
cfgconvergence::FeatureTurns bundleFeatureTurns;
std::array<cfgconvergence::Name,cfgconvergence::KeyCount> bundleConvergenceNames{};
constexpr std::array<cfgconvergence::disk::Binding,6> ConvergenceFunctions{{
    cfgconvergence::disk::Conv_StringToName,cfgconvergence::disk::GetValueAsObject,
    cfgconvergence::disk::GetValueAsBool,cfgconvergence::disk::SetValueAsObject,
    cfgconvergence::disk::SetValueAsBool,cfgconvergence::disk::GetCurrentWeapon}};
struct ConvergenceResource {
    std::uintptr_t function=0,owner=0;
    cfgprobe::lifetime::SlotObservation functionSlot{},ownerSlot{};
};
std::array<ConvergenceResource,6> bundleConvergenceResources{};
unsigned bundleConvergencePrepared=0;
cfgconvergence::Publication bundleConvergencePublication;
cfgconvergence::PublicationContinuation bundleConvergenceContinuation;
cfgconvergence::PublicationContinuation bundleAxeContinuation;
std::array<cfgconvergence::BlackboardSchema,cfgbundle::MaximumBots> bundleBlackboardSchemas;
std::array<cfgkillerstart::Bootstrap,4> bundleBlackboardClassWarm;
unsigned bundleBlackboardClassIndex=0;
bool bundleBlackboardClassesRefused=false;
cfgprobe::lifetime::SlotObservation bundleConvergenceBlackboard{};
cfgsweater::Policy bundleSweaterPolicy;
// Separate ability owner; the immutable startup Plan remains unchanged.
cfgprobe::lifetime::SlotObservation bundleSweaterOwner;
using SweaterCleanupOriginal=void(__fastcall*)(void*);
SweaterCleanupOriginal bundleSweaterCleanupOriginal=nullptr;
std::uintptr_t bundleSweaterCleanupTarget=0;
bool bundleSweaterCleanupEnabled=false;
void __fastcall BundleNativeSweaterCleanup(void* counselor);
bool PrepareBundleSweaterCleanup(std::uintptr_t module){
    if(bundleSweaterCleanupTarget)return bundleSweaterCleanupEnabled&&bundleSweaterCleanupTarget==module+cfgsweater::retaindisk::Cleanup;
    Reader r;r.cap=64;
    if(!cfgbundle::Code(r,module,cfgsweater::retaindisk::Cleanup,cfgsweater::retaindisk::CleanupCode))return false;
    void* original=nullptr;const auto target=module+cfgsweater::retaindisk::Cleanup;
    if(MH_CreateHook(reinterpret_cast<void*>(target),reinterpret_cast<void*>(&BundleNativeSweaterCleanup),&original)!=MH_OK)return false;
    bundleSweaterCleanupOriginal=reinterpret_cast<SweaterCleanupOriginal>(original);bundleSweaterCleanupTarget=target;MemoryBarrier();
    if(MH_EnableHook(reinterpret_cast<void*>(target))==MH_OK){bundleSweaterCleanupEnabled=true;return true;}
    MH_DisableHook(reinterpret_cast<void*>(target));return false;
}
cfgsweater::ClassLease bundleSweaterClass;
bool bundleSweaterClassAttempted=false,bundleSweaterClassPending=false,bundleSweaterFreshPending=false,bundleSweaterFreshRequested=false;
std::uint64_t bundleSweaterFreshUse=0;
bool bundleSweaterFreshGrantAttempted=false;
cfgsweater::PickupState bundleSweaterPickup;
cfgprobe::lifetime::SlotObservation bundleSweaterAbility{},bundleSweaterSource{},bundleSweaterRetiredAbility{};
unsigned bundleSweaterCursor=0,bundleSweaterSettingsCursor=0,bundleSweaterSourceIndex=~0u,bundleSweaterWarm=0;
cfgofflinesettings::Runtime bundleDuration;
cfgofflinesettings::SkillRuntime bundleSkill;
cfgloadout::restricted::Runtime bundleLoadout;
cfgloadout::expanded::Runtime bundleExpandedLoadout;
cfgloadoutresource::Pending bundleLoadoutResource;
cfgtrap::startup::Runtime bundleTraps;
cfgtrap::startup::Sources bundleTrapSources;
cfgtrap::startup::Inputs bundleTrapInputs;
cfgtrap::MapFamily bundleTrapFamily=cfgtrap::MapFamily::Unknown;
std::uintptr_t bundleTrapMapWorld=0;
cfgnames::Text bundleTrapMapPackage;
cfgkill::Runtime bundleKill;
cfgkilldiag::Budget bundleKillDiagnostics;
cfgkill::DamageGate bundleDamageGate;
std::uintptr_t bundleKillStanceFunction=0;
cfgprobe::lifetime::SlotObservation bundleKillStanceSlot;
cfgkill::EligibilityOriginal bundleKillOriginal=nullptr;
std::uintptr_t bundleKillHookComponent=0,bundleKillClone=0;
cfgprobe::lifetime::SlotObservation bundleKillActorSlot{},bundleKillComponentSlot{},bundleKillInteractSlot{};
std::uint64_t bundleKillAt=0;
cfgkillcontact::Tracker bundleMeleeNativeTracker;
cfgkill::EligibilityTicket bundleKillTicket;
cfgprobe::lifetime::SlotObservation bundleKillWeaponClass;
cfgkill::Weapon bundleKillWeaponKind=cfgkill::Weapon::Other;
class BundleAccess;
#include "HostedJasonState.hpp"
BundleAccess* bundleKillEligibilityAccess=nullptr; // synchronous native predicate only
int __fastcall BundleKillEligibility(void*,void*,const cfgkill::EligibilityVector*,const cfgkill::EligibilityVector*);
int __fastcall BundleSweaterCanInteract(void*,void*,const cfgkill::EligibilityVector*,const cfgkill::EligibilityVector*);

bool BundleInitializeCheck(cfgbundle::InitializationStep step,bool okay)noexcept{
    bundleReport.initializationStage=static_cast<unsigned>(step);return okay;
}
#include "J21Timing.hpp"
#include "DonorConvergenceNativeState.hpp"
#include "DonorJasonRosterRuntime.hpp"
#include "LeanConvergenceNativeState.hpp"
#include "CounselorPracticeRuntimeState.hpp"
#include "H226HostedRuntimePolicy.hpp"
#include "../Core/HostedContextMemo.hpp"
class BundleAccess {
    Reader r_;unsigned calls_=0,startingCalls_=0;bool exception_=false;
    unsigned hostedPresentationCopyRefusal_=0,hostedPresentationCopyBundleNativeStage_=0;
    bool hostedKnifeAimSibling_=false;
#ifdef CFG_H1_JASON
    cfghostedcontext::CallbackMemo hostedContextProof_{};
    unsigned hostedContextClause_=0; // H2.55c diagnostics: last hosted context proof step reached (1 lookup,2 binding,3 world,4 adoption,5 game state,6 killer pawn,7 killer state)
#endif
public:
#ifdef CFG35_URINATION_NATIVE_PORT
#include "UrinationLoaderInvokeMembers.hpp"
#endif
    cfgobserver::Plan context=plan;
    explicit BundleAccess(unsigned readCap=cfgbundle::ReadCap,unsigned callDebt=0):calls_((std::min)(callDebt,cfgbundle::CallCap)),startingCalls_(calls_){r_.cap=(std::min)(readCap,cfgbundle::ReadCap);
#if CFG_H250_ARRAY_GROWTH && defined(CFG_H1_JASON)
        // H2.50b: the plan's GUObjectArray header (count) is captured once per hosted plan. Objects
        // born later (the adopted AI controller, Tommy's pawn) can have indices beyond that count,
        // so ReadSlot refused them for the whole match (live: adopted controller idx 177891,
        // ReadOk=0 while it still possessed Jason -> clause 7 on every check). Same growth-only
        // rule as ConvergenceInvoke: same storage, count may only grow.
        if(HostedRoutineActive()){cfgprobe::lifetime::ArrayHeader registry;
            if(cfgprobe::lifetime::ReadArrayHeader(r_,plan.module+cfgprobe::lifetime::ObjectArrayRva,registry)&&
               registry.data==context.objectArray.data&&registry.count>context.objectArray.count)context.objectArray=registry;}
#endif
    }
    unsigned Reads()const{return r_.reads;}unsigned Calls()const{return calls_-startingCalls_;}
    unsigned HostedPresentationCopyRefusal()const noexcept{return hostedPresentationCopyRefusal_;}
    unsigned HostedPresentationCopyBundleNativeStage()const noexcept{return hostedPresentationCopyBundleNativeStage_;}
    // Accounting only: a fresh sibling reader inherits this callback's debt.
    // Absorbing its delta cannot create native/GC/retention permission.
    void AccountChildCalls(unsigned count)noexcept{calls_+=(std::min)(count,cfgbundle::CallCap-calls_);}
    class ChildAccounting {
        BundleAccess& parent_;BundleAccess& child_;
    public:
        ChildAccounting(BundleAccess& parent,BundleAccess& child)noexcept:parent_(parent),child_(child){}
        ChildAccounting(const ChildAccounting&)=delete;ChildAccounting& operator=(const ChildAccounting&)=delete;
        ~ChildAccounting(){
            parent_.AccountChildCalls(child_.Calls());
            bundleReport.reads+=child_.Reads();
            bundleReport.maxReads=(std::max)(bundleReport.maxReads,child_.Reads());
            bundleReport.maxCalls=(std::max)(bundleReport.maxCalls,parent_.Calls());
        }
    };
    DWORD Error()const{return r_.error;}bool Exception()const{return exception_;}
    std::uintptr_t TrapJason()const{return bundleJason.pawn.object;}
    std::uintptr_t TrapController()const{return bundleJason.killerController.object;}
    bool ObjectiveScope(){
#ifdef CFG_H1_JASON
        if(HostedRoutineActive())return HostedContext(false);
#endif
        if(!BundleGameplayStarted()||!bundleSchedule.active||GetCurrentThreadId()!=plan.threadId||
           plan.module!=reinterpret_cast<std::uintptr_t>(GetModuleHandleW(nullptr)))return false;
        cfgprobe::lifetime::ArrayHeader registry;cfgprobe::lifetime::SlotObservation world;
        std::uintptr_t stateClass=0;
        return cfgobserver::Same(r_,plan.viewport+cfgprobe::ViewportWorld,plan.world)&&
            cfgobserver::Same(r_,plan.world+cfgprobe::WorldNetDriver,std::uintptr_t{})&&
            cfgobserver::Same(r_,plan.world+cfgprobe::WorldGameMode,plan.mode)&&
            cfgobserver::Same(r_,plan.world+cfgprobe::WorldGameState,spawnAssets.gameState)&&
            cfgprobe::lifetime::ReadArrayHeader(r_,plan.module+cfgprobe::lifetime::ObjectArrayRva,registry)&&
            cfgprobe::lifetime::ReadSlot(r_,registry,plan.world,world)&&world==plan.objects[0]&&
            r_.Read(plan.module+cfgofflinesettings::disk::SCGameStateClass,stateClass)&&cfgprobe::ObjectPointer(stateClass)&&
            cfgobserver::Same(r_,plan.module+cfgofflinesettings::disk::SCGameStateConstructor,stateClass)&&
            cfgspawn::ObjectIs(r_,spawnAssets.gameState,stateClass);
    }
    bool ObjectiveCarCaches(std::uint64_t now){
        if(bundleObjectiveCarPublication.pending){
            auto& publication=bundleObjectiveCarPublication;const auto bit=1u<<publication.index;
            if(!publication.Due(now))return false;
            if(publication.Expired(now)){bundleReport.objectiveCacheRefusedMask|=bit;publication.Finish();return true;}
            struct CommitHost {BundleAccess& a;cfgobjective::PublicationWindow& publication;unsigned bit;
                bool Current(){return a.ObjectiveScope()&&cfgprimary::LookupSafe(a.r_,plan.module);}
                bool Exchange(std::uintptr_t at,std::uintptr_t expected,std::uintptr_t value){
                    if(!publication.MarkAttempt())return false;
                    bundleReport.objectiveCachePublicationAttempts|=bit;
                    return !expected&&cfgobjective::PublishEmptyPointer(at,value);}} host{*this,publication,bit};
            cfgprobe::lifetime::ArrayHeader registry;
            // PublishRosterCache repeats authority immediately before CAS and
            // after it. Do not spend another full authority pass beforehand;
            // reading the header alone neither adopts nor mutates any object.
            const bool okay=cfgprobe::lifetime::ReadArrayHeader(r_,plan.module+cfgprobe::lifetime::ObjectArrayRva,registry)&&
                cfgvehicle::PublishRosterCache(r_,registry,bundleObjectiveCarProof,host);
            if(!okay&&!publication.attempted&&r_.reads>=r_.cap&&publication.Defer(now)){
                ++bundleReport.objectiveCacheAcquisitionDeferrals;return true;
            }
            if(okay)bundleReport.objectiveCacheMask|=bit;else bundleReport.objectiveCacheRefusedMask|=bit;
            publication.Finish();bundleObjectiveCarProof={};return true;
        }
        const auto index=bundleObjectiveCarCache.index;
        if(index>=2)return false;
        bundleObjectiveCarCache.Begin(now);
        struct Host {BundleAccess& a;bool Current(){return a.ObjectiveScope()&&cfgprimary::LookupSafe(a.r_,plan.module);}
            bool Exchange(std::uintptr_t at,std::uintptr_t expected,std::uintptr_t value){return cfgobjective::CompareWrite(at,expected,value);}} host{*this};
        cfgprobe::lifetime::ArrayHeader registry;std::uintptr_t car=0;cfgvehicle::RosterProof proof{};
        const auto source=index?cfgvehicle::disk::Car2:cfgvehicle::disk::Car4;
        const bool acquired=host.Current()&&cfgprobe::lifetime::ReadArrayHeader(r_,plan.module+cfgprobe::lifetime::ObjectArrayRva,registry)&&
            cfgvehicle::RosterCar(r_,registry,plan.module,plan.world,plan.mode,spawnAssets.gameState,source,car,&proof);
        if(r_.reads>=r_.cap){bundleObjectiveCarCache.BudgetDeferred(now);
            if(bundleObjectiveCarCache.index!=index)bundleReport.objectiveCacheRefusedMask|=1u<<index;
            return true;}
        ++bundleReport.objectiveCacheChecks;
        const bool publishable=acquired&&car;
        // Finish this acquisition slot, then revalidate/publish in a fresh
        // callback. PublicationWindow consumes the actual CAS before writing;
        // never replay an ambiguous native cache write.
        const bool finished=bundleObjectiveCarCache.Acquired(now,publishable);
        if(publishable){
            bundleObjectiveCarProof=proof;bundleObjectiveCarPublication.Begin(index,now);
        }else if(finished){
            if(acquired)bundleReport.objectiveCacheAbsentMask|=1u<<index;
            else bundleReport.objectiveCacheRefusedMask|=1u<<index;
        }else ++bundleReport.objectiveCacheAcquisitionDeferrals;
        return true; // at most one native cache per fresh slice
    }
#include "RadioObjectiveRuntimeMembers.hpp"
    bool ObjectivePhase(std::uint64_t now){
#ifdef CFG_H1_JASON
        HostedLaneEnter(cfgh1::Lane::Objectives);
#endif

        J21Timing timing(cfgj21::Objectives);
        constexpr unsigned required=0x0/*CFG3.5 address withheld*/;
        const auto& status=bundleObjectives.Status();
        const bool radio=cfgoptionalwork::RadioDue(bundleRadioObjectiveFinished,bundleHelpers.Active(),bundleRadioObjectiveNext,now);
        const bool repair=bundleObjectiveCarCache.index>=2&&!bundleObjectiveCarPublication.pending&&bundleRadioObjectiveFinished&&!bundleObjectiveRepair.Finished()&&bundleObjectives.InstanceCount()&&
            ((status.changedMask|status.baseMask)&required)==required;
        const bool patch=bundleObjectives.Pending(now);
        const bool admission=!bundleObjectiveWidget.object&&!bundleObjectiveRepair.report.attempts&&
            bundleObjectives.RootClass()&&bundleObjectiveStackVisits<128&&now>=bundleObjectiveStackAt;
        const bool cache=bundleObjectiveCarPublication.pending?bundleObjectiveCarPublication.Due(now):bundleObjectiveCarCache.Due(now);
        if(bundleObjectiveRetiring||(!patch&&!repair&&!admission&&!cache&&!radio)||now<bundleObjectiveNext||r_.reads||calls_)return
#ifdef CFG_H1_JASON
 HostedLaneRefuse(cfgh1::Lane::Objectives,1)
#else
 false
#endif
;
        bundleObjectiveNext=now+25;
        if(cache)return ObjectiveCarCaches(now);
        if(radio)return ObjectiveRadioPhase(now);
        if(admission){bundleObjectiveStackAt=now+500;return ObjectiveStackAdmission(now);}
        if(!bundleObjectiveModule){bundleObjectiveModule=plan.module;bundleObjectiveWorld=plan.world;
            bundleObjectiveViewport=plan.viewport;bundleObjectiveThread=plan.threadId;}
        if(repair&&(!patch||bundleObjectiveRepairTurn)){
            bundleObjectiveRepairTurn=false;
            if(bundleObjectiveRepair.report.stage==cfgobjective::repair::Stage::Dormant)bundleObjectiveRepair.Configure(cfgobjective::repair::AllMask);
            return ObjectiveRepairPhase(now);
        }
        bundleObjectiveRepairTurn=true;
        struct Host{BundleAccess& a;bool Current(){return a.ObjectiveScope();}
            bool Exchange(std::uintptr_t at,std::uintptr_t expected,std::uintptr_t value){return cfgobjective::CompareWrite(at,expected,value);}}host{*this};
        bundleObjectives.Step(r_,plan.module,host,now);return true;
    }
#include "ObjectiveHudRepairRuntimeMembers.hpp"
    bool TrapMorphDue(std::uint64_t now){return now>=bundleCarMorph.next;}
    bool TrapOpeningInventory(int expected,int desired){
        if(expected<0||desired<=expected||desired>4||bundleTrapSources.discovered||
           bundleTraps.report.stage!=cfgtrap::startup::Stage::Discover||
           !Context()||!Jason()||!cfgprimary::LookupSafe(r_,plan.module)||
           !cfgbundle::Code(r_,plan.module,cfgtrap::disk::AttemptPlaceTrapExecutor,cfgtrap::disk::AttemptPlaceTrapCode)||
           !cfgescape::Same(r_,bundleJason.pawn.object+cfgtrap::disk::TrapCount,expected)||
           GetCurrentThreadId()!=plan.threadId)return false;
        __try{return InterlockedCompareExchange(reinterpret_cast<volatile LONG*>(bundleJason.pawn.object+cfgtrap::disk::TrapCount),desired,expected)==expected;}
        __except(EXCEPTION_EXECUTE_HANDLER){exception_=true;return false;}
    }
    void TrapMorphCompleted(std::uint64_t now){bundleCarMorph.Attempted(now);bundleFollowPath.Clear();bundlePathCooldown.Clear();bundlePursuit.Clear();}
    bool TrapMotionAllowed(bool& allowed){
        allowed=false;std::uintptr_t held=0;bool stunned=false,attacking=false;
        if(!Held(held)||!cfgbundle::Code(r_,plan.module,cfghunter::disk::Stunned,cfghunter::disk::StunnedBytes)||
            !cfgbundle::Code(r_,plan.module,cfghunter::disk::Attacking,cfghunter::disk::AttackingBytes)||
            !NativeBool(cfghunter::disk::Stunned,stunned)||!NativeBool(cfghunter::disk::Attacking,attacking))return false;
        if(held)++bundleReport.heldSkips;
        if(stunned)++bundleReport.stunSkips;
        if(attacking)++bundleReport.attackAnimations;
        allowed=!held&&!stunned&&!attacking;return true;
    }
    void TrapPhase(std::uint64_t now){
#ifdef CFG_H1_JASON
        HostedLaneEnter(cfgh1::Lane::Traps);
#endif

        const auto wasBusy=bundleTraps.Busy();
        cfgtrap::startup::NativeAccess<Reader,BundleAccess> access(r_,*this,bundleTrapInputs,bundleTrapSources);
        bundleTraps.Step(access,now);
        const auto& traps=bundleTraps.report;
        bundleReport.trapStage=static_cast<unsigned>(traps.stage);bundleReport.trapFailure=static_cast<unsigned>(traps.failure);
        bundleReport.trapObjectives=traps.objectives;bundleReport.trapObjective=traps.objective;
        bundleReport.trapAttempts=traps.attempts;bundleReport.trapConsumed=traps.consumed;
        bundleReport.trapSkipped=traps.skipped;bundleReport.trapProbes=traps.probes;
        bundleReport.trapShackStage=bundleTrapSources.shack.stage;bundleReport.trapShackFailure=bundleTrapSources.shack.failure;
        bundleReport.trapShackSlices=bundleTrapSources.shack.slices;bundleReport.trapShackReady=bundleTrapSources.shackReady;
        bundleReport.trapPrepared=bundleTrapSources.prepared;
        bundleReport.trapShackRestarts=bundleTrapSources.shack.restarts;bundleReport.trapCurrentKind=traps.currentKind;
        bundleReport.trapOutcomes=traps.outcome;bundleReport.trapLastImpact=traps.lastImpact;
        bundleReport.trapLastStages=traps.lastStage;bundleReport.trapObjectiveProbes=traps.objectiveProbes;
        bundleReport.trapLastPawns=traps.lastPawn;bundleReport.trapLastStands=traps.lastStand;
        bundleReport.trapTravelFailures=traps.travelFailures;bundleReport.trapLastMoves=traps.lastMove;bundleReport.trapLastMorphs=traps.lastMorph;
        bundleReport.trapKinds=traps.kinds;bundleReport.trapNativePlacement=traps.nativePlacement;bundleReport.trapLegalPlacement=traps.legalPlacement;
        bundleReport.trapStops=traps.stops;bundleReport.trapImpacts=traps.impacts;
        for(unsigned i=0;i<4;++i){const auto& m=bundleTrapSources.morph[i];
            bundleReport.trapMorphProof[i]=(m.projected?1u:0u)|(m.tried?2u:0u)|(m.nativeAccepted?4u:0u)|(m.arrivalVerified?8u:0u)|(m.directFallback?16u:0u);
            bundleReport.trapProjectionCandidates[i]=m.projectionCandidate;
            bundleReport.trapMorphDestination[i]=m.destination;bundleReport.trapMorphActual[i]=m.actual;}
        if(wasBusy&&!bundleTraps.Busy()){bundleFollowPath.Clear();bundlePathCooldown.Clear();bundlePursuit.Clear();bundleMoveAt=now+700;}
    }
    bool Current(const cfgofflinesettings::Request& request){
        return request.Valid()&&request.submission==bundleConfiguration.submission&&
            request.world==plan.world&&request.pawn==plan.pawn&&
            request.worldSerial==plan.objects[0].serial&&request.pawnSerial==plan.objects[3].serial&&Context(true)&&CurrentHumanAlive();
    }
    bool CurrentHumanAlive(){bool living=false;return Alive(plan.pawn,living)&&living;}
    bool InvokeGrant(std::uintptr_t entry,std::uintptr_t pawn,std::uintptr_t type)noexcept{
        if(!NativeCallAllowed()||pawn!=plan.pawn||
            entry!=plan.module+cfgmenu::loadoutdisk::GiveStartingItem)return false;
#ifdef CFG_RETAINED_GAMEPLAY_BUNDLE
        if(!RetainBundleActors(pawn)||!RetainPrimaryObjects(r_,std::array<std::uintptr_t,1>{type})||!NativeCallAllowed()||!BeginBundleNative())return false;
#endif
        ++calls_;++bundleReport.loadoutCalls;
        __try{reinterpret_cast<void(__fastcall*)(void*,void*)>(entry)(reinterpret_cast<void*>(pawn),reinterpret_cast<void*>(type));}
        __except(EXCEPTION_EXECUTE_HANDLER){exception_=true;
#ifdef CFG_RETAINED_GAMEPLAY_BUNDLE
            BundleRetentionFailed();
#endif
            return false;}
#ifdef CFG_RETAINED_GAMEPLAY_BUNDLE
        return FinishBundleNative();
#else
        return true;
#endif
    }
    bool ResolveLoadedClass(const cfgofflinesettings::Request& owner,std::uintptr_t field,const cfgprimary::SoftClassBytes& source,std::uintptr_t& result){
        result=0;cfgprimary::ConversionParameters packet{};cfgprimary::SoftClassBytes fresh{};
        if(!Current(owner)||!r_.Read(field,fresh)||fresh!=source||!cfgprimary::BuildConversion(source,packet))return false;
        cfgprobe::lifetime::SlotObservation resource{};
        if(!cfgobserver::Same(r_,plan.module+cfgprimary::disk::Conversion.cache,spawnAssets.conversionFunction)||
           !cfgspawn::ObjectSlot(r_,context.objectArray,spawnAssets.conversionFunction,resource)||!cfgbundle::OwnedClassSlotMatches(spawnAssets.conversionSlot,resource)||
           !cfgspawn::ObjectSlot(r_,context.objectArray,spawnAssets.kismetClass,resource)||!cfgbundle::OwnedClassSlotMatches(spawnAssets.kismetClassSlot,resource)||
           !cfgspawn::ObjectSlot(r_,context.objectArray,spawnAssets.kismetDefault,resource)||!cfgbundle::OwnedClassSlotMatches(spawnAssets.kismetDefaultSlot,resource)||
           !cfgobserver::Same(r_,spawnAssets.kismetClass+cfgspawn::ClassDefaultObject,spawnAssets.kismetDefault)||
           !cfgobserver::Same(r_,spawnAssets.kismetDefault+cfgprobe::ObjectClass,spawnAssets.kismetClass)||
           !cfgobserver::Same(r_,spawnAssets.conversionFunction+0x20,spawnAssets.kismetClass)||
           !cfgbundle::Code(r_,plan.module,cfgprimary::disk::Conversion.factory,cfgprimary::disk::Conversion.prefix)||
           !Invoke(spawnAssets.kismetDefault,spawnAssets.conversionFunction,&packet,false)||!Current(owner)||!r_.Read(field,fresh)||fresh!=source)return false;
        result=packet.result;return true; // null is a valid cold result, not permission to grant
    }
    void LoadoutPhase(std::uint64_t now){
        if(bundleConfiguration.startingWeapon||bundleConfiguration.startingInventory){
            bundleExpandedLoadout.Step(r_,*this,plan.module,now);
            bundleReport.loadoutState=static_cast<unsigned>(bundleExpandedLoadout.CurrentState());
            const auto& trace=bundleExpandedLoadout.report;
            bundleReport.loadoutSlot=trace.slot;bundleReport.loadoutChoice=trace.choice;
            bundleReport.loadoutPhase=trace.phase;bundleReport.loadoutFailure=trace.failure;
            bundleReport.loadoutNativeStage=trace.nativeStage;bundleReport.loadoutSlices=trace.slices;
            bundleReport.loadoutVisited=trace.visited;bundleReport.loadoutCursor=trace.cursor;bundleReport.loadoutTotal=trace.total;
            bundleReport.loadoutReadDeferrals=trace.readDeferrals;bundleReport.loadoutRestarts=trace.restarts;
            bundleReport.loadoutLastReads=trace.lastReads;bundleReport.loadoutPreparations=trace.preparations;
            bundleReport.loadoutGrantAttempts=trace.grantAttempts;bundleReport.loadoutVerifications=trace.verifications;
            bundleReport.loadoutType=trace.type;
            bundleSchedule.loadoutActive=bundleExpandedLoadout.Active();bundleSchedule.loadoutAt=now+25;return;
        }
        const auto request=cfgloadout::restricted::FromConfiguration(bundleConfiguration);
        if(bundleLoadout.CurrentState()==cfgloadout::restricted::State::Waiting&&!bundleLoadoutResource.ready){
            auto& pending=bundleLoadoutResource;
            if(pending.Poll(now)){
                const auto field=plan.mode+(request.choice==cfgloadout::restricted::Choice::PocketKnife?cfgmenu::loadoutdisk::PocketKnifeClass:
                    request.choice==cfgloadout::restricted::Choice::FirstAid?cfgmenu::loadoutdisk::MedSprayClass:cfgmenu::loadoutdisk::MapClass);
                cfgprimary::SoftClassBytes source{},fresh{};std::uintptr_t resolved=0;
                if(!request.Valid()||!r_.Read(field,source)||!ResolveLoadedClass(request.owner,field,source,resolved))pending.failed=true;
                else if(resolved)pending.ready=true;
                else if(!pending.requested){
                    std::uintptr_t function=0,owner=0;cfgcounselorload::Packet loading;
                    if(!cfgcounselorload::Build(plan.world,plan.mode,0,source,loading)||
                       !FeatureFunction(cfgcounselorload::disk::LoadClass,function,owner)||owner!=spawnAssets.kismetClass||
                       !cfgobserver::Same(r_,function+0xB0,plan.module+cfgcounselorload::disk::Executor)||
                       !cfgbundle::Code(r_,plan.module,cfgcounselorload::disk::Executor,cfgcounselorload::disk::ExecutorCode)||
                       !Current(request.owner)||!r_.Read(field,fresh)||fresh!=source)pending.failed=true;
                    else {loading.latent.uuid=cfgloadoutresource::LatentUuid;
                        if(!pending.Request(now)||!Invoke(spawnAssets.kismetDefault,function,&loading,false))pending.failed=true;}
                }
            }
            if(pending.failed){bundleLoadout.FailResource();bundleSchedule.loadoutActive=false;}
            else {bundleLoadout.AwaitResource(now);bundleSchedule.loadoutAt=now+250;}
            bundleReport.loadoutState=static_cast<unsigned>(bundleLoadout.CurrentState());
            return; // cold load, resident proof and inventory grant are separate phases
        }
        cfgloadout::restricted::NativeAccess<Reader,BundleAccess> access(r_,*this,request,{plan.module,plan.mode,false,
#ifdef CFG_FRONTEND_BUNDLE
            NativeOfflineWorld()
#else
            false
#endif
        });
        bundleLoadout.Service(access,now);bundleReport.loadoutState=static_cast<unsigned>(bundleLoadout.CurrentState());
        const auto state=bundleLoadout.CurrentState();
        bundleSchedule.loadoutActive=state==cfgloadout::restricted::State::Waiting||state==cfgloadout::restricted::State::Dispatched;
    }
    bool BeforeHelpers(const cfgofflinesettings::Request& request){
        return !bundleReport.spawned&&!bundleReport.spawnAttempts&&!bundleReport.helperDispatched&&bundleHelpers.stage==cfgbundle::HelperStage::Project&&Current(request);
    }
    cfgofflinesettings::Write Exchange(std::uintptr_t address,std::int32_t expected,std::int32_t desired)noexcept{
        if(GetCurrentThreadId()!=plan.threadId||address%4)return cfgofflinesettings::Write::Refused;
        __try{const auto old=InterlockedCompareExchange(reinterpret_cast<volatile LONG*>(address),desired,expected);
            if(old!=expected)return cfgofflinesettings::Write::Refused;
            ++bundleReport.settingsWrites;return cfgofflinesettings::Write::Changed;}
        __except(EXCEPTION_EXECUTE_HANDLER){return cfgofflinesettings::Write::Indeterminate;}
    }
    cfgofflinesettings::Write ExchangeByte(std::uintptr_t address,std::uint8_t expected,std::uint8_t desired)noexcept{
        if(GetCurrentThreadId()!=plan.threadId)return cfgofflinesettings::Write::Refused;
        __try{const auto old=static_cast<std::uint8_t>(_InterlockedCompareExchange8(reinterpret_cast<volatile CHAR*>(address),static_cast<CHAR>(desired),static_cast<CHAR>(expected)));
            if(old!=expected)return cfgofflinesettings::Write::Refused;
            ++bundleReport.settingsWrites;return cfgofflinesettings::Write::Changed;}
        __except(EXCEPTION_EXECUTE_HANDLER){return cfgofflinesettings::Write::Indeterminate;}
    }
    bool SettingsPhase(std::uint64_t now){
        if(HostedRoutineActive())return true;
        const auto request=cfgofflinesettings::FromConfiguration(bundleConfiguration);
        if(!request.Valid())return false;
        cfgofflinesettings::NativeAccess<Reader,BundleAccess> access(r_,*this,request,{plan.module,plan.mode,spawnAssets.gameState,
#ifdef CFG_FRONTEND_BUNDLE
            NativeOfflineWorld()
#else
            false
#endif
        });
#ifdef CFG_FRONTEND_BUNDLE
        const auto& startup=nativeOfflineStartup.Status();
        if(NativeOfflineWorld()){
            if(!startup.skillVerified||startup.nativeSkill!=request.nativeSkill)return false;
            bundleReport.skillState=static_cast<unsigned>(request.nativeSkill==3?cfgofflinesettings::State::Preserved:cfgofflinesettings::State::Applied);
        }else
#endif
        if(bundleSkill.CurrentState()==cfgofflinesettings::State::Empty){
            const auto applied=bundleSkill.Apply(access,request);
            bundleReport.skillState=static_cast<unsigned>(bundleSkill.CurrentState());
            return applied; // separate skill and duration work, never stacked cold writes
        }
        bundleDuration.StepParts(access,now);bundleReport.durationState=static_cast<unsigned>(bundleDuration.CurrentState());
        if(cfgofflinesettings::DurationFinished(bundleDuration.CurrentState())){
            bundleSchedule.settingsActive=false;
            if(bundleDuration.CurrentState()==cfgofflinesettings::State::Refused||bundleDuration.CurrentState()==cfgofflinesettings::State::Indeterminate)
                cfgclosedtrace::Mark("optional-duration-unapplied",plan.mode,
                    (static_cast<std::uint64_t>(bundleDuration.partFailure)<<32)|r_.reads);
            return true; // keep failure visible; no repeat write and no unrelated AI shutdown
        }
        return true;
    }
    #include "HostedRoutineRuntimeMembers.hpp"
    bool Context(bool human=false){
        if(GetCurrentThreadId()!=plan.threadId||!originalActorDispatcher||!cfgprimary::LookupSafe(r_,plan.module))return false;
#ifdef CFG_H1_JASON
        if(HostedRoutineActive())return HostedContext(human);
#endif
        if(human){if(!cfgprimary::RefreshKnownObjectStorage(r_,plan,context))return false;}
        else{
            // AI belongs to the same owned offline world, not the human's
            // continued possession. Death/escape must not shut down Jason and
            // living helper AI. Never adopt or repair a replacement human.
            if(!cfgprimary::RetainedWorld(r_,plan,context))return false;
        }
        const bool current=(!(bundleCompletion.terminalMask&1u)||cfgjason::SameSlot(r_,context.objectArray,plan.objects[2]))&&
            cfgobserver::Same(r_,plan.world+cfgprobe::WorldGameState,spawnAssets.gameState)&&
            cfgprimary::RegistryMatches(r_,context.objectArray,spawnAssets.primary.stateSlot,
                spawnRuntime.report.result,plan.controller,spawnAssets.resources.classes[cfgspawn::PlayerState]);
        if(current&&!human)bundleContextDeferrals=0;
        return current;
    }
    // Diagnostic re-read after Context() refused: which predicate failed, on a
    // fresh bounded reader. Grants nothing; 3/5/10 are transient, others real.
    // 1=thread 2=dispatcher 3=phase-budget 4=lookup-scope 5=GC/serialize-veto
    // 6=offline-world 7=terminal-slot 8=game-state 9=registry 10=passed-on-reread
    unsigned ContextFailureStage(){
        if(GetCurrentThreadId()!=plan.threadId)return 1;
        if(!originalActorDispatcher)return 2;
        if(r_.reads>=r_.cap)return 3;
        Reader fresh;fresh.cap=256;cfgobserver::Plan observed=plan;
        if(!cfgprimary::LookupVetoClearForObservation(fresh,plan.module))return 5;
        if(!cfgprimary::LookupSafe(fresh,plan.module))return 4;
        if(!cfgprimary::RetainedWorld(fresh,plan,observed))return 6;
        if((bundleCompletion.terminalMask&1u)&&!cfgjason::SameSlot(fresh,observed.objectArray,plan.objects[2]))return 7;
        if(!cfgobserver::Same(fresh,plan.world+cfgprobe::WorldGameState,spawnAssets.gameState))return 8;
        if(!cfgprimary::RegistryMatches(fresh,observed.objectArray,spawnAssets.primary.stateSlot,
            spawnRuntime.report.result,plan.controller,spawnAssets.resources.classes[cfgspawn::PlayerState]))return 9;
        return 10;
    }
    // True: skip this phase without acting (bounded consecutive deferrals).
    bool ContextTransient(){
        const unsigned stage=ContextFailureStage();
        const bool transient=(stage==3||stage==5||stage==10)&&bundleContextDeferrals<BundleContextDeferralLimit;
        if(transient){++bundleContextDeferrals;++bundleContextDeferralsTotal;}
        if(bundleContextTraces<16&&(!transient||bundleContextDeferralsTotal<=8||bundleContextDeferrals==BundleContextDeferralLimit)){
            ++bundleContextTraces;
            cfgclosedtrace::Mark(transient?"bundle-context-deferred":"bundle-context-stop",plan.world,
                (static_cast<std::uint64_t>(stage)<<48)|(static_cast<std::uint64_t>(bundleContextDeferrals)<<32)|
                (static_cast<std::uint64_t>(r_.reads)<<16)|(r_.cap&0xFFFFu));
        }
        return transient;
    }
    bool Jason(){
        const auto& t=JasonTemplate();const auto& a=spawnAssets.resources;
#ifdef CFG_H1_JASON
        if(HostedRoutineActive())return cfgjason::LinkedAdoptedKiller(r_,context.objectArray,bundleJason,hostedAdoptionReceipt,plan,plan.pawn,
            t.pawnClass,t.defaults,spawnAssets.killerBase,a.classes[cfgspawn::KillerAI],spawnAssets.classes[3],plan.controller);
#endif
        return cfgjason::LinkedKiller(r_,context.objectArray,bundleJason,plan,plan.pawn,t.pawnClass,t.defaults,
            spawnAssets.killerBase,a.classes[cfgspawn::KillerAI],spawnAssets.classes[3],plan.controller);
    }
    bool Initialize(){
        using Step=cfgbundle::InitializationStep;
        // Observe existing short-circuit predicates only; no extra reads,
        // retries, native work or changed admission semantics for diagnostics.
        if(!BundleInitializeCheck(Step::Context,Context(true))||
            !BundleInitializeCheck(Step::SpawnState,spawnRuntime.report.state==static_cast<unsigned>(cfgspawn::SpawnState::MoveSubmitted))||
            !BundleInitializeCheck(Step::Pawn,cfgspawn::ObjectSlot(r_,context.objectArray,spawnRuntime.report.result,bundleJason.pawn))||
            !BundleInitializeCheck(Step::PawnSerial,bundleJason.pawn.serial==spawnRuntime.report.resultPawnSerial)||
            !BundleInitializeCheck(Step::Controller,cfgspawn::ObjectSlot(r_,context.objectArray,spawnRuntime.report.resultController,bundleJason.killerController))||
            !BundleInitializeCheck(Step::ControllerSerial,bundleJason.killerController.serial==spawnRuntime.report.resultControllerSerial)||
            !BundleInitializeCheck(Step::Path,cfgspawn::ObjectSlot(r_,context.objectArray,spawnRuntime.report.pathComponent,bundleJason.path))||
            !BundleInitializeCheck(Step::Jason,Jason())||
            !BundleInitializeCheck(Step::ManagerClass,r_.Read(plan.module+cfghunter::disk::ManagerReflection,bundleManagerClass)&&bundleManagerClass)||
            !BundleInitializeCheck(Step::ManagerPair,cfgobserver::Same(r_,plan.module+cfghunter::disk::ManagerConstructor,bundleManagerClass))||
            !BundleInitializeCheck(Step::ManagerIdentity,cfgspawn::ObjectSlot(r_,context.objectArray,bundleManagerClass,bundleManagerClassSlot))||
            !BundleInitializeCheck(Step::CounselorClass,r_.Read(plan.module+cfgprobe::NativeClasses[cfgprobe::Counselor].reflectionCacheRva,bundleCounselorClass))||
            !BundleInitializeCheck(Step::TreeIdentity,cfgspawn::ObjectSlot(r_,context.objectArray,spawnAssets.primary.counselorTree,bundleTreeSlot))||
            !BundleInitializeCheck(Step::Tree,Tree())||!BundleInitializeCheck(Step::NativeBindings,NativeBindings())||
            !BundleInitializeCheck(Step::Manager,Manager()))return false;
        // Optional bounded read-only diagnostic, separate from gameplay/native
        // admission. Do not use its observations to grant engine authority.
        if(!bundlePreviousWorld.world.object||bundlePreviousWorld.checked){Reader diagnostic;diagnostic.cap=96;
            cfgworlddiag::BeginRecord(bundlePreviousWorld); // stale Ready may not survive a failed new record/cache read
            std::uintptr_t packageType=0;
            if(diagnostic.Read(plan.module+cfgcompat::cfg35::worldMap::NativePackageCache,packageType))
                (void)cfgworlddiag::Record(diagnostic,context.objectArray,plan.world,bundlePreviousWorld,packageType);}
        bundleReport.jason=bundleJason.pawn.object;bundleReport.target=plan.pawn;
        // One-time, read-only participant evidence. Null is recorded, never
        // repaired by borrowing the human player's role or creating fake state.
        if(r_.Read(bundleJason.pawn.object+cfgescape::disk::PlayerState,bundleReport.initialJasonPawnState))bundleReport.initialKillerStateReadMask|=1;
        if(r_.Read(bundleJason.killerController.object+0x388,bundleReport.initialJasonControllerState))bundleReport.initialKillerStateReadMask|=2;
        if(r_.Read(spawnAssets.gameState+cfgprimary::CurrentKillerState,bundleReport.initialRegisteredKillerState))bundleReport.initialKillerStateReadMask|=4;
        bundleKillOriginal=reinterpret_cast<cfgkill::EligibilityOriginal>(plan.module+cfgkill::disk::Eligibility);
#ifdef CFG_FRONTEND_BUNDLE
        if(NativeOfflineWorld()){
            const auto& startup=nativeOfflineStartup.Status();
            const auto cohort=cfgpopulation::Make(bundleConfiguration.counselorBots);
            if(!BundleInitializeCheck(Step::NativeCohort,cohort.valid&&startup.bots==cohort.native&&startup.bots<=cfgbundle::MaximumBots))return false;
            std::array<cfgbundle::Bot,cfgbundle::MaximumBots> bots{};
            std::array<cfgprobe::lifetime::SlotObservation,cfgbundle::MaximumBots> pawns{},controllers{};
            for(unsigned i=0;i<startup.bots;++i){
                const auto& born=startup.roster[i];std::uintptr_t type=0,controller=0;
                if(!BundleInitializeCheck(Step::HelperClass,born.entered&&born.accounted&&r_.Read(born.pawn+cfgprobe::ObjectClass,type))||
                   !BundleInitializeCheck(Step::HelperSpawn,cfgspawn::ObserveCounselorSpawnResult(r_,plan,spawnAssets.gameState,type,bundleCounselorClass,
                       spawnAssets.resources.classes[cfgspawn::CounselorAI],spawnAssets.resources.classes[cfgspawn::Brain],born.pawn,controller)&&controller==born.controller)||
                   !BundleInitializeCheck(Step::HelperPlayerState,cfgobserver::Same(r_,born.pawn+0x388,born.state)&&cfgobserver::Same(r_,born.controller+0x388,born.state))||
                   !BundleInitializeCheck(Step::HelperPawnIdentity,cfgspawn::ObjectSlot(r_,context.objectArray,born.pawn,pawns[i])&&pawns[i]==born.identities[2])||
                   !BundleInitializeCheck(Step::HelperControllerIdentity,cfgspawn::ObjectSlot(r_,context.objectArray,born.controller,controllers[i])&&controllers[i]==born.identities[0]))return false;
                for(unsigned j=0;j<i;++j)if(!BundleInitializeCheck(Step::HelperDuplicate,bots[j].pawn!=born.pawn&&bots[j].controller!=controller))return false;
                bots[i]={born.pawn,controller,pawns[i].serial,controllers[i].serial};
                bundleRandomCounselors.Reserve(type); // F3 helpers pick a different character
            }
            // Adopt the normal donor cohort. Only the difference to the chosen
            // total is placed by the existing paced authored-start pipeline.
            bundleReport.roster=bots;bundlePawnSlots=pawns;bundleControllerSlots=controllers;
            bundleReport.bots=bundleReport.spawned=startup.bots;
            if(!BundleInitializeCheck(Step::HelperAdoption,bundleHelpers.AdoptNative(startup.bots,bundleConfiguration.counselorBots)))return false;
        }
#endif
        return true;
    }
    bool NativeBindings(){
        return cfgbundle::NativeBindings(r_,plan.module,bundleJason.pawn.object);
    }
    struct JasonIdentity {
        cfgprobe::lifetime::SlotObservation pawn{},controller{};
        bool hosted=false;
    };
    bool JasonAdmission(JasonIdentity& identity){
        identity={bundleJason.pawn,bundleJason.killerController,false};
#ifdef CFG_H1_JASON
        if(HostedRoutineActive()){
            const auto& receipt=hostedAdoptionReceipt;
            if(!(identity.pawn==receipt.pawn)||!(identity.controller==receipt.controller)||
               !cfghosted::adoption::Current(r_,context.objectArray,receipt,plan))return false;
            identity={receipt.pawn,receipt.controller,true};
        }
#endif
        return true;
    }
    bool DoorPressBinding(){
        if(!HostedRoutineActive())return NativeBindings();
        JasonIdentity identity{};
        return JasonAdmission(identity)&&cfgbundle::PressBinding(r_,plan.module,identity.pawn.object);
    }
    bool FootObstacleBindings(){
        if(!HostedRoutineActive())return NativeBindings();
        JasonIdentity identity{};
        return JasonAdmission(identity)&&HostedCarBindings();
    }
#if defined(CFG_H1_JASON) && defined(CFG_H1_OBSERVE) && defined(CFG_RETAINED_GAMEPLAY_BUNDLE) && defined(CFG_FRONTEND_BUNDLE)
    bool HostedRosterCounselor(std::uintptr_t pawn,cfgdonorhunt::CounselorObservation& observation){
        observation={};
        if(!HostedRoutineActive()||!(bundleDonorRoster.world==plan.objects[0]))return false;
        const auto count=bundleDonorRoster.roster.count;
        if(count<=0||static_cast<unsigned>(count)>bundleDonorRoster.pawns.size())return false;
        const auto index=bundleDonorRoster.roster.DonorFindJasonAITargetIndex(pawn);
        if(index<0)return false;
        const auto slot=bundleDonorRoster.pawns[static_cast<unsigned>(index)];
        if(slot.object!=pawn||!slot.HasAssignedSerial())return false;
        Reader proof;proof.cap=cfgbundle::ReadCap;
        cfgprobe::lifetime::ArrayHeader registry{};
        const bool okay=cfgprobe::lifetime::ReadArrayHeader(proof,plan.module+cfgprobe::lifetime::ObjectArrayRva,registry)&&
            cfgdonorhunt::ObserveDonorCounselor(proof,registry,plan.module,plan.world,slot,bundleCounselorClass,observation)&&observation.Live();
        bundleReport.reads+=proof.reads;
        bundleReport.maxReads=(std::max)(bundleReport.maxReads,proof.reads);
        return okay;
    }
#endif
    bool ReleaseBinding(){
        return cfgbundle::ReleaseBinding(r_,plan.module,bundleJason.pawn.object);
    }
#include "BundleManagerObservationMembers.hpp"
    bool Tree(){
        cfgprobe::lifetime::SlotObservation fresh{};
        return cfgobserver::Same(r_,plan.mode+cfgprimary::CounselorTree,spawnAssets.primary.counselorTree)&&
            cfgspawn::ObjectSlot(r_,context.objectArray,spawnAssets.primary.counselorTree,fresh)&&fresh==bundleTreeSlot&&
            cfgspawn::ObjectIs(r_,spawnAssets.primary.counselorTree,spawnAssets.resources.classes[cfgspawn::Tree]);
    }
    bool Alive(std::uintptr_t pawn,bool& alive){
        if(pawn==bundleJason.pawn.object)return cfgbundle::Alive(r_,pawn,alive);
        cfgescape::State state;alive=false;
        if(!cfgescape::Counselor(r_,context.objectArray,plan.module,pawn,bundleCounselorClass,state))return false;
        alive=state==cfgescape::State::Living;return true;
    }
    bool Held(std::uintptr_t& held){return r_.Read(bundleJason.pawn.object+cfghunter::disk::Held,held);}
    bool MirrorHuman(cfgemotemirror::Slot& out){
        std::uintptr_t pawn=0;out={};
        if(!Known(0,pawn))return false;out=bundleHumanTarget;return out.object==pawn;
    }
    #if !CFG_J21_EMOTE_MIRROR
cfgemotemirror::Inputs MirrorInputs(){
        cfgemotemirror::Inputs in;
        in.module=plan.module;in.world=plan.world;in.controller=plan.controller;in.controllerSlot=plan.objects[2];
        in.counselorClass=bundleCounselorClass;in.counselorAIClass=spawnAssets.resources.classes[cfgspawn::CounselorAI];
        in.count=bundleReport.bots;
        if(in.count<=cfgemotemirror::Queue::Capacity)
            for(unsigned i=0;i<in.count;++i)in.bots[i]={bundlePawnSlots[i],bundleControllerSlots[i]};
        return in;
    }
#else
cfgemotemirror::Inputs MirrorInputs(){
        cfgemotemirror::Inputs in;
        in.module=plan.module;in.world=plan.world;in.controller=plan.controller;in.controllerSlot=plan.objects[2];
        in.counselorClass=bundleCounselorClass;in.counselorAIClass=spawnAssets.resources.classes[cfgspawn::CounselorAI];
        // Donor convergence/emotes own the complete observed roster, including
        // native births. F3's helper-only report is not the AI population.
#if defined(CFG_RETAINED_GAMEPLAY_BUNDLE) && defined(CFG_FRONTEND_BUNDLE)
        if(bundleDonorRoster.world==plan.objects[0]){
            const auto& roster=bundleDonorRoster.roster;
            for(int i=0;i<roster.count&&in.count<in.bots.size();++i){
                const auto pawn=bundleDonorRoster.pawns[static_cast<unsigned>(i)];
                if(!pawn.HasAssignedSerial()||pawn.object==bundleHumanTarget.object)continue;
                std::uintptr_t controller=0;if(!r_.Read(pawn.object+cfgescape::disk::Controller,controller))break;
                for(int c=0;c<roster.controllerCount;++c){const auto known=bundleDonorRoster.controllers[static_cast<unsigned>(c)];
                    if(known.object==controller){in.bots[in.count++]={pawn,known};break;}}
            }
        }else
#endif
        {
            in.count=bundleReport.bots;
            if(in.count<=in.bots.size())for(unsigned i=0;i<in.count;++i)in.bots[i]={bundlePawnSlots[i],bundleControllerSlots[i]};
        }
        return in;
    }
#endif
    #if !CFG_J21_EMOTE_MIRROR
void MirrorPrepare(){
        cfgemotemirror::NativeAccess<Reader,BundleAccess> access(r_,*this,MirrorInputs());bundleEmotes.Prepare(access);
    }
#else
void MirrorPrepare(){
        if(!cfgj21::EmotesEnabled)return;
        cfgemotemirror::NativeAccess<Reader,BundleAccess> access(r_,*this,MirrorInputs());bundleEmotes.Prepare(access);
    }
#endif
    void MirrorBefore(std::uintptr_t object,std::uintptr_t function,std::uintptr_t parameters,bool nested){
        cfgemotemirror::NativeAccess<Reader,BundleAccess> access(r_,*this,MirrorInputs());
        bundleEmotes.Before(access,object,function,parameters,nested);
    }
    void MirrorAfter(std::uintptr_t object,std::uintptr_t function,bool nested,bool completed,std::uint64_t now){
        cfgemotemirror::NativeAccess<Reader,BundleAccess> access(r_,*this,MirrorInputs());
        bundleEmotes.After(access,object,function,nested,completed,now);
    }
    #if !CFG_J21_EMOTE_MIRROR
void MirrorService(std::uint64_t now){
#ifdef CFG_H1_JASON
        HostedLaneEnter(cfgh1::Lane::Emote);
#endif

        if(bundleKill.policy.Exclusive(now)){
            cfgkill::Context death;
            if(!Context()||!DeathContext(death)||death.active||bundleKill.policy.interactionAttempted)return;
        }
        if(!bundleEmotes.Due(now)&&MirrorMaintain(now))return;
        Reader selection;selection.cap=cfgbundle::ReadCap;
#if CFG_H245_EMOTE && defined(CFG_H1_JASON)
    selection.cap=cfghostedbudget::EmoteReadCap(HostedRoutineActive(),selection.cap); // one shared parameterised proof budget, H2.45
#endif
        cfgemoteobservation::PlaybackAccess<Reader,BundleAccess> host(selection,plan,*this);
        cfgemotemirror::NativeAccess<Reader,decltype(host)> access(selection,host,MirrorInputs());
        bundleEmotes.Service(access,now);
        bundleReport.reads+=selection.reads;
        bundleReport.maxReads=(std::max)(bundleReport.maxReads,selection.reads);
    }
#else
void MirrorService(std::uint64_t now){
#ifdef CFG_H1_JASON
        HostedLaneEnter(cfgh1::Lane::Emote);
#endif

        if(bundleKill.policy.Exclusive(now)){
            cfgkill::Context death;
            if(!Context()||!DeathContext(death)||death.active||bundleKill.policy.interactionAttempted)return;
        }
        Reader selection;selection.cap=cfgbundle::ReadCap;
#if CFG_H245_EMOTE && defined(CFG_H1_JASON)
    selection.cap=cfghostedbudget::EmoteReadCap(HostedRoutineActive(),selection.cap); // one shared parameterised proof budget, H2.45
#endif
        cfgemoteobservation::PlaybackAccess<Reader,BundleAccess> host(selection,plan,*this);
        cfgemotemirror::NativeAccess<Reader,decltype(host)> access(selection,host,MirrorInputs());
        DWORD foreground=0;GetWindowThreadProcessId(GetForegroundWindow(),&foreground);
        const bool movement=foreground==GetCurrentProcessId()&&
            ((GetAsyncKeyState('W')&0x8000)||(GetAsyncKeyState('A')&0x8000)||
             (GetAsyncKeyState('S')&0x8000)||(GetAsyncKeyState('D')&0x8000));
        bundleEmotes.Service(access,now,movement);
        if(access.proofStage){bundleReport.j21EmoteProofStage=access.proofStage;bundleReport.j21EmoteProofDifference=access.proofDifference;
            bundleReport.j21EmoteProofReads=selection.reads;bundleReport.j21EmoteProofError=selection.error;} // retain last failure across idle playback turns
        bundleReport.reads+=selection.reads;
        bundleReport.maxReads=(std::max)(bundleReport.maxReads,selection.reads);
    }
#endif
#include "GrabRuntimeMembers.hpp"
#include "SpectatorRuntimeMembers.hpp"
#include "NativeSpectatorLabelMembers.hpp"
#include "DeadSpectatorRecoveryMembers.hpp"
#include "CurrentHelperOwnerMembers.hpp"
    bool Known(unsigned index,std::uintptr_t& pawn){
        pawn=0;
        if(index==BundleNativeTargetIndex){
            cfgnativepursuit::Proof fresh{};
            if(!bundleNativeTarget.pawn.object||!cfgnativepursuit::Observe(r_,context.objectArray,plan.module,
                spawnAssets.gameState,bundleCounselorClass,spawnAssets.resources.classes[cfgspawn::BaseAI],
                bundleJason.pawn.object,bundleNativeTarget.index,bundleNativeTarget.pawn.object,fresh,&bundleNativeTarget))return HostedDonorTargetFallback(pawn);
            pawn=fresh.pawn.object;return true;
        }
        if(index>bundleReport.bots)return false;
        if(!index){
            if(!bundleHumanTarget.object)bundleHumanTarget=plan.objects[3];
            std::uintptr_t current=0;
            if(!cfgjason::SameSlot(r_,context.objectArray,plan.objects[2])||
               !r_.Read(plan.controller+cfgescape::disk::Pawn,current)){
                bundleCompletion.Observe(0,cfgescape::State::Unknown);return false;
            }
            // Follow only the already-owned local controller. Tommy is a new
            // target observation, never a replacement Plan or mutation target.
            if(current&&current!=bundleHumanTarget.object){
                if(!cfgspawn::ObjectIs(r_,current,bundleCounselorClass)){
                    // Stock escape/death may immediately possess a spectator
                    // pawn. Preserve proved terminal status of the LAST local
                    // counselor, not Unknown merely because spectating began.
                    // Native roster/Tommy-pending checks still veto completion.
                    cfgescape::State previous;
                    if(cfgjason::SameSlot(r_,context.objectArray,bundleHumanTarget)&&
                       cfgescape::Counselor(r_,context.objectArray,plan.module,bundleHumanTarget.object,bundleCounselorClass,previous)&&
                       cfgescape::Terminal(previous))bundleCompletion.Observe(0,previous);
#ifdef CFG_H1_JASON
                    else if(HostedRoutineActive()){
                        cfgescape::State hostedTerminal=cfgescape::State::Unknown;
                        if(NativeJasonLabelProof(&hostedTerminal)&&cfgescape::Terminal(hostedTerminal))
                            bundleCompletion.Observe(0,hostedTerminal);
                        else bundleCompletion.Observe(0,cfgescape::State::Unknown);
                    }
#endif
                    else bundleCompletion.Observe(0,cfgescape::State::Unknown);
                    return false;
                }
                cfgprobe::lifetime::SlotObservation fresh{};
                if(!cfgspawn::ObjectSlot(r_,context.objectArray,current,fresh)||!fresh.HasAssignedSerial()||
                   !cfgobserver::Same(r_,current+cfgescape::disk::Controller,plan.controller)||
                   !cfgobserver::Same(r_,plan.controller+cfgescape::disk::Pawn,current))return false;
                bundleHumanTarget=fresh;bundleCompletion.Observe(0,cfgescape::State::Unknown);
            }
            pawn=bundleHumanTarget.object;
            if(!cfgjason::SameSlot(r_,context.objectArray,bundleHumanTarget))return false;
            cfgescape::State state;
            if(!cfgescape::Counselor(r_,context.objectArray,plan.module,pawn,bundleCounselorClass,state)){
                bundleCompletion.Observe(0,cfgescape::State::Unknown);return false;
            }
            bundleCompletion.Observe(0,state);
            return state==cfgescape::State::Living&&current==pawn;
        }
        const auto i=index-1;const auto& bot=bundleReport.roster[i];pawn=bot.pawn;
        if(!cfgjason::SameSlot(r_,context.objectArray,bundlePawnSlots[i]))return false;
        cfgescape::State state;
        if(!cfgescape::Counselor(r_,context.objectArray,plan.module,pawn,bundleCounselorClass,state)){
            bundleCompletion.Observe(index,cfgescape::State::Unknown);return false;
        }
        bundleCompletion.Observe(index,state);
        return state==cfgescape::State::Living&&
            cfgjason::SameSlot(r_,context.objectArray,bundleControllerSlots[i])&&
            cfgobserver::Same(r_,pawn+0x3A0,bot.controller)&&cfgobserver::Same(r_,bot.controller+0x370,pawn)&&
            cfgspawn::ObjectIs(r_,pawn,bundleCounselorClass)&&
            cfgspawn::ObjectIs(r_,bot.controller,spawnAssets.resources.classes[cfgspawn::CounselorAI]);
    }
    bool NativeTerminal(){
        // Only inspect the native owned character roster after an empty target
        // sweep. Never enumerate levels, GUObjects, actors, or names to hunt.
        ++bundleReport.nativeTerminalChecks;
        cfgescape::TerminalWitness witness;
        const bool ready=cfgescape::NativeRosterTerminal(r_,context.objectArray,plan.module,plan.mode,
            spawnAssets.gameState,bundleCounselorClass,bundleJason.pawn.object,&witness);
        bundleReport.nativeTerminalFailure=witness.failure;bundleReport.nativeTerminalIndex=witness.index;
        bundleReport.nativeTerminalPawn=witness.pawn;bundleReport.nativeTerminalPending=witness.pending;
        bundleReport.nativeTerminalCount=witness.count;bundleReport.nativeTerminalHunter=witness.hunter;
        // Same discovery source as native completion. A nonterminal witness
        // cannot authorize an actor action; acquisition gets its own scope.
        bundleNativeTargetHint=witness.failure==6?witness.pawn:0;
        bundleNativeTargetHintIndex=witness.index;
        if(ready)++bundleReport.nativeTerminalPasses;return ready;
    }
    bool NativePursuitAcquire(){
        if(!bundleNativeTargetHint)return false;
        cfgnativepursuit::Proof proof{};
        if(!cfgnativepursuit::Observe(r_,context.objectArray,plan.module,spawnAssets.gameState,
            bundleCounselorClass,spawnAssets.resources.classes[cfgspawn::BaseAI],bundleJason.pawn.object,
            bundleNativeTargetHintIndex,bundleNativeTargetHint,proof)){
            ++bundleReport.nativePursuitRefusals;bundleNativeTargetHint=0;return false;
        }
        bundleNativeTargetHint=0;
        if(bundleTargetIndex==BundleNativeTargetIndex&&bundleReport.target==proof.pawn.object&&
           bundleNativeTarget.pawn==proof.pawn&&bundleNativeTarget.controller==proof.controller)return true;
        bundleNativeTarget=proof;++bundleReport.nativePursuitAcquires;
        bundleTargetIndex=BundleNativeTargetIndex;bundleReport.target=proof.pawn.object;
        bundlePursuit.Clear();bundleFollowPath.Clear();bundlePathCooldown.Clear();
        bundleMoveAt=0;bundleTargetLockUntil=0;bundleCompletion.candidateSince=0;return true;
    }
    bool RetireTarget(){
        std::uintptr_t function=0,owner=0;
        if(!FeatureFunction(cfgtrap::disk::StopMovement,function,owner)||
           !cfgspawn::ObjectIs(r_,bundleJason.killerController.object,owner)||
           !cfgobserver::Same(r_,function+0xB0,plan.module+cfgtrap::disk::StopMovementExecutor)||
           !cfgbundle::Code(r_,plan.module,cfgtrap::disk::StopMovementExecutor,cfgtrap::disk::StopMovementCode)||
           !Invoke(bundleJason.killerController.object,function,nullptr))return false;
        ++bundleReport.targetRetireStops;
        bundleReport.target=0;bundleTargetLockUntil=0;bundleTrapPriority.Clear();bundlePursuit.Clear();bundleFollowPath.Clear();bundlePathCooldown.Clear();return true;
    }
    bool TrapPriorityCurrent(std::uintptr_t pawn){return pawn==bundleTrapPriority.victim&&cfgjason::SameSlot(r_,context.objectArray,bundleTrapVictimSlot);}
#include "TrapEventObservationMembers.hpp"
    bool TrapPriorityPhase(std::uint64_t now){
#ifdef CFG_H1_JASON
        HostedLaneEnter(cfgh1::Lane::TrapPriority);
#endif

        // Occupied car FIRST, fresh caught victim SECOND. Drain uncertain
        // placement/animation before suspending; unpaid objectives resume.
        const bool car=bundleCarPursuit.Active(now)||bundleCarActionPending.active||bundleRoadMorph.stage;
        if(!bundleTrapPriority.ServiceDue(bundleTraps.Busy()&&!bundleTraps.CanSuspend(),car))return
#ifdef CFG_H1_JASON
 HostedLaneRefuse(cfgh1::Lane::TrapPriority,1)
#else
 false
#endif
;
        std::uintptr_t victim=0;
        if(!bundleTrapPriority.Active(now)||!Known(bundleTrapPriority.index,victim)||victim!=bundleTrapPriority.victim||
           !cfgjason::SameSlot(r_,context.objectArray,bundleTrapVictimSlot)||!KnownLiving(victim)){
            bundleTrapPriority.Clear();++bundleTrapRefused;return true;
        }
        cfgtrappriority::Slot freshVictim{},freshTrap{};
        if(!cfgtrappriority::Capture(r_,context.objectArray,plan.module,bundleJason.pawn.object,victim,
           bundleTriggeredTrapSlot.object,freshVictim,freshTrap)||!(freshVictim==bundleTrapVictimSlot)||!(freshTrap==bundleTriggeredTrapSlot)){
            bundleTrapPriority.Clear();++bundleTrapRefused;return true;}
        if(bundleTraps.Busy()&&!bundleTraps.PauseForVehicle(now))return
#ifdef CFG_H1_JASON
 HostedLaneRefuse(cfgh1::Lane::TrapPriority,2)
#else
 false
#endif
;
        if(bundleCarSpeed.active){if(!CarBoost(false)){++bundleTrapRefused;bundleTrapPriority.Clear();}return true;}
        CarRetire(now);
        bundleTargetIndex=bundleTrapPriority.index;bundleReport.target=victim;bundleTargetLockUntil=now+6000;
        bundlePursuit.Clear();bundleFollowPath.Clear();bundlePathCooldown.Clear();bundleDoorRecovery={};bundleDoorBreakSelection={};
        bundleMoveAt=now;bundleSchedule.huntAt=now;bundleTrapPriority.pending=false;
        return true; // next callback owns movement/Morph, never the overlap callback
    }
    #include "EndingRuntimeMembers.hpp"
    #include "SwimRuntimeMembers.hpp"
    #include "JasonDeathCompletionMembers.hpp"
    #include "BotNameRuntimeMembers.hpp"
    void ObservePostMatch(){
        ++bundleReport.postMatchChecks;bundleReport.postMatchFailure=1;
        if(!cfgprimary::LookupSafe(r_,plan.module)||!GameplayWorld(context)){
            bundlePostMatchWatch.Lost();return;
        }
        cfgnames::Words phase{};cfgnames::Text text;cfgnames::Observation<Reader> names(r_,plan.module);
        bundleReport.postMatchFailure=2;
        if(!r_.Read(plan.mode+cfgescape::disk::MatchState,phase)||!names.Decode(phase,text)||
           !cfgescape::Same(r_,plan.mode+cfgescape::disk::MatchState,phase))return;
        bundleReport.postMatchPhase={};const auto label=text.View();
        for(std::size_t i=0;i<label.size()&&i+1<bundleReport.postMatchPhase.size();++i)
            bundleReport.postMatchPhase[i]=label[i]<=127?static_cast<char>(label[i]):'?';
        ++bundleReport.postMatchObserved;bundleReport.postMatchFailure=0;bundlePostMatchWatch.Observe(label);
        const auto now=GetTickCount64();
        bool inactive=false;
        if(label==u"PostMatchOutro"&&bundlePostMatchRecovery.observed&&
           bundlePostMatchRecovery.Elapsed(now)>=cfgoutro::InactiveGraceMs&&
           bundlePostMatchRecovery.Elapsed(now)<cfgoutro::NativeGraceMs){cfgoutro::Owner owner;
            inactive=cfgoutro::ObserveOwner(r_,context.objectArray,plan.module,plan.world,owner)&&
                cfgoutro::InactiveSequence(r_,context.objectArray,plan.module,plan.world,owner);}
        bundleReport.outroInactiveProof=inactive?1u:0u;
        const auto recovery=bundlePostMatchRecovery.Next(label,now,inactive);
        bundleReport.outroObservedAt=bundlePostMatchRecovery.observed?bundlePostMatchRecovery.since:0;
        bundleReport.outroElapsedMs=bundlePostMatchRecovery.Elapsed(now);
        bundleReport.outroRecoveryDue=recovery!=cfgoutro::Action::None;
        if(recovery!=cfgoutro::Action::None)RecoverPostMatch(recovery,phase);
    }
    unsigned ObserveHostedEndState(){
        if(!HostedRoutineActive())return 0;
        bundleReport.hostedEndReadFailure=1;
        cfgnames::Words phase{};cfgnames::Text text;
        cfgnames::Observation<Reader> names(r_,plan.module);
        std::uintptr_t pawn=0;
        if(!r_.Read(plan.mode+cfgescape::disk::MatchState,phase)||!names.Decode(phase,text)||
           !cfgescape::Same(r_,plan.mode+cfgescape::disk::MatchState,phase)||
           !r_.Read(plan.controller+cfgescape::disk::Pawn,pawn))return 0;
        const auto label=text.View();std::array<char,33> phaseText{};
        for(std::size_t i=0;i<label.size()&&i+1<phaseText.size();++i)
            phaseText[i]=label[i]<=127?static_cast<char>(label[i]):'?';
        const bool phaseChanged=bundleReport.hostedEndSamples==0||phaseText!=bundleReport.hostedEndPhase;
        if(pawn!=bundleReport.hostedEndPawn){if(bundleReport.hostedEndSamples)++bundleReport.hostedEndPawnChanges;
            bundleReport.hostedEndPawn=pawn;}
        if(phaseChanged){
            if(bundleReport.hostedEndSamples)++bundleReport.hostedEndPhaseChanges;
            bundleReport.hostedEndPhase=phaseText;
        }
        ++bundleReport.hostedEndSamples;bundleReport.hostedEndReadFailure=0;
        return label==u"InProgress"?1u:2u; // 0=unproven, 1=live, 2=confirmed out of play
    }
    bool RecoverPostMatch(cfgoutro::Action action,cfgnames::Words phase){
        auto& recovery=bundlePostMatchRecovery;
        ++bundleReport.outroRecoveryChecks;bundleReport.outroRecoveryFailure=1;
        const auto refuse=[&](){if(r_.reads<r_.cap&&!exception_)recovery.finished=true;return false;};
        std::uintptr_t modeClass=0,modeTable=0;
        if(!cfgprimary::LookupSafe(r_,plan.module)||!GameplayWorld(context)||
           !cfgescape::Same(r_,plan.mode+cfgescape::disk::MatchState,phase)||
           !cfgescape::Same(r_,plan.mode+0x110,std::uint8_t{3})||
           !NativeClass(cfgofflinesettings::disk::SCGameModeClass,cfgofflinesettings::disk::SCGameModeConstructor,modeClass)||
           !cfgspawn::ObjectIs(r_,plan.mode,modeClass)||!r_.Read(plan.mode,modeTable)||
           modeTable<=plan.module||modeTable>=plan.module+0x3000000||
           !cfgobserver::Same(r_,modeTable+0x938,plan.module+0x3A7070)||
           !cfgobserver::Same(r_,modeTable+0x7E8,plan.module+cfgescape::disk::EndMatchNative)||
           !cfgtommyend::EndBodyCurrent(r_,plan.module)||
           !cfgbundle::Code(r_,plan.module,0x3A7070,std::array<unsigned char,10>{{0x48,0x8B,0x01,0x48,0xFF,0xA0,0xE8,0x07,0,0}}))return refuse();
        cfgprobe::lifetime::ArrayHeader registry;cfgoutro::Owner owner;
        bundleReport.outroRecoveryFailure=2;
        if(!cfgprobe::lifetime::ReadArrayHeader(r_,plan.module+cfgprobe::lifetime::ObjectArrayRva,registry)||
           !cfgoutro::ObserveOwner(r_,registry,plan.module,plan.world,owner)||
           (recovery.settings.object&&!(recovery.settings==owner.settings)))return refuse();
        if(!recovery.skipped&&recovery.Elapsed(GetTickCount64())<cfgoutro::NativeGraceMs&&
           !cfgoutro::InactiveSequence(r_,registry,plan.module,plan.world,owner))return false;
        std::uintptr_t function=0;const bool complete=action==cfgoutro::Action::Complete;
        const auto cache=complete?cfgoutro::disk::CompleteCache:cfgoutro::disk::SkipCache;
        const auto executor=complete?cfgoutro::disk::CompleteExecutor:cfgoutro::disk::SkipExecutor;
        cfgreflection::Observation<Reader> reflection(r_,plan.module);cfgreflection::Field field;
        std::uint8_t parms=1;std::uint16_t bytes=1;std::uint32_t flags=0;
        bundleReport.outroRecoveryFailure=3;
        if(!r_.Read(plan.module+cache,function))return refuse();
        if(!function){
            // Donor resolves the native callback by its owner's reflected
            // hierarchy. CFG's registration cache can still be cold. Warm the
            // pinned factory ONCE, in its own callback, then repeat all native
            // ownership/frame checks before any outro action next time.
            const auto bit=complete?2u:1u;
            const auto factory=complete?cfgoutro::disk::CompleteFactory:cfgoutro::disk::SkipFactory;
            bundleReport.outroRecoveryFailure=5;
            const bool factoryCode=complete?
                cfgbundle::Code(r_,plan.module,factory,cfgoutro::disk::CompleteFactoryCode):
                cfgbundle::Code(r_,plan.module,factory,cfgoutro::disk::SkipFactoryCode);
            cfgoutro::Owner current;
            if(r_.reads+300>cfgbundle::ReadCap||calls_>=cfgbundle::CallCap)return false; // no action/attempt consumed; finite witness may defer
            if((recovery.factoryAttempts&bit)||!factoryCode||
               !cfgoutro::ObserveOwner(r_,registry,plan.module,plan.world,current)||!(current.settings==owner.settings)||
               !(current.level==owner.level)||!cfgprimary::LookupSafe(r_,plan.module)||
               !GameplayWorld(context)||
               !cfgescape::Same(r_,plan.mode+cfgescape::disk::MatchState,phase)||
               !cfgobserver::Same(r_,plan.module+cache,std::uintptr_t{}))return refuse();
            if(!NativeCallAllowed())return false;
#ifdef CFG_RETAINED_GAMEPLAY_BUNDLE
            if(!RetainPrimaryObjects(r_,std::array<std::uintptr_t,3>{owner.type,owner.settings.object,owner.level.object})||!NativeCallAllowed())return false;
#endif
            if(!recovery.AttemptFactory(bit))return refuse();
#ifdef CFG_RETAINED_GAMEPLAY_BUNDLE
            if(!BeginBundleNative(1))return false;
#endif
            bundleReport.outroFactoryAttempts=recovery.factoryAttempts;++calls_;
            __try{function=reinterpret_cast<std::uintptr_t(__fastcall*)()>(plan.module+factory)();}
            __except(EXCEPTION_EXECUTE_HANDLER){exception_=true;recovery.finished=true;
#ifdef CFG_RETAINED_GAMEPLAY_BUNDLE
                BundleRetentionFailed();
#endif
                return false;}
#ifdef CFG_RETAINED_GAMEPLAY_BUNDLE
            if(!RetainBundleReturnedObject(function)){recovery.finished=true;return false;}
#endif
            cfgprobe::lifetime::ArrayHeader freshRegistry;
            if(!function||!cfgprimary::LookupSafe(r_,plan.module)||!GameplayWorld(context)||
               !cfgescape::Same(r_,plan.mode+cfgescape::disk::MatchState,phase)||
               !cfgprobe::lifetime::ReadArrayHeader(r_,plan.module+cfgprobe::lifetime::ObjectArrayRva,freshRegistry)||
               !cfgoutro::ObserveOwner(r_,freshRegistry,plan.module,plan.world,current)||!(current.settings==owner.settings)||
               !(current.level==owner.level)||!cfgobserver::Same(r_,plan.module+cache,function)||
               !cfgobserver::Same(r_,function+0x20,owner.type)){recovery.finished=true;return false;}
            bundleReport.outroFactoryReturns|=bit;bundleReport.outroRecoveryFailure=0;
            return true; // no Skip/Complete action in this registration slice
        }
        if(!reflection.ReadField(function,owner.type,field)||
           field.kind.View()!=u"Function"||field.label.View()!=(complete?u"OutroCompleted":u"SkipLevelIntroOutro")||
           !r_.Read(function+0x88,flags)||!(flags&0x400)||
           !r_.Read(function+0x8C,parms)||parms||!r_.Read(function+0x8E,bytes)||bytes||
           !cfgobserver::Same(r_,function+0xB0,plan.module+executor))return refuse();
        const bool code=complete?
            cfgbundle::Code(r_,plan.module,executor,cfgoutro::disk::CompleteExecutorCode)&&cfgbundle::Code(r_,plan.module,cfgoutro::disk::CompleteNative,cfgoutro::disk::CompleteNativeCode):
            cfgbundle::Code(r_,plan.module,executor,cfgoutro::disk::SkipExecutorCode)&&cfgbundle::Code(r_,plan.module,cfgoutro::disk::SkipNative,cfgoutro::disk::SkipNativeCode);
        cfgoutro::Owner again;bundleReport.outroRecoveryFailure=4;
        if(!code||!cfgoutro::ObserveOwner(r_,registry,plan.module,plan.world,again)||!(again.settings==owner.settings)||
           !(again.level==owner.level)||!GameplayWorld(context)||
           !cfgspawn::ObjectIs(r_,plan.mode,modeClass)||!cfgobserver::Same(r_,plan.mode,modeTable)||
           !cfgobserver::Same(r_,modeTable+0x938,plan.module+0x3A7070)||
           !cfgobserver::Same(r_,modeTable+0x7E8,plan.module+cfgescape::disk::EndMatchNative)||
           !cfgprimary::LookupSafe(r_,plan.module)||!cfgescape::Same(r_,plan.mode+cfgescape::disk::MatchState,phase)||
           !cfgobserver::Same(r_,plan.module+cache,function)||calls_>=cfgbundle::CallCap)return refuse();
        // Consume before dispatch, including exceptions. This is the donor's
        // native WorldSettings recovery, not a second direct EndMatch call.
        // OutroCompleted legitimately advances native mode authority itself.
        if(!complete&&recovery.Elapsed(GetTickCount64())<cfgoutro::NativeGraceMs&&
           !cfgoutro::InactiveSequence(r_,registry,plan.module,plan.world,owner))return false;
        recovery.settings=owner.settings;
        if(complete){recovery.finished=true;bundleReport.outroRecoveryStage=2;}
        else {recovery.skipped=true;bundleReport.outroRecoveryStage=1;}
        ++bundleReport.outroRecoveryAttempts;
        const bool returned=Invoke(owner.settings.object,function,nullptr);
        if(returned){++bundleReport.outroRecoveryReturns;bundleReport.outroRecoveryFailure=0;}
        else recovery.finished=true;
        return returned;
    }
    bool CompleteMatch(){
        if(HostedRoutineActive())return false; // native Hunt owns hosted match outcome/stats
        bundleReport.completionFailure=1;
        std::uintptr_t function=0,owner=0;cfgnames::Words phase;cfgnames::Text text;
        cfgnames::Observation<Reader> names(r_,plan.module);
        if(!cfgjason::SameSlot(r_,context.objectArray,plan.objects[2]))return false;
        std::uintptr_t human=0;Known(0,human);
        bundleReport.completionFailure=2;
        if(bundleCompletion.terminalMask!=((1u<<(bundleReport.bots+1))-1))return false;
        bundleReport.completionFailure=3;
        if(bundleCompletion.dispatched||!FeatureFunction(cfgescape::disk::EndMatch,function,owner)||
           !cfgspawn::ObjectIs(r_,plan.mode,owner)||
           !cfgobserver::Same(r_,function+0xB0,plan.module+cfgescape::disk::EndMatchExecutor)||
           !cfgbundle::Code(r_,plan.module,cfgescape::disk::EndMatchExecutor,cfgescape::disk::EndMatchCode))return false;
        bundleReport.completionFailure=4;
        if(!r_.Read(plan.mode+cfgescape::disk::MatchState,phase)||!names.Decode(phase,text))return false;
        bundleReport.completionPhase={};const auto label=text.View();
        for(std::size_t i=0;i<label.size()&&i+1<bundleReport.completionPhase.size();++i)
            bundleReport.completionPhase[i]=label[i]<=127?static_cast<char>(label[i]):'?';
        // Donor offline completion advances native WaitingPostMatchOutro with
        // EndMatch. Hosted progression is deliberately NOT enabled here.
        // One owned dispatch only; never replay during PostMatchOutro/credits.
        if(!cfgescape::OfflineCompletionPhase(label))return false;
        if(label==u"WaitingPostMatchOutro"){
            std::uintptr_t vtable=0;
            if(!r_.Read(plan.mode,vtable)||vtable<=plan.module||vtable>=plan.module+0x3000000||
               !cfgobserver::Same(r_,vtable+0x7E8,plan.module+cfgescape::disk::EndMatchNative)||
               !cfgtommyend::EndBodyCurrent(r_,plan.module))return false;
        }
        bundleReport.completionFailure=5;
        if(!Context()||!NativeTerminal()||!cfgescape::Same(r_,plan.mode+cfgescape::disk::MatchState,phase))return false;
        // Drain our owned camera selection before optional diagnostics and
        // before consuming native completion. A bounded cleanup refusal must
        // leave the ending eligible for a fresh, independently budgeted retry.
        bundleReport.completionFailure=6;
        if(!Abandon())return false;
        // Small read-only donor-parity snapshot. Native PlayOutro consumes
        // GameState's current killer PlayerState/loadout, not the AI pawn's
        // appearance alone. Do not fabricate/copy a soft class or override
        // a native scene owner without a proved CFG setter.
        bundleReport.endingMetadataStage=1;
        std::uintptr_t nativeKiller=0,nativeState=0,jasonState=0,skin=0;
        if(r_.Read(spawnAssets.gameState+cfgprimary::CurrentKiller,nativeKiller)&&
           r_.Read(spawnAssets.gameState+cfgprimary::CurrentKillerState,nativeState)&&
           r_.Read(bundleJason.pawn.object+cfgescape::disk::PlayerState,jasonState)){
            bundleReport.endingNativeKiller=nativeKiller;bundleReport.endingNativeKillerState=nativeState;
            bundleReport.endingJasonState=jasonState;bundleReport.endingMetadataStage=2;
            // Exact SCPlayerState property proof: Inspect-CfgEndingMetadata.ps1.
            // Never read the donor's +0x7A0 offset in CFG.
            std::uintptr_t stateType=0;
            cfgprobe::lifetime::SlotObservation stateSlot{};
            if(nativeState&&r_.Read(plan.module+cfgescape::disk::SCPlayerStateReflection,stateType)&&
               cfgobserver::Same(r_,plan.module+cfgescape::disk::SCPlayerStateConstructor,stateType)&&
               cfgspawn::ObjectSlot(r_,context.objectArray,nativeState,stateSlot)&&stateSlot.HasAssignedSerial()&&
               cfgspawn::ObjectIs(r_,nativeState,stateType)&&r_.Read(nativeState+0x7E8,skin)){
                bundleReport.endingSkin=skin;bundleReport.endingMetadataStage=3;
            }
        }
        // A native ending may synchronously destroy game objects. Consume the
        // one-shot before dispatch, including exceptions; do not synthesize HUD.
        bundleReport.completionFailure=7;
        if(calls_>=cfgbundle::CallCap)return false;
        bundleCompletion.dispatched=true;
        ++bundleReport.endMatchAttempts;
        const bool returned=Invoke(plan.mode,function,nullptr);
        cfgclosedtrace::Mark(returned?"ending-endmatch-returned":"ending-endmatch-refused",plan.mode);
        if(returned){++bundleReport.endMatchReturns;bundleReport.completionFailure=0;bundlePostMatchWatch.Begin(GetTickCount64());}return returned;
    }
#ifdef CFG_RETAINED_GAMEPLAY_BUNDLE
#include "BundleRetentionRuntimeMembers.hpp"
#include "SoftClassRetentionRuntimeMembers.hpp"
#endif
#include "BundleNativeCallRuntimeMembers.hpp"
#include "DonorJasonHuntingNativeMembers.hpp"
#include "DonorJasonMovementNativeMembers.hpp"
    bool Location(std::uintptr_t pawn,cfgspawn::Vec3& out){
        cfgprobe::lifetime::SlotObservation fresh{};
        return cfgspawn::ObjectSlot(r_,context.objectArray,spawnAssets.functions[2],fresh)&&fresh==spawnAssets.functionSlots[2]&&
            cfgobserver::Same(r_,plan.module+cfgjason::disk::Functions[2].cache,spawnAssets.functions[2])&&
            cfgobserver::Same(r_,spawnAssets.functions[2]+0x20,spawnAssets.classes[2])&&
            Invoke(pawn,spawnAssets.functions[2],&out)&&cfgspawn::Finite(out);
    }
    bool Distance(std::uintptr_t target,float& squared){cfgspawn::Vec3 j{},t{};return Distance(target,squared,j,t);}
    bool Distance(std::uintptr_t target,float& squared,cfgspawn::Vec3& j,cfgspawn::Vec3& t){
        if(!Location(bundleJason.pawn.object,j)||!Location(target,t))return false;
        const auto x=j.x-t.x,y=j.y-t.y,z=j.z-t.z;squared=x*x+y*y+z*z;return std::isfinite(squared);
    }
    // Lead point for the existing verified MoveToLocation binding (same packet
    // and proofs as the door-approach anchor). False = binding unavailable.
    bool MoveToPoint(const cfgspawn::Vec3& goal,unsigned& result){
        cfgtrap::MoveLocationParameters packet;packet.destination=goal;packet.acceptance=60.f;packet.allowPartial=1;packet.result=255;
        result=255;
        if(!CarCall(cfgtrap::disk::MoveToLocation,cfgtrap::disk::MoveToLocationExecutor,cfgtrap::disk::MoveToLocationCode,
            bundleJason.killerController.object,&packet))return false;
        result=packet.result;return cfgbundle::ClassifyMove(result)!=cfgbundle::MoveOutcome::Invalid;
    }
    bool Move(std::uintptr_t target){
        cfgprobe::lifetime::SlotObservation fresh{};cfgspawn::MoveParameters p;p.goal=target;
        bundleReport.lastMoveResult=255;
        if(!(cfgspawn::ObjectSlot(r_,context.objectArray,spawnAssets.functions[3],fresh)&&fresh==spawnAssets.functionSlots[3]&&
            cfgobserver::Same(r_,plan.module+cfgjason::disk::Functions[3].cache,spawnAssets.functions[3])&&
            cfgobserver::Same(r_,spawnAssets.functions[3]+0x20,spawnAssets.resources.classes[cfgspawn::BaseAI]))){
            bundleReport.resourceFailure=9;return false;
        }
        if(!Invoke(bundleJason.killerController.object,spawnAssets.functions[3],&p)||
           cfgbundle::ClassifyMove(p.result)==cfgbundle::MoveOutcome::Invalid){bundleReport.resourceFailure=10;return false;}
        bundleReport.lastMoveResult=p.result;
        if(p.result==0)++bundleReport.moveRefusals;
        return true; // native Failed is a completed path request, not a broken resource
    }
    template<class Binding> bool FeatureFunction(const Binding& binding,std::uintptr_t& function,std::uintptr_t& owner,std::uint16_t* nativeExtent=nullptr){
        function=owner=0;std::uintptr_t reflection=0,constructor=0;
        if(!r_.Read(plan.module+binding.ownerReflection,reflection)||!r_.Read(plan.module+binding.ownerConstructor,constructor)||
            !(owner=cfgprobe::MatchingClassCache(reflection,constructor))||!cfgbundle::Code(r_,plan.module,binding.factory,binding.code)||
            !r_.Read(plan.module+binding.cache,function))return false;
        cfgprobe::lifetime::SlotObservation ownerSlot{},functionSlot{},fresh{};
        if(!cfgspawn::ObjectSlot(r_,context.objectArray,owner,ownerSlot))return false;
#ifdef CFG_RETAINED_GAMEPLAY_BUNDLE
        if(!RetainPrimaryObjects(r_,std::array<std::uintptr_t,1>{owner}))return false;
#endif
        if(!function){
            if(!NativeCallAllowed())return false;
#ifdef CFG_RETAINED_GAMEPLAY_BUNDLE
            if(!BeginBundleNative(1))return false;
#endif
            ++calls_;
            __try{function=reinterpret_cast<std::uintptr_t(__fastcall*)()>(plan.module+binding.factory)();}
            __except(EXCEPTION_EXECUTE_HANDLER){exception_=true;
#ifdef CFG_RETAINED_GAMEPLAY_BUNDLE
                BundleRetentionFailed();
#endif
                return false;}
#ifdef CFG_RETAINED_GAMEPLAY_BUNDLE
            if(!RetainBundleReturnedObject(function))return false;
#endif
            if(!function||!Context())return false;
        }
        std::uint16_t parameterBytes=0;
        if(!cfgspawn::ObjectSlot(r_,context.objectArray,function,functionSlot)||
            !r_.Read(function+cfgreflection::arraydisk::FunctionParameterBytes,parameterBytes))return false;
#ifdef CFG_RETAINED_GAMEPLAY_BUNDLE
        if(!RetainPrimaryObjects(r_,std::array<std::uintptr_t,1>{function}))return false;
#endif
        if(nativeExtent)*nativeExtent=parameterBytes; // existing read only, including a rejected frame
        return cfgbundle::NativeFrame(parameterBytes,binding.nativeBytes,binding.storageBytes)&&
            cfgobserver::Same(r_,function+0x20,owner)&&cfgobserver::Same(r_,plan.module+binding.cache,function)&&
            cfgspawn::ObjectSlot(r_,context.objectArray,owner,fresh)&&fresh==ownerSlot&&
            cfgobserver::Same(r_,plan.module+binding.ownerReflection,owner)&&cfgobserver::Same(r_,plan.module+binding.ownerConstructor,owner);
    }
    bool ConvergenceInvoke(unsigned id,std::uintptr_t object,void* packet,bool actor=false){
        if(id>=bundleConvergencePrepared||id>=ConvergenceFunctions.size())return false;
        const auto& resource=bundleConvergenceResources[id];std::uintptr_t function=0,owner=0;
        cfgprobe::lifetime::SlotObservation f{},o{};
        cfgprobe::lifetime::ArrayHeader registry;
        if(!cfgprobe::lifetime::ReadArrayHeader(r_,plan.module+cfgprobe::lifetime::ObjectArrayRva,registry)||
            registry.data!=context.objectArray.data||registry.count<context.objectArray.count)return false;
        context.objectArray=registry; // growth only; never use retired registry storage after a setter
        if(!FeatureFunction(ConvergenceFunctions[id],function,owner)||function!=resource.function||owner!=resource.owner||
            !cfgspawn::ObjectSlot(r_,context.objectArray,function,f)||!(f==resource.functionSlot)||
            !cfgspawn::ObjectSlot(r_,context.objectArray,owner,o)||!(o==resource.ownerSlot)||
            !ConvergenceExecutor(id,function))return false;
        return Invoke(object,function,packet,actor);
    }
    bool ConvergenceExecutor(unsigned id,std::uintptr_t function){
        using namespace cfgconvergence::disk;
        const auto address=function+FunctionExecutor;
        switch(id){
        case 0:return cfgobserver::Same(r_,address,plan.module+Conv_StringToNameExecutor)&&cfgbundle::Code(r_,plan.module,Conv_StringToNameExecutor,Conv_StringToNameExecutorCode);
        case 1:return cfgobserver::Same(r_,address,plan.module+GetValueAsObjectExecutor)&&cfgbundle::Code(r_,plan.module,GetValueAsObjectExecutor,GetValueAsObjectExecutorCode);
        case 2:return cfgobserver::Same(r_,address,plan.module+GetValueAsBoolExecutor)&&cfgbundle::Code(r_,plan.module,GetValueAsBoolExecutor,GetValueAsBoolExecutorCode);
        case 3:return cfgobserver::Same(r_,address,plan.module+SetValueAsObjectExecutor)&&cfgbundle::Code(r_,plan.module,SetValueAsObjectExecutor,SetValueAsObjectExecutorCode);
        case 4:return cfgobserver::Same(r_,address,plan.module+SetValueAsBoolExecutor)&&cfgbundle::Code(r_,plan.module,SetValueAsBoolExecutor,SetValueAsBoolExecutorCode);
        case 5:return cfgobserver::Same(r_,address,plan.module+GetCurrentWeaponExecutor)&&cfgbundle::Code(r_,plan.module,GetCurrentWeaponExecutor,GetCurrentWeaponExecutorCode);
        default:return false;
        }
    }
    bool NativeClass(std::uintptr_t reflection,std::uintptr_t constructor,std::uintptr_t& out){
        std::uintptr_t a=0,b=0;cfgprobe::lifetime::SlotObservation slot{};
        out=0;return r_.Read(plan.module+reflection,a)&&r_.Read(plan.module+constructor,b)&&
            (out=cfgprobe::MatchingClassCache(a,b))&&cfgspawn::ObjectSlot(r_,context.objectArray,out,slot);
    }
    bool TypedFeature(std::uintptr_t object,unsigned id,bool assigned=true){
        if(id>=cfgfeature::disk::Classes.size())return false;
        const auto& binding=cfgfeature::disk::Classes[id];std::uintptr_t native=0;
        cfgprobe::lifetime::SlotObservation slot{},fresh{};
        return NativeClass(binding.reflection,binding.constructor,native)&&
            cfgspawn::ObjectSlot(r_,context.objectArray,object,slot)&&(!assigned||slot.HasAssignedSerial())&&
            cfgspawn::ObjectIs(r_,object,native)&&cfgspawn::ObjectSlot(r_,context.objectArray,object,fresh)&&fresh==slot;
    }
#include "HostedMaskTriggerMembers.hpp"
#include "PamelaTriggerInventoryMembers.hpp"
#include "SweaterRuntimeMembers.hpp"
#include "JasonRunRuntimeMembers.hpp"
#include "KillRuntimeMembers.hpp"
#include "PamelaKillRuntimeMembers.hpp"
#include "KillHitDiagnosticMembers.hpp"
#include "KnifeRuntimeMembers.hpp"
    bool Blackboard(unsigned index,std::uintptr_t& pawn,std::uintptr_t& blackboard,cfgprobe::lifetime::SlotObservation& slot){
        pawn=blackboard=0;std::uintptr_t native=0;cfgprobe::lifetime::SlotObservation fresh{};
        bool alive=false;
        if(index>=bundleReport.bots||!Known(index+1,pawn)||!Alive(pawn,alive)||!alive)return false;
        const auto controller=bundleReport.roster[index].controller;
        return NativeClass(cfgconvergence::disk::BlackboardComponentReflection,cfgconvergence::disk::BlackboardComponentConstructor,native)&&
            r_.Read(controller+cfgconvergence::disk::ControllerBlackboard,blackboard)&&
            // As with Jason's owned interaction manager, this typed native
            // component is held by a positively identified live controller.
            // Native calls require repeated strong ownership, not invented IDs.
            cfgspawn::ObjectSlot(r_,context.objectArray,blackboard,slot)&&
            cfgspawn::ObjectIs(r_,blackboard,native)&&cfgobserver::Same(r_,blackboard+0x20,controller)&&
            cfgobserver::Same(r_,blackboard+0xE0,controller)&&
            cfgspawn::ObjectSlot(r_,context.objectArray,blackboard,fresh)&&fresh==slot&&
            cfgobserver::Same(r_,controller+cfgconvergence::disk::ControllerBlackboard,blackboard);
    }
    bool Weapon(std::uintptr_t weapon){
        std::uintptr_t native=0;cfgprobe::lifetime::SlotObservation slot{};
        return NativeClass(cfgconvergence::disk::SCWeaponReflection,cfgconvergence::disk::SCWeaponConstructor,native)&&
            cfgspawn::ObjectSlot(r_,context.objectArray,weapon,slot)&&slot.HasAssignedSerial()&&cfgspawn::ObjectIs(r_,weapon,native);
    }
    bool BlackboardClassesWarm(){
        if(bundleBlackboardClassIndex>=4)return false;
        struct Host {
            BundleAccess& a;unsigned index;
            const cfgconvergence::schemadisk::TypeBinding& Binding(){
                using namespace cfgconvergence::schemadisk;
                constexpr std::array<const TypeBinding*,4> bindings{{&BlackboardComponent,&BlackboardData,&BoolKey,&ObjectKey}};
                return *bindings[index];
            }
            bool Current(){
#ifdef CFG_H1_JASON
                if(HostedRoutineActive())return a.HostedContext(false);
#endif
                return GetCurrentThreadId()==plan.threadId&&cfgprimary::LookupSafe(a.r_,plan.module)&&
                    cfgobserver::Same(a.r_,plan.viewport+cfgprobe::ViewportWorld,plan.world)&&
                    cfgobserver::Same(a.r_,plan.world+cfgprobe::WorldNetDriver,std::uintptr_t{})&&
                    cfgobserver::Same(a.r_,plan.world+cfgprobe::WorldGameMode,plan.mode)&&
                    cfgjason::SameSlot(a.r_,a.context.objectArray,plan.objects[0])&&
                    cfgjason::SameSlot(a.r_,a.context.objectArray,bundleJason.pawn)&&
                    cfgjason::SameSlot(a.r_,a.context.objectArray,bundleJason.killerController);
            }
            bool Caches(std::uintptr_t& x,std::uintptr_t& y){const auto& b=Binding();return a.r_.Read(plan.module+b.reflection,x)&&a.r_.Read(plan.module+b.constructor,y);}
            bool InitializeClass(bool getter,std::uintptr_t& out){
                using namespace cfgconvergence::schemadisk;
                if(a.calls_>=cfgbundle::CallCap)return false;
                std::uintptr_t entry=Binding().factory;bool code=false;
                if(getter){
                    switch(index){
                    case 0:entry=BlackboardComponentConstructorGetter;code=cfgbundle::Code(a.r_,plan.module,entry,BlackboardComponentConstructorGetterCode);break;
                    case 1:entry=BlackboardDataConstructorGetter;code=cfgbundle::Code(a.r_,plan.module,entry,BlackboardDataConstructorGetterCode);break;
                    case 2:entry=BoolKeyConstructorGetter;code=cfgbundle::Code(a.r_,plan.module,entry,BoolKeyConstructorGetterCode);break;
                    case 3:entry=ObjectKeyConstructorGetter;code=cfgbundle::Code(a.r_,plan.module,entry,ObjectKeyConstructorGetterCode);break;
                    }
                }else code=cfgbundle::Code(a.r_,plan.module,entry,Binding().code);
                if(!code||!a.NativeCallAllowed())return false;
#ifdef CFG_RETAINED_GAMEPLAY_BUNDLE
                if(!a.RetainBundleActors(bundleJason.pawn.object)||!a.NativeCallAllowed()||!a.BeginBundleNative(1))return false;
#endif
                ++a.calls_;
                __try{out=reinterpret_cast<std::uintptr_t(__fastcall*)()>(plan.module+entry)();}
                __except(EXCEPTION_EXECUTE_HANDLER){a.exception_=true;
#ifdef CFG_RETAINED_GAMEPLAY_BUNDLE
                    a.BundleRetentionFailed();
#endif
                    return false;}
#ifdef CFG_RETAINED_GAMEPLAY_BUNDLE
                return a.RetainBundleReturnedObject(out);
#else
                return out!=0;
#endif
            }
        }host{*this,bundleBlackboardClassIndex};
        const auto state=bundleBlackboardClassWarm[bundleBlackboardClassIndex].Step(host);
        if(state==cfgkillerstart::BootstrapState::Ready)++bundleBlackboardClassIndex;
        else if(state==cfgkillerstart::BootstrapState::Refused){bundleBlackboardClassIndex=4;bundleBlackboardClassesRefused=true;bundleReport.convergenceSchemaFailure=1;}
        return true; // compiled metadata only, no asset/actor load or stacked work
    }
#include "ConvergenceTravelRuntimeMembers.hpp"
#include "LeanNearConvergenceTravelMembers.hpp"
#include "EmoteBrainRuntimeMembers.hpp"
#include "ConvergenceAxeRuntimeMembers.hpp"
#include "DifficultyRuntimeMembers.hpp"
#include "RBConfirmationRuntimeMembers.hpp"
    bool RBStep(std::uint64_t now,bool allowed){
        if(allowed&&bundleKill.policy.Exclusive(now)){
            cfgkill::Context death;
            allowed=Context()&&DeathContext(death)&&!death.active&&!bundleKill.policy.interactionAttempted;
        }
        Reader metadata;metadata.cap=cfgbundle::ReadCap;
        cfgrbemote::NativeAccess<Reader,BundleAccess> access(metadata,*this,plan.module,plan.controller,plan.objects[2]);
        bundleRBDispatch=true;const bool worked=bundleRBEmote.Step(access,now,allowed);bundleRBDispatch=false;
        bundleReport.reads+=metadata.reads;bundleReport.maxReads=(std::max)(bundleReport.maxReads,metadata.reads);return worked;
    }
    bool RBClose(bool allowed){
        cfgrbemote::NativeAccess<Reader,BundleAccess> access(r_,*this,plan.module,plan.controller,plan.objects[2]);
        return bundleRBEmote.Close(access,allowed);
    }
    bool HostedSoundBlipScope(const cfgsoundblip::Scope& scope){
#ifdef CFG_H1_JASON
        std::uintptr_t human=0;
        return HostedRoutineActive()&&NativeCallAllowed()&&scope.active&&scope.module==plan.module&&
            scope.world==plan.world&&scope.mode==plan.mode&&scope.viewport==plan.viewport&&
            ((scope.killer==bundleJason.pawn&&scope.controller==bundleJason.killerController)||
             (CFG_H262_F1_SOUND_BLIPS&&bundleAdditional.actor.pawn.HasAssignedSerial()&& // the F1 Jason clears its own blips online too, as offline does
              scope.killer==bundleAdditional.actor.pawn&&scope.controller==bundleAdditional.actor.killerController))&&
            scope.humanController==plan.controller&&Context()&&Jason()&&Known(0,human)&&human==scope.humanPawn;
#else
        (void)scope;return false;
#endif
    }
    bool SoundBlipPhase(bool release=false){
        if(release?!bundleSoundBlips.Owned():bundleSoundBlips.Attempted())return false;
        cfgsoundblip::Scope scope;
        std::uintptr_t human=0;
        if(Context()&&Jason()&&Known(0,human)&&
            r_.Read(bundleJason.pawn.object+cfgprobe::ObjectClass,scope.killerClass)&&
            r_.Read(bundleJason.killerController.object+cfgprobe::ObjectClass,scope.controllerClass)){
            scope.active=scope.offlineCounselor=bundleSchedule.active;
            scope.module=plan.module;scope.viewport=plan.viewport;scope.world=plan.world;scope.mode=plan.mode;
            scope.humanController=plan.controller;scope.humanPawn=human;
            scope.killer=bundleJason.pawn;scope.controller=bundleJason.killerController;
        }
        if(release)bundleSoundScope.active=false;
        else if(scope.active)bundleSoundScope=scope;
        cfgsoundblip::NativeAccess<Reader,BundleAccess> access(r_,*this,scope,
            cfgprimary::LookupSafe(r_,plan.module),GetCurrentThreadId()==plan.threadId);
        if(release)bundleSoundBlips.Release(access);else bundleSoundBlips.Apply(access);
        return true;
    }
    // PID1692: ConvergenceUnavailable=19 with no reason. Diagnostic only:
    // 1-3=warm resources/keys 4=sweater-trigger 5=blackboard-changed
    // 6=weapon-getter 7=weapon-not-held 8=publication-begin.
    bool ConvergenceRefuse(unsigned code){
        ++bundleConvergenceRefusals[code<bundleConvergenceRefusals.size()?code:0];
        if(bundleConvergence.active&&bundleConvergenceTraces<16){++bundleConvergenceTraces;
            cfgclosedtrace::Mark("convergence-refused",bundleJason.pawn.object,(static_cast<std::uint64_t>(code)<<32)|r_.reads);}
        return false;
    }
    bool Convergence(std::uint64_t now){
        if(cfgj21::ConvergenceEnabled&&DonorHeldDecisionInstalled())return false; // new B owns it only when enabled; otherwise exact accepted legacy path
        if(cfgj21::LeanEnabled&&DonorHeldDecisionInstalled()&&bundleConvergencePrepared>=ConvergenceFunctions.size()&&
           bundleReport.convergenceKeys>=cfgconvergence::KeyCount&&bundleBlackboardClassIndex>=4)return false; // accepted read-only warmup only; lean owns mutation
        J21Timing legacyTiming(cfgj21::LegacyConvergence);
        using namespace cfgconvergence;
        // Warm compiled API/name resources in the already-paced startup lane,
        // one resource per phase, before mask acquisition. No bot Blackboard
        // writes occur here. Waiting until pickup delayed the response by all
        // six API, nine key and four class warm-up phases on top of schema work.
        if(bundleConvergencePrepared<ConvergenceFunctions.size()){
            auto& resource=bundleConvergenceResources[bundleConvergencePrepared];
            if(!FeatureFunction(ConvergenceFunctions[bundleConvergencePrepared],resource.function,resource.owner)||
                !cfgspawn::ObjectSlot(r_,context.objectArray,resource.function,resource.functionSlot)||
                !cfgspawn::ObjectSlot(r_,context.objectArray,resource.owner,resource.ownerSlot))return ConvergenceRefuse(1);
            ++bundleConvergencePrepared;return true;
        }
        if(bundleReport.convergenceKeys<KeyCount){
            const auto key=bundleReport.convergenceKeys;const auto text=KeyText[key];unsigned length=0;
            while(text[length])++length;
            NamePacket packet;packet.input={reinterpret_cast<std::uintptr_t>(text),static_cast<int>(length+1),static_cast<int>(length+1)};
            const auto owner=bundleConvergenceResources[0].owner;std::uintptr_t cdo=0;cfgprobe::lifetime::SlotObservation cdoSlot{};
            if(!r_.Read(owner+cfgspawn::ClassDefaultObject,cdo)||!cfgobserver::Same(r_,cdo+0x10,owner)||
                !cfgspawn::ObjectSlot(r_,context.objectArray,cdo,cdoSlot)||!ConvergenceInvoke(0,cdo,&packet))return ConvergenceRefuse(2);
            cfgnames::Observation<Reader> names(r_,plan.module);cfgnames::Text decoded{};
            if(packet.result.number!=0||!names.Decode({packet.result.index,packet.result.number},decoded)||
                decoded.View()!=std::u16string_view(text,length))return ConvergenceRefuse(3);
            bundleConvergenceNames[key]=packet.result;++bundleReport.convergenceKeys;return true;
        }
        if(BlackboardClassesWarm())return true;
        if(bundleBlackboardClassesRefused)return true;
        // Donor primes helper melee through native GiveStartingItem/AddOrSwap.
        // Survey is finite/cold; mutation only starts after sweater acquisition.
        // Finish pending Blackboard publication first so these phases cannot
        // interrupt its independently budgeted readback transaction.
        if(!bundleConvergencePublication.Active()&&ConvergenceAxePhase(now))return true;
        if(!bundleConvergence.active){
            if(!bundleConvergence.ProbeDue(now))return true;++bundleReport.convergenceProbes;
            bool acquired=false;if(!HasPamelaTrigger(acquired))return ConvergenceRefuse(4);
            if(!acquired)return true;bundleConvergence.Acquired();bundleReport.convergenceActive=1;return true;
        }
        if(bundleConvergencePublication.Active()){
            if(BundleEmoteHeld(bundleConvergencePublication.Index()))return true;
            const auto index=bundleConvergencePublication.Index();const auto action=bundleConvergencePublication.CurrentAction();
            std::uintptr_t pawn=0,blackboard=0;cfgprobe::lifetime::SlotObservation slot{};
            if(!Blackboard(index,pawn,blackboard,slot)||!(slot==bundleConvergenceBlackboard)){
                bundleConvergencePublication.Reset();bundleConvergence.Retry(index,now);return ConvergenceRefuse(5);
            }
            auto& schema=bundleBlackboardSchemas[index];
            struct Host {
                BundleAccess& a;cfgconvergence::BlackboardSchema& schema;std::uintptr_t blackboard;
                bool Current(){return schema.Current(a.r_,a.context.objectArray);}
                bool SetObject(unsigned key,std::uintptr_t value){ObjectPacket p{bundleConvergenceNames[key],value};return schema.AcceptsObject(a.r_,key,value)&&a.ConvergenceInvoke(3,blackboard,&p);}
                bool GetObject(unsigned key,std::uintptr_t& value){ObjectPacket p{bundleConvergenceNames[key],0};if(!a.ConvergenceInvoke(1,blackboard,&p))return false;value=p.value;return true;}
                bool SetBool(unsigned key,unsigned value){BoolPacket p{bundleConvergenceNames[key]};p.value=static_cast<std::uint8_t>(value);return a.ConvergenceInvoke(4,blackboard,&p);}
                bool GetBool(unsigned key,unsigned& value){BoolPacket p{bundleConvergenceNames[key]};if(!a.ConvergenceInvoke(2,blackboard,&p))return false;value=p.value;return true;}
            }host{*this,schema,blackboard};
            const auto result=bundleConvergencePublication.Step(host);
            if(result==Publication::Result::Refused){
                ++bundleReport.convergenceReadbackRefused;
                bundleReport.convergenceFailureKey=bundleConvergencePublication.failureKey;
                bundleReport.convergenceExpectedBool=bundleConvergencePublication.expectedBool;
                bundleReport.convergenceActualBool=bundleConvergencePublication.actualBool;
                bundleReport.convergenceExpectedObject=bundleConvergencePublication.expectedObject;
                bundleReport.convergenceActualObject=bundleConvergencePublication.actualObject;
                bundleConvergencePublication.Reset();bundleConvergence.Retry(index,now);return true;
            }
            if(result==Publication::Result::Complete){
                bundleConvergence.Confirm(index,action,now);bundleConvergencePublication.Reset();
                if(action==Action::Publish){++bundleReport.convergencePublished;bundleConvergenceFearPending=index+1;}
                else ++bundleReport.convergenceCleared;
            }
            return true;
        }
        unsigned index=0;if(!bundleConvergence.Pick(now,bundleReport.bots,index))return true;
        if(BundleEmoteHeld(index))return true;
        if(bundleConvergence.bots[index].unsupported)return true;
        std::uintptr_t pawn=0,blackboard=0;cfgprobe::lifetime::SlotObservation slot{};
        if(!Blackboard(index,pawn,blackboard,slot))return true;
        auto& schema=bundleBlackboardSchemas[index];
        const auto schemaState=schema.Step(r_,context.objectArray,plan.module,blackboard,slot,bundleConvergenceNames);
        bundleReport.convergenceSchemaMask=schema.ObservedMask();bundleReport.convergenceSchemaBot=index;
        bundleReport.convergenceSchemaFailure=static_cast<unsigned>(schema.Why());
        if(schemaState==BlackboardSchema::State::Unavailable){bundleConvergence.Refuse(index);return true;}
        if(schemaState!=BlackboardSchema::State::Ready){bundleConvergence.Continue(index,now);return true;}
        // Blackboard may retain a dropped/swapped weapon. Only the native
        // current-weapon getter proves that this counselor is actually armed.
        ObjectPacket w{bundleConvergenceNames[cfgconvergence::Weapon],0};
        if(!cfgspawn::ObjectIs(r_,pawn,bundleConvergenceResources[5].owner)||!ConvergenceInvoke(5,pawn,&w.value,true))return ConvergenceRefuse(6);
        if(w.value&&!HeldWeapon(pawn,w.value)){bundleConvergence.Continue(index,now);return ConvergenceRefuse(7);}
        const auto action=bundleConvergence.Decide(index,true,w.value!=0,now);
        if(!w.value){++bundleReport.convergenceUnarmed;bundleConvergenceAxes.RequestRearm(index,now);}
        if(action==Action::None)return true;
        // A fresh guarded callback publishes/readbacks one group immediately;
        // native BT changes between groups never become "unsupported schema".
        bundleConvergenceBlackboard=slot;
        return bundleConvergencePublication.Begin(index,action,schema.Mask(),bundleJason.pawn.object,w.value)||ConvergenceRefuse(8);
    }
    bool CabinBoundsFunction(std::uintptr_t& function){
        std::uintptr_t owner=0;
        return FeatureFunction(cfgcabin::disk::GetActorBounds,function,owner)&&
            cfgobserver::Same(r_,function+0xB0,plan.module+cfgcabin::disk::Executor)&&
            cfgbundle::Code(r_,plan.module,cfgcabin::disk::Executor,cfgcabin::disk::ExecutorCode)&&
            cfgbundle::Code(r_,plan.module,cfgcabin::disk::Native,cfgcabin::disk::NativeCode);
    }
    bool CabinBoundsPhase(std::uint64_t now){
#ifdef CFG_H1_JASON
        HostedLaneEnter(cfgh1::Lane::Roof);
#endif

        J21Timing timing(cfgj21::Cabin);
        if(bundleHelpers.Active()||now<bundleCabinsNext||bundleCabins.cache.stage==cfgcabin::Stage::Ready||
            bundleCabins.cache.stage==cfgcabin::Stage::Refused)return
#ifdef CFG_H1_JASON
 HostedLaneRefuse(cfgh1::Lane::Roof,1)
#else
 false
#endif
;
        bundleCabinsNext=now+250;
        struct Host {
            BundleAccess& a;
            bool Scope(){return a.Context();}
            unsigned PrepareBounds(){std::uintptr_t function=0;return a.CabinBoundsFunction(function)?1u:0u;}
            bool ReadBounds(std::uintptr_t cabin,cfgcabin::Packet& packet){
                std::uintptr_t function=0;return a.CabinBoundsFunction(function)&&a.Invoke(cabin,function,&packet);
            }
        }host{*this};
        bundleCabins.Step(r_,context.objectArray,plan.module,plan.world,host);return true;
    }
    bool ExteriorLanding(cfgspawn::Vec3 point){
        return bundleCabins.Allows(r_,plan.world,point);
    }
    bool TeleportFunction(std::uintptr_t& function){
        std::uintptr_t owner=0;
        return FeatureFunction(BundleTeleport,function,owner)&&owner==spawnAssets.classes[2]&&
            cfgobserver::Same(r_,function+0xB0,plan.module+cfgteleport::disk::Thunk)&&
            cfgbundle::Code(r_,plan.module,cfgteleport::disk::Thunk,cfgteleport::disk::ThunkCode)&&
            cfgbundle::Code(r_,plan.module,cfgteleport::disk::Implementation,cfgteleport::disk::ImplementationCode);
    }
#include "CounselorMorphRuntimeMembers.hpp"
#include "PoliceExitRuntimeMembers.hpp"
#define CarStopBeforeRoad NativeCarStopBeforeRoad
#define CarInteraction NativeCarInteraction
#include "VehicleActionRuntimeMembers.hpp"
#undef CarInteraction
#undef CarStopBeforeRoad
#include "DriverNavigationRuntimeMembers.hpp"
#include "StuckCarRuntimeMembers.hpp"
#include "OnFootBurstRuntimeMembers.hpp"
#include "FootCarObstacleRuntimeMembers.hpp"
#include "HidingRuntimeMembers.hpp"
#include "VehicleRoadRuntimeMembers.hpp"
#include "VehicleProgressRuntimeMembers.hpp"
#include "CarPendingActionRuntimeMembers.hpp"
#include "HeldRecoveryRuntimeMembers.hpp"
#include "ChaseRecoveryRuntimeMembers.hpp"
    bool CarActorBindings(){
#if defined(CFG_H1_JASON) && defined(CFG_H1_OBSERVE)
        if(!HostedRoutineActive())return NativeBindings();
        if(!(bundleJason.pawn==hostedAdoptionReceipt.pawn)||
           !(bundleJason.killerController==hostedAdoptionReceipt.controller)||
           !cfghosted::adoption::Current(r_,context.objectArray,hostedAdoptionReceipt,plan)){
            ++hostedH1Report.carAdmissionRefusals[0];return false;
        }
        if(HostedCarBindings())return true;
        ++hostedH1Report.carAdmissionRefusals[1];return false;
#else
        return NativeBindings();
#endif
    }
    bool HostedCarBindings(){
        return cfgbundle::Code(r_,plan.module,cfghunter::disk::Stunned,cfghunter::disk::StunnedBytes)&&
            cfgbundle::Code(r_,plan.module,cfghunter::disk::Attacking,cfghunter::disk::AttackingBytes);
    }
    bool CarPursuit(std::uint64_t now){
        struct Continuation {
            std::uint64_t now;
            ~Continuation(){bundleSchedule.vehicleAt=now+cfgvehicleaction::ContinuationDelay(bundleVehicleActions,bundleCarActionPending.active||bundleRoadMorph.stage!=0,now);}
        } continuation{now};
        ++bundleReport.carChecks;bundleReport.carStage=1;
        if(bundleVehicleActions.stopPending){
            std::uintptr_t manager=0,locked=0,held=0;bool stunned=false,attacking=false,living=false;
            if(!Context()||!Jason()||!Manager()||!r_.Read(bundleJason.pawn.object+cfghunter::disk::Manager,manager)||
               !r_.Read(manager+cfgvehicleinteraction::disk::LockedComponent,locked)||!Held(held)||
               !Alive(bundleJason.pawn.object,living)||!NativeBool(cfghunter::disk::Stunned,stunned)||
               !NativeBool(cfghunter::disk::Attacking,attacking))return false;
            if(locked||held||stunned||attacking||!living){bundleVehicleActions.stopPending=false;bundleVehicleActions.morphStopPending=false;return true;}
            if(bundleVehicleActions.morphStopPending){cfgvehicleoccupancy::Proof proof{};
                if(!CarOccupancy(bundleVehicleActions.car,proof)||proof.result!=cfgvehicleoccupancy::Result::Occupied||
                   proof.driver!=bundleVehicleActions.driver||!CarOccupancyCurrent(proof)){bundleVehicleActions.stopPending=false;bundleVehicleActions.morphStopPending=false;return true;}}
            bundleVehicleActions.stopPending=false;bundleVehicleActions.morphStopPending=false;
            bundleReport.carStage=14;
            return CarCall(cfgtrap::disk::StopMovement,cfgtrap::disk::StopMovementExecutor,cfgtrap::disk::StopMovementCode,bundleJason.killerController.object,nullptr);
        }
        if(bundleCarActionPending.active){
            const auto pending=bundleCarActionPending;bundleCarActionPending.active=false;
            if(pending.speedChange){
                // Outer gameplay Context/Jason proofs are fresh in this slice.
                // Re-observe the typed native occupancy before the optional
                // movement latch; never pay the acquisition's native debt too.
                cfgvehicleoccupancy::Proof proof{};
                if(!cfgjason::SameSlot(r_,context.objectArray,pending.carSlot)||
                   !cfgjason::SameSlot(r_,context.objectArray,pending.driverSlot)||
                   !CarSource(pending.source,pending.car)||!CarOccupancy(pending.car,proof)||
                   proof.result!=cfgvehicleoccupancy::Result::Occupied||proof.driver!=pending.driver||
                   !CarOccupancyCurrent(proof)||!CarBoost(true,true))++bundleVehicleActions.refusals;
                else if(pending.moving)bundleCarPursuit.Renew(now);
                return true;
            }
            return CarPendingAction(pending,now); // one dedicated action callback
        }
        std::uintptr_t vehicleClass=0,constructor=0,car=0,driver=0;
        const auto carIndex=bundleCarPursuit.Active(now)&&bundleCarActionPending.car?
            (bundleCarActionPending.source==cfgvehicle::disk::Car2?1u:0u):bundleCarIndex++%2;
        const auto offset=carIndex?cfgvehicle::disk::Car2:cfgvehicle::disk::Car4;
        if(!r_.Read(plan.module+cfgvehicle::disk::VehicleReflection,vehicleClass)||
            !r_.Read(plan.module+cfgvehicle::disk::VehicleConstructor,constructor)||
            !cfgprobe::MatchingClassCache(vehicleClass,constructor)||!CarResolve(offset,car))return false;
        if(!car){std::uintptr_t again=0;if(!CarResolve(offset,again)||again)return false;CarRetire(now);bundleReport.carStage=2;return true;}
        cfgprobe::lifetime::SlotObservation carSlot{},fresh{};
        if(!cfgspawn::ObjectSlot(r_,context.objectArray,car,carSlot)||!carSlot.HasAssignedSerial()||
            !cfgspawn::ObjectIs(r_,car,vehicleClass)||!r_.Read(car+cfgvehicle::disk::Driver,driver))return false;
        if(bundleVehicleProgress.cars[carIndex].Ignored(car,carSlot.serial,now))return true;
        cfgvehicleoccupancy::Proof occupancy{};
        if(!CarOccupancy(car,occupancy))return false;
        if(occupancy.result!=cfgvehicleoccupancy::Result::Occupied){
            bool held=false;if(!CarDrain(car,occupancy,now,held))return false;
            if(!held)CarVacant(car,now);bundleReport.carStage=3;return true;}
        driver=occupancy.driver;
        bundleReport.carStage=4;++bundleReport.carDriverChecks;
        cfgprobe::lifetime::SlotObservation driverSlot{};
        cfgescape::State driverState=cfgescape::State::Unknown;
        if(!cfgspawn::ObjectSlot(r_,context.objectArray,driver,driverSlot)||!driverSlot.HasAssignedSerial()||
           !cfgescape::Counselor(r_,context.objectArray,plan.module,driver,bundleCounselorClass,driverState))return false;
        if(!cfgescape::Driving(driverState)){CarVacant(car,now);bundleReport.carStage=5;return true;}
        ++bundleReport.carDriverLiving;bundleReport.carStage=6;
        if(bundleVehicleActions.KnownVacant(car,driver))return CarInteraction(car,driver,{},false,now,true);
        const auto refuse=[&](unsigned clause){
#if defined(CFG_H1_JASON) && defined(CFG_H1_OBSERVE)
            if(HostedRoutineActive())++hostedH1Report.carAdmissionRefusals[clause];
#else
            (void)clause;
#endif
            return false;
        };
        // Hosted car movement uses direct predicates, independent of melee vtable pins.
        if(!CarActorBindings())return false;
        if(!Manager()){
#if defined(CFG_H1_JASON) && defined(CFG_H1_OBSERVE)
            if(HostedRoutineActive()){
                hostedH1Report.carManagerClause=bundleManagerLastClause;
                hostedH1Report.carManagerReads=bundleManagerLastReads;
                hostedH1Report.carManagerCap=bundleManagerLastCap;
            }
#endif
            return refuse(2);
        }
        std::uintptr_t held=0;if(!Held(held))return refuse(3);if(held){++bundleCarPursuit.held;return true;}
        bool stunned=false,attacking=false;
        if(!NativeBool(cfghunter::disk::Stunned,stunned))return refuse(4);
        if(!NativeBool(cfghunter::disk::Attacking,attacking))return refuse(5);
        if(stunned||attacking){++bundleCarPursuit.busy;return true;}
        cfgspawn::Vec3 velocity{},position{},lead{};float squared=0;
        bundleReport.carStage=7;
        if(!Location(car,position))return false;
        bundleRoadTrace.Sample(car,carSlot.serial,{position.x,position.y,position.z},now);
        if(bundleCarPursuit.samples[carIndex].Observe(car,driver,now,position,velocity))++bundleCarPursuit.positionSamples;
        else {
            std::uintptr_t velocityFunction=0,owner=0;
            if(!FeatureFunction(cfgvehicle::disk::Velocity,velocityFunction,owner)||!Invoke(car,velocityFunction,&velocity))return false;
            ++bundleCarPursuit.velocityFallbacks;
        }
        // Native velocity may report zero on an occupied moving car (donor
        // uses position deltas). A stopped occupied car still needs pursuit.
        const bool moving=cfgvehicle::Lead(position,velocity,lead);
        if(!moving){lead=position;++bundleCarPursuit.stationary;}
        if(!bundleCarSpeed.active&&now>=bundleCarSpeedRetryAt){
            if(!CarOccupancyCurrent(occupancy))return false;
            bundleCarSpeedRetryAt=now+1379;
            bundleCarActionPending={true,moving,offset,car,driver,carSlot,driverSlot,position,velocity,true};
            bundleCarPursuit.Renew(now);
            return true; // optional boost has a separate freshly guarded slice
        }
        bundleReport.carStage=8;
        cfgspawn::Vec3 jasonPosition{};
        if(!Location(bundleJason.pawn.object,jasonPosition))return false;
        const auto distanceX=jasonPosition.x-position.x,distanceY=jasonPosition.y-position.y,distanceZ=jasonPosition.z-position.z;
        squared=distanceX*distanceX+distanceY*distanceY+distanceZ*distanceZ;
        if(!std::isfinite(squared)||!cfgspawn::ObjectSlot(r_,context.objectArray,car,fresh)||!(fresh==carSlot)||
            !CarSource(offset,car)||!CarOccupancyCurrent(occupancy))return false;
        // Acquire native stop/extraction ownership BEFORE exit/road priority.
        // Pending road phases repeat this check; a queued moving sample is not
        // authority to teleport after a successful hood stop in between slices.
        bool stopOwns=false;
        if(!CarStopBeforeRoad(occupancy,position,jasonPosition,velocity,now,stopOwns))return false;
        if(stopOwns){
            bundleRoadMorph={};
            if(bundleVehicleActions.car!=car){bundleCarPursuit.Renew(now);return true;}
            bundleCarActionPending={true,false,offset,car,driver,carSlot,driverSlot,position,velocity};
            bundleCarPursuit.Renew(now);return true;
        }
        // Escape priority (user): a MOVING occupied car is never abandoned for
        // another counselor. Lack of closing escalates to the road-ahead Morph
        // below instead; the donor back-off remains only for a stopped car
        // Jason cannot reach.
        const bool carStalled=bundleVehicleProgress.cars[carIndex].Observe(car,carSlot.serial,driver,driverSlot.serial,std::sqrt(squared),now);
        if(carStalled&&!moving){
            bundleVehicleProgress.pending=carIndex+1;bundleVehicleProgress.cursor=0;bundleVehicleProgress.next=now;
            return true; // fresh callback must prove an alternative before backoff
        }
        const bool prolongedTail=bundleCarIntercept.Observe(car,driver,now,moving,position,velocity,jasonPosition);
        cfgroad::Candidate exitHint;const float planarSpeed=std::sqrt(velocity.x*velocity.x+velocity.y*velocity.y);
        const bool exitPriority=moving&&planarSpeed>=100.f&&bundleVehicleActions.phase==0&&!bundleVehicleActions.extractionCommitted&&
            bundleRoads.ExitCandidate({position.x,position.y,position.z},{velocity.x/planarSpeed,velocity.y/planarSpeed,0},exitHint,car);
        const bool escalate=carStalled&&moving&&planarSpeed>=100.f&&bundleVehicleActions.phase==0&&!bundleVehicleActions.extractionCommitted&&now>=bundleRoadAttemptAt;
        if(escalate&&bundleEscalationTraces<16){++bundleEscalationTraces;cfgclosedtrace::Mark("car-stalled-road-escalation",car,static_cast<std::uint64_t>(std::sqrt(squared)));}
        if(((exitPriority&&now>=bundleRoadAttemptAt)||escalate||(cfgroad::RequestDue(now,bundleCarIntercept.next,bundleRoadAttemptAt,moving,bundleVehicleActions.phase,squared,prolongedTail)))&&
           bundleRoads.stage==cfgroad::Stage::Ready&&
           bundleCabins.cache.stage==cfgcabin::Stage::Ready){
            bundleRoadAttemptAt=now+cfgroad::FailedRoadRetry;
            bundleRoadMorph={1,car,driver,offset,position,velocity,{},{}};
            bundleRoadMorph.prolongedTail=prolongedTail;
            bundleRoadMorph.exitPriority=exitPriority;
            bundleCarPursuit.Renew(now);
            return true; // road/projection/write own fresh callbacks, not this observation's debt
        }
        bundleReport.carStage=11;
        if(!Context()||!Jason()||!cfgspawn::ObjectSlot(r_,context.objectArray,car,fresh)||!(fresh==carSlot)||
           !CarSource(offset,car)||!CarOccupancyCurrent(occupancy)||
           !cfgjason::SameSlot(r_,context.objectArray,driverSlot)||
           !cfgescape::Counselor(r_,context.objectArray,plan.module,driver,bundleCounselorClass,driverState)||
           !cfgescape::Driving(driverState))return false;
        bundleCarActionPending={true,moving,offset,car,driver,carSlot,driverSlot,position,velocity};
        bundleCarPursuit.Renew(now);
        return true; // fresh action proof runs on next vehicle slice
    }
    bool DoorState(const cfgdoor::Selected& selected,std::uintptr_t doorClass,cfgdoor::BreakState& out){
        std::uintptr_t type=0;
        return cfgdoor::ObserveBreak(r_,context.objectArray,bundleJason.pawn.object,selected,doorClass,out)&&
            NativeClass(cfgdoor::breakdisk::BarricadeReflection,cfgdoor::breakdisk::BarricadeConstructor,type)&&
            cfgdoor::ObserveBarricade(r_,context.objectArray,selected,type,out.barricaded);
    }
    cfgdoor::Result DoorRecovery(std::uint64_t now,float targetDistance,std::uintptr_t chaseTarget=0){
        HostedLaneEnter(cfgh1::Lane::Door);
        const auto result=DoorRecoveryBody(now,targetDistance,chaseTarget);
        // Unavailable is the existing door selector/validation refusal result;
        // no door candidate is still reported by the established door counters.
        if(result==cfgdoor::Result::Unavailable)HostedLaneRefuse(cfgh1::Lane::Door,1);
        return result;
    }
    unsigned hostedDoorClause_=1;
    cfgdoor::Result DoorRecoveryBody(std::uint64_t now,float targetDistance,std::uintptr_t chaseTarget){
        using cfgdoor::Result;
        if(bundleDoorRecovery.breaking){
            if(now<bundleDoorRecovery.next)return Result::Acted;
            bundleDoorRecovery.ObserveBreak(now);
            std::uintptr_t doorClass=0;cfgdoor::BreakState state;
            ++bundleReport.doorBreakObservations;
            std::uintptr_t target=0;
            if(!cfgdoor::ObserveBreakTarget(r_,bundleJason.pawn.object,target))return Result::Unavailable;
            if(target!=bundleDoorBreakSelection.door){
                // Native completion can retire the door object itself. Clear
                // ONLY our local latch when Jason's native target has gone;
                // do not dereference a retired actor or adopt a replacement.
                bundleReport.doorBreakTarget=target;bundleReport.doorBreakOwner=0;
                ++bundleReport.doorBreakEnds;bundleDoorRecovery.EndBreak(now,false);
                bundleDoorBreakSelection={};return Result::Acted;
            }
            if(!NativeClass(cfgdoor::disk::SCDoorReflection,cfgdoor::disk::SCDoorConstructor,doorClass)||
               !DoorState(bundleDoorBreakSelection,doorClass,state))return Result::Unavailable;
            bundleReport.doorBreakTarget=state.target;bundleReport.doorBreakOwner=state.killer;
            if(state.Owned(bundleJason.pawn.object,bundleDoorBreakSelection.door)){
                bundleDoorRecovery.ObserveBreak(now);return Result::Acted;
            }
            // Never abort a live native breakdown with the melee release,
            // overwrite its target or clear a foreign manager lock.
            if(state.target==bundleDoorBreakSelection.door&&state.killer==bundleJason.pawn.object){
                bundleDoorRecovery.ObserveBreak(now);return Result::Acted;
            }
            ++bundleReport.doorBreakEnds;bundleDoorRecovery.EndBreak(now,state.open||state.destroyed);
            bundleDoorBreakSelection={};return Result::Acted;
        }
        if(bundleDoorRecovery.SampleDue(now)){
            cfgspawn::Vec3 position{};
            if(!Location(bundleJason.pawn.object,position))return Result::Unavailable;
            bundleDoorRecovery.Sample(now,position,targetDistance);
        }
        if(now<bundleDoorRecovery.next){if(bundleDoorRecovery.phase){bundleSchedule.huntAt=bundleDoorRecovery.next;return Result::Acted;}return Result::None;}
        ++bundleReport.doorChecks;
        std::uintptr_t doorClass=0,componentClass=0;
        if(!NativeClass(cfgdoor::disk::SCDoorReflection,cfgdoor::disk::SCDoorConstructor,doorClass)||
           !NativeClass(cfgdoor::disk::ILLInteractableComponentReflection,cfgdoor::disk::ILLInteractableComponentConstructor,componentClass))return Result::Unavailable;
        cfgdoor::Selected selected{},fresh{};
        if(!cfgdoor::ObserveSelected(r_,context.objectArray,bundleJason.pawn.object,doorClass,componentClass,selected)){
            if(bundleDoorRecovery.door&&bundleDoorRecovery.door==bundleDoorRecovery.lastAttemptDoor)
                cfgclosedtrace::Mark("door-followup-selection-released",bundleDoorRecovery.door,0);
            bundleDoorRecovery.phase=0;bundleDoorRecovery.door=0;return Result::None;}
        hostedDoorClause_=3;
        cfgdoor::BreakState nativeDoor;
        if(!DoorState(selected,doorClass,nativeDoor))return Result::Unavailable;
        // Native selection may outlive an opened/destroyed door. Do not stop
        // and face it again; allow ordinary pursuit to advance this callback.
        if(nativeDoor.open||nativeDoor.destroyed){
            // The earlier Press return can precede the native animation. This
            // later, freshly owned selected-door state is physical completion;
            // emit once without another interaction, scan or saved-pointer read.
            if(bundleDoorRecovery.door==selected.door&&bundleDoorRecovery.lastAttemptDoor==selected.door){
                cfgclosedtrace::Mark(nativeDoor.open?"door-followup-opened":"door-followup-destroyed",selected.door,selected.component);
                bundleDoorRecovery.Traversal(now);
            }
            bundleDoorRecovery.phase=0;bundleDoorRecovery.door=0;return Result::None;
        }
        if(nativeDoor.Owned(bundleJason.pawn.object,selected.door)){
            bundleDoorBreakSelection=selected;bundleDoorRecovery.NativeBreak(selected.door,now);
            bundleReport.doorBreakTarget=nativeDoor.target;bundleReport.doorBreakOwner=nativeDoor.killer;
            ++bundleReport.doorBreakStarts;return Result::Acted;
        }
        hostedDoorClause_=4;
        std::uintptr_t sceneClass=0;cfgspawn::Vec3 anchor{};
        if(!NativeClass(cfgposition::disk::SceneClassCache,cfgposition::disk::SceneConstructorCache,sceneClass)||
           !(nativeDoor.Blocked()?cfgdoor::ObserveAnchor(r_,context.objectArray,selected,componentClass,sceneClass,anchor):
             cfgdoor::ObserveSelectedAnchor(r_,context.objectArray,selected,sceneClass,anchor)))return Result::Unavailable;
        float squared=0;
        if(chaseTarget){
            cfgspawn::Vec3 j{},door{},target{};
            door=anchor;
            if(!Location(bundleJason.pawn.object,j)||!Location(chaseTarget,target))return Result::Unavailable;
            const auto dx=door.x-j.x,dy=door.y-j.y,dz=door.z-j.z;squared=dx*dx+dy*dy+dz*dz;
            if(!JasonDoorApproachPolicy::IsOnApproach(door.x-j.x,door.y-j.y,door.z-j.z,
                target.x-j.x,target.y-j.y,target.z-j.z)){
                bundleDoorRecovery.phase=0;bundleDoorRecovery.door=0;
                return Result::None; // donor: do not attack an irrelevant selected door
            }
        }else {cfgspawn::Vec3 j{};if(!Location(bundleJason.pawn.object,j))return Result::Unavailable;
            const auto x=anchor.x-j.x,y=anchor.y-j.y,z=anchor.z-j.z;squared=x*x+y*y+z*z;}
        if(!bundleDoorRecovery.SelectedDue(now,selected.door,squared))return Result::None;
        hostedDoorClause_=5;
        std::uintptr_t locked=0;
        if(!r_.Read(selected.manager+cfgvehicleinteraction::disk::LockedComponent,locked))return Result::Unavailable;
        if(locked){++bundleReport.doorForeignLockWaits;bundleDoorRecovery.next=now+250;bundleSchedule.huntAt=now+250;return Result::Acted;} // foreign/native interaction owns animation/path
        cfgspawn::Vec3 approachFrom{},approachGoal{};
        if(!Location(bundleJason.pawn.object,approachFrom))return Result::Unavailable;
        if(cfgdoor::Approach(approachFrom,anchor,approachGoal)){
            cfgtrap::MoveLocationParameters packet;packet.destination=approachGoal;packet.acceptance=35.f;packet.allowPartial=1;packet.result=255;
            if(!CarCall(cfgtrap::disk::MoveToLocation,cfgtrap::disk::MoveToLocationExecutor,cfgtrap::disk::MoveToLocationCode,
                bundleJason.killerController.object,&packet))return Result::Unavailable;
            bundleDoorRecovery.phase=0;bundleDoorRecovery.door=0;bundleDoorRecovery.next=now+500;
            bundleFollowPath.Clear();bundleMoveAt=now+500;
            cfgclosedtrace::Mark("door-approach-native-anchor",selected.door,packet.result);return Result::Acted;
        }
        if(bundleDoorRecovery.phase==0){
            if(!CarCall(cfgtrap::disk::StopMovement,cfgtrap::disk::StopMovementExecutor,cfgtrap::disk::StopMovementCode,
                bundleJason.killerController.object,nullptr))return Result::Unavailable;
            bundleDoorRecovery.Stopped(selected.door,now);bundleSchedule.huntAt=now+25;return Result::Acted;
        }
        // Face the SAME native component used by CanBreakDownDoor and issue
        // its Press without a callback yield in between. The AI can overwrite
        // control yaw during the old100ms Face -> Press gap.
        cfgspawn::Vec3 jason{},rotation{};
        if(!Location(bundleJason.pawn.object,jason))return Result::Unavailable;
        rotation.y=std::atan2(anchor.y-jason.y,anchor.x-jason.x)*57.2957795f;
        cfgtrap::RotationParameters body;body.rotation=rotation;
        if(!CarCall(cfgtrap::disk::K2_SetActorRotation,cfgtrap::disk::K2_SetActorRotationExecutor,cfgtrap::disk::K2_SetActorRotationCode,
            bundleJason.pawn.object,&body)||body.result!=1||
           !CarCall(cfgtrap::disk::SetControlRotation,cfgtrap::disk::SetControlRotationExecutor,cfgtrap::disk::SetControlRotationCode,
            bundleJason.killerController.object,&rotation))return Result::Unavailable;
        cfgdoor::BreakState doorState;
        if(!DoorState(selected,doorClass,doorState))return Result::Unavailable;
        std::uintptr_t attackComponentClass=0;std::uint8_t permitted=0;
        if(!NativeClass(cfgdoor::breakdisk::ComponentReflection,cfgdoor::breakdisk::ComponentConstructor,attackComponentClass))return Result::Unavailable;
        const bool attackComponent=cfgspawn::ObjectIs(r_,selected.component,attackComponentClass);
        if(attackComponent&&(!r_.Read(selected.component+cfgdoor::breakdisk::CanAttackToInteract,permitted)||permitted>1))return Result::Unavailable;
        hostedDoorClause_=6;
        if(doorState.Blocked()&&attackComponent&&permitted==1){
            const auto press=[&](BundleAccess& door){
                const auto refuse=[&](unsigned clause){
#ifdef CFG_H1_JASON
                    if(HostedRoutineActive())++hostedH1Report.doorLockedRefusals[clause];
#else
                    (void)clause;
#endif
                    return Result::Unavailable;
                };
                if(!doorState.CanRequestNative(permitted))return refuse(1);
                if(!cfgdoor::NativeSelectionMatches(door.r_,selected))return refuse(2);
                if(door.r_.reads>cfgbundle::ReadCap-512)return refuse(3);
                if(!door.DoorPressBinding())return refuse(4);
                if(!door.Context())return refuse(5);
                if(!door.Jason())return refuse(6);
                if(!door.Manager())return refuse(7);
                if(!cfgdoor::ObserveSelected(door.r_,door.context.objectArray,bundleJason.pawn.object,doorClass,componentClass,fresh))return refuse(8);
                if(!cfgdoor::Same(selected,fresh))return refuse(9);
                if(!cfgdoor::NativeSelectionMatches(door.r_,fresh))return refuse(10);
                ++bundleReport.doorAttempts;bundleDoorRecovery.Attempted(now);
                ++bundleReport.doorBreakRequests;
                cfgclosedtrace::Mark("door-locked-before",selected.door,selected.component);
                return door.Input(false)?Result::Acted:Result::Fatal;
            };
            const Result dispatched=press(*this);
            if(dispatched!=Result::Acted)return dispatched;
            ++bundleReport.doorNativeReturns;
            cfgclosedtrace::Mark("door-locked-input-returned",selected.door);
            if(!DoorState(selected,doorClass,doorState))return Result::Unavailable;
            bundleReport.doorBreakTarget=doorState.target;bundleReport.doorBreakOwner=doorState.killer;
            bundleReport.doorBreakIntent=doorState.wants;
            if(doorState.Owned(bundleJason.pawn.object,selected.door)){
                bundleDoorBreakSelection=selected;bundleDoorRecovery.NativeBreak(selected.door,now);++bundleReport.doorBreakStarts;
            }else if(!doorState.open&&!doorState.destroyed&&!doorState.target&&!doorState.killer)bundleDoorRecovery.Unconfirmed(now);
            bundleFollowPath.Clear();bundleMoveAt=now+1600;return Result::Acted;
        }
        if(doorState.Blocked()||doorState.target||doorState.killer){
#ifdef CFG_H1_JASON
            if(HostedRoutineActive())++hostedH1Report.doorLockedRefusals[0];
#endif
            return Result::Unavailable;
        }
        // PID10400: three open requests were followed by native breakdowns.
        // CFG's JasonInteraction can choose the attackable-door branch even
        // when this door is unlocked. Reuse the EXISTING reflected explicit
        // component Press below for ALL unlocked doors, not that generic input.
        // Locked doors retain the pinned donor JasonInteraction branch above.
        hostedDoorClause_=7;
        std::uintptr_t function=0,owner=0;
        if(!FeatureFunction(cfgdoor::disk::Interact,function,owner)||
           !cfgobserver::Same(r_,function+cfgdoor::disk::FunctionExecutor,plan.module+cfgdoor::disk::Executor)||
           !cfgbundle::Code(r_,plan.module,cfgdoor::disk::Executor,cfgdoor::disk::ExecutorCode))return Result::Unavailable;
        if(!Context()){if(ContextTransient())return Result::Unavailable;bundleReport.resourceFailure=1;return Result::Fatal;}
        if(!Jason()){bundleReport.resourceFailure=2;return Result::Fatal;}
        const auto manager=ManagerForAction(now);
        if(manager!=BundleManagerGateResult::Ready){
            bundleReport.resourceFailure=6;
            if(manager==BundleManagerGateResult::Deferred)return Result::Unavailable;
            return Result::Fatal;
        }
        if(!cfgdoor::ObserveSelected(r_,context.objectArray,bundleJason.pawn.object,doorClass,componentClass,fresh)||
           !cfgdoor::Same(selected,fresh))return Result::Unavailable;
        cfgdoor::BreakState freshDoor;
        if(!DoorState(fresh,doorClass,freshDoor)||
           freshDoor.Blocked()||freshDoor.open||freshDoor.destroyed||freshDoor.target||freshDoor.killer)return Result::Unavailable;
        cfgdoor::InteractPacket packet;packet.component=selected.component;
        bundleDoorRecovery.Attempted(now);
        ++bundleReport.doorAttempts;
        ++bundleReport.doorOpenRequests;
        cfgclosedtrace::Mark("door-unlocked-before",selected.door,selected.component);
        if(!Invoke(bundleJason.pawn.object,function,&packet))return Result::Fatal;
        ++bundleReport.doorNativeReturns; // return is not proof of door destruction
        // Let the native interaction/animation advance before resetting MoveTo.
        bundleMoveAt=now+1600;bundleFollowPath.Clear();
        cfgdoor::BreakState returnedDoor;
        if(DoorState(fresh,doorClass,returnedDoor)){
            cfgclosedtrace::Mark(returnedDoor.open?"door-unlocked-opened":returnedDoor.destroyed?"door-unlocked-destroyed":"door-unlocked-pending",fresh.door,fresh.component);
            if(returnedDoor.open||returnedDoor.destroyed)bundleDoorRecovery.Traversal(now);
            else if(!returnedDoor.target&&!returnedDoor.killer)bundleDoorRecovery.Unconfirmed(now);
        }
        // A returned native Press does not mean a locked door opened. Keep
        // obstruction evidence and retry the native bash instead of resetting
        // the sample and granting repeated traversal grace to a closed door.
        return Result::Acted; // yield; cannot use pre-interaction state for attack/Move
    }
    bool HelperResources(){
        const auto& a=spawnAssets.resources;
        bundleReport.helperPreparation=10;if(!HelperContext())return false;
        bundleReport.helperPreparation=11;if(!Tree())return false;
        bundleReport.helperPreparation=12;
        return cfgspawn::ObserveOwnedNativeHelper(r_,plan.module,context.objectArray,a.classes[cfgspawn::Helper],a.helperDefault,
            a.spawnFunction,spawnAssets.helperClassSlot,spawnAssets.slots[3],spawnAssets.slots[4]);
    }
    bool Current(){return HelperContext();} // one-shot counselor-start survey host
    bool WarmCounselorStarts(){
        struct Host {
            BundleAccess& access;
            bool Current(){return access.HelperContext();}
            bool Caches(std::uintptr_t& a,std::uintptr_t& b){return access.r_.Read(plan.module+cfgcounselorstart::disk::ClassCache,a)&&
                access.r_.Read(plan.module+cfgcounselorstart::disk::ConstructorCache,b);}
            bool InitializeClass(bool getter,std::uintptr_t& result)noexcept{
                using namespace cfgcounselorstart::disk;
                if(!access.NativeCallAllowed()||!Current()||
                    !(getter?cfgbundle::Code(access.r_,plan.module,ClassGetter,GetterCode):
                        cfgbundle::Code(access.r_,plan.module,ClassFactory,FactoryCode)))return false;
#ifdef CFG_RETAINED_GAMEPLAY_BUNDLE
                if(!access.RetainBundleActors(plan.pawn)||!access.NativeCallAllowed()||!access.BeginBundleNative(1))return false;
#endif
                ++access.calls_;
                __try{result=reinterpret_cast<std::uintptr_t(__fastcall*)()>(plan.module+(getter?ClassGetter:ClassFactory))();}
                __except(EXCEPTION_EXECUTE_HANDLER){access.exception_=true;
#ifdef CFG_RETAINED_GAMEPLAY_BUNDLE
                    access.BundleRetentionFailed();
#endif
                    return false;}
#ifdef CFG_RETAINED_GAMEPLAY_BUNDLE
                return access.RetainBundleReturnedObject(result);
#else
                return result!=0;
#endif
            }
        } host{*this};
        return bundleCounselorStartWarm.Step(host)!=cfgkillerstart::BootstrapState::Refused;
    }
    bool HelperClassCurrent(){
        cfgspawn::PlayerArray header{};cfgprimary::SoftClassBytes entry{};cfgspawn::TemplateObservation fresh{};
        const bool okay=bundleHelperSelected&&HelperContext()&&
            r_.Read(plan.mode+cfgmenu::cataloguedisk::CounselorCharacterClasses,header)&&
            header.data==bundleCounselorCatalogue.data&&header.count==bundleCounselorCatalogue.count&&header.capacity==bundleCounselorCatalogue.capacity&&
            bundleHelperEntry<static_cast<unsigned>(header.count)&&r_.Read(header.data+40*bundleHelperEntry,entry)&&entry==bundleHelperPacket&&
            cfgbundle::ResolvedCounselorTemplate(r_,context.objectArray,bundleHelperTemplate.pawnClass,bundleCounselorClass,
                spawnAssets.resources.classes[cfgspawn::CounselorAI],fresh)&&fresh.pawnClass==bundleHelperTemplate.pawnClass&&
            fresh.defaults==bundleHelperTemplate.defaults&&cfgbundle::OwnedClassSlotMatches(bundleHelperTemplate.classSlot,fresh.classSlot)&&
            cfgbundle::OwnedClassSlotMatches(bundleHelperTemplate.defaultSlot,fresh.defaultSlot)&&
            cfgmenu::classchoice::MatchesPath(r_,plan.module,fresh.pawnClass,bundleCounselorClass,entry)&&HelperContext();
        if(okay)bundleHelperTemplate=fresh;return okay;
    }
    bool HelperClassInvoke(std::uintptr_t function,void* packet){
#ifdef CFG_RETAINED_GAMEPLAY_BUNDLE
        bundleInvocationStage_=0;bundleRetentionStage_=0;bundleSoftProof_={};
        bundleEnvironmentStage_=0;bundleActorStage_=0;bundleNativeStage_=0;
#endif
        const bool returned=Invoke(spawnAssets.kismetDefault,function,packet,false);
#ifdef CFG_RETAINED_GAMEPLAY_BUNDLE
        if(!returned){
            const auto& d=bundleSoftProof_;
            cfgclosedtrace::Mark("helper-invoke-stage",function,(static_cast<std::uint64_t>(bundleInvocationStage_)<<32)|bundleRetentionStage_);
            cfgclosedtrace::Mark("helper-invoke-budget",function,(static_cast<std::uint64_t>(calls_)<<32)|r_.reads);
            cfgclosedtrace::Mark("helper-invoke-environment",function,bundleEnvironmentStage_);
            cfgclosedtrace::Mark("helper-invoke-actor",function,(static_cast<std::uint64_t>(bundleActorStage_)<<32)|bundleActorRetentionReads_);
            cfgclosedtrace::Mark("helper-invoke-native",function,bundleNativeStage_);
            cfgclosedtrace::Mark("helper-soft-stage",d.property,(static_cast<std::uint64_t>(d.stage)<<32)|d.reads);
            cfgclosedtrace::Mark("helper-soft-frame",function,(static_cast<std::uint64_t>(d.count)<<32)|d.bytes);
            cfgclosedtrace::Mark("helper-soft-shape",d.property,(static_cast<std::uint64_t>(static_cast<unsigned>(d.dimension))<<32)|static_cast<unsigned>(d.elementBytes));
            cfgclosedtrace::Mark("helper-soft-offset",d.property,static_cast<unsigned>(d.offset));
            cfgclosedtrace::Mark("helper-soft-flags",d.property,d.flags);
            cfgclosedtrace::Mark("helper-soft-mask",d.property,(static_cast<std::uint64_t>(d.index)<<32)|d.mask);
            const auto text=[&](const char* prefix,std::u16string_view value){
                std::array<char,192> tag{};unsigned n=0;
                for(unsigned i=0;prefix[i]&&n+1<tag.size();++i)tag[n++]=prefix[i];
                for(const auto c:value){if(n+1==tag.size())break;tag[n++]=c>=32&&c<127?static_cast<char>(c):'?';}
                cfgclosedtrace::Mark(tag.data(),d.property);
            };
            text("helper-soft-name-",d.name.View());text("helper-soft-type-",d.type.View());
        }
#endif
        return returned;
    }
    bool SelectHelperClass(){
        if(!bundleRandomCounselors.count){
            cfghelpercatalogue::Diagnostic diagnostic;
            if(!cfghelpercatalogue::Capture(r_,*this,plan.mode,bundleCounselorCatalogue,bundleCounselorEntries,diagnostic)){
                cfgclosedtrace::Mark("helper-catalogue-refused",plan.mode,static_cast<unsigned>(diagnostic.stage));
                cfgclosedtrace::Mark("helper-catalogue-shape",diagnostic.header.data,
                    (static_cast<std::uint64_t>(static_cast<std::uint32_t>(diagnostic.header.count))<<32)|static_cast<std::uint32_t>(diagnostic.header.capacity));
                cfgclosedtrace::Mark("helper-catalogue-entry",plan.mode,diagnostic.index);
                cfgclosedtrace::Mark("helper-catalogue-owner-stage",plan.controller,helperContextStage_);
                return ++bundleHelperCatalogueFailures<3; // finite read-only pre-allocation retry, never replay a native call
            }
            bundleHelperCatalogueFailures=0;
            if(!bundleRandomCounselors.Begin(static_cast<unsigned>(bundleCounselorCatalogue.count),static_cast<std::uint32_t>(GetTickCount64())^static_cast<std::uint32_t>(plan.world)))return false;
            cfgclosedtrace::Mark("helper-catalogue-captured",plan.mode,bundleCounselorCatalogue.count);
            return true; // catalogue capture and conversion never bunch
        }
        const auto now=GetTickCount64();
        const bool preload=bundleCounselorPreload.Loading(bundleRandomCounselors.count);
        if(!preload&&bundleCounselorPreload.Warming(now))return true;
        unsigned index=preload?bundleCounselorPreload.cursor:0;
        if(!preload&&!bundleHelperResidentPass.Next(bundleRandomCounselors,index)){bundleReport.helperPreparation=9;return false;}
        bundleHelperEntry=index;
        const auto& source=bundleCounselorEntries[index];cfgprimary::SoftClassBytes fresh{};cfgspawn::PlayerArray header{};
        if(!HelperContext()||!r_.Read(plan.mode+cfgmenu::cataloguedisk::CounselorCharacterClasses,header)||
            header.data!=bundleCounselorCatalogue.data||header.count!=bundleCounselorCatalogue.count||header.capacity!=bundleCounselorCatalogue.capacity||
            !r_.Read(header.data+40*index,fresh)||fresh!=source){
            cfgclosedtrace::Mark("helper-selected-source-refused",plan.mode,(static_cast<std::uint64_t>(index)<<32)|helperContextStage_);return false;}
        if(preload){
            cfgcounselorload::Packet load{};std::uintptr_t function=0,type=0;
            if(cfgcounselorload::Build(plan.world,plan.controller,index,source,load)){
                const auto proof=[&](unsigned stage,bool okay){if(!okay)cfgclosedtrace::Mark("helper-preload-refused",function,
                    (static_cast<std::uint64_t>(index)<<32)|stage);return okay;};
                if(!proof(1,FeatureFunction(cfgcounselorload::disk::LoadClass,function,type))||
                   !proof(2,cfgspawn::ObjectIs(r_,spawnAssets.kismetDefault,type))||
                   !proof(3,cfgobserver::Same(r_,function+0xB0,plan.module+cfgcounselorload::disk::Executor))||
                   !proof(4,cfgbundle::Code(r_,plan.module,cfgcounselorload::disk::Executor,cfgcounselorload::disk::ExecutorCode)))return false;
                if(!proof(5,HelperClassInvoke(function,&load)))return false;
                ++bundleReport.helperLoadRequests;
                if(!proof(6,HelperContext()))return false;
            }
            bundleCounselorPreload.Requested(bundleRandomCounselors.count,now);return true;
        }
        cfgprimary::ConversionParameters packet{};
        if(!cfgprimary::BuildConversion(source,packet)){++bundleReport.helperInvalidPackets;bundleReport.helperPreparation=8;return true;} // empty/subobject rows aren't counselor choices
        ++bundleReport.helperResolveCalls;
        cfgprobe::lifetime::SlotObservation resource{};
        if(!cfgobserver::Same(r_,plan.module+cfgprimary::disk::Conversion.cache,spawnAssets.conversionFunction)||
            !cfgspawn::ObjectSlot(r_,context.objectArray,spawnAssets.conversionFunction,resource)||
            !cfgbundle::OwnedClassSlotMatches(spawnAssets.conversionSlot,resource)||
            !cfgspawn::ObjectSlot(r_,context.objectArray,spawnAssets.kismetClass,resource)||
            !cfgbundle::OwnedClassSlotMatches(spawnAssets.kismetClassSlot,resource)||
            !cfgspawn::ObjectSlot(r_,context.objectArray,spawnAssets.kismetDefault,resource)||
            !cfgbundle::OwnedClassSlotMatches(spawnAssets.kismetDefaultSlot,resource)||
            !cfgobserver::Same(r_,spawnAssets.kismetClass+cfgspawn::ClassDefaultObject,spawnAssets.kismetDefault)||
            !cfgobserver::Same(r_,spawnAssets.kismetDefault+cfgprobe::ObjectClass,spawnAssets.kismetClass)||
            !cfgobserver::Same(r_,spawnAssets.conversionFunction+0x20,spawnAssets.kismetClass)||
            !cfgbundle::Code(r_,plan.module,cfgprimary::disk::Conversion.factory,cfgprimary::disk::Conversion.prefix)||
            !HelperClassInvoke(spawnAssets.conversionFunction,&packet)||!HelperContext()||
            !r_.Read(header.data+40*index,fresh)||fresh!=source)return false;
        if(!packet.result){
            ++bundleReport.helperColdClasses;
            return true; // donor: continue shuffled resident choices, no30s cold-row stall
        }
        cfgnames::Observation<Reader> names(r_,plan.module);cfgnames::Text className;
        if(!names.Object(packet.result,className))return false;
        if(!cfgresidentcounselor::Ordinary(className.View()))return true; // donor excludes base/hero, never random Tommy
        if(bundleHelperResidentPass.fallback&&packet.result==spawnAssets.resources.local.pawnClass&&bundleCounselorPreload.Pending(now)){
            bundleRandomCounselors.NextSelection();bundleHelperResidentPass={};return true;
        }
        if(!bundleHelperResidentPass.Use(packet.result,spawnAssets.resources.local.pawnClass,index,bundleRandomCounselors.Used(packet.result)))return true;
        cfgspawn::TemplateObservation value{};
        if(!cfgbundle::ResolvedCounselorTemplate(r_,context.objectArray,packet.result,bundleCounselorClass,
            spawnAssets.resources.classes[cfgspawn::CounselorAI],value)||
            !cfgmenu::classchoice::MatchesPath(r_,plan.module,packet.result,bundleCounselorClass,source))return true; // skip incompatible native entries, no actor allocated
        bundleHelperTemplate=value;bundleHelperPacket=source;bundleHelperEntry=index;bundleHelperSelected=true;
        return true; // Fresh source/class proof is repeated in the next phase.
    }
#include "HelperMarkerProofRuntimeMembers.hpp"
#include "HelperProjectionRuntimeMembers.hpp"
#include "HelperBirthRuntimeMembers.hpp"
    void SpawnCounselor(){
        using namespace cfgbundle;
        if(!bundleHelpers.Active()||bundleReport.bots>=bundleSchedule.requestedBots)return;
        bundleReport.helperDispatched=0;
        if(bundleHelperBirth.pending){FinishHelperBirth();return;}
        if(!bundleHelperSelected&&!bundleRandomCounselors.count&&
           bundleCounselorStartWarm.state==cfgkillerstart::BootstrapState::Ready&&
           bundleCounselorStarts.CurrentState()==cfgcounselorstart::State::Ready){
            bundleReport.helperPreparation=2;
            if(!SelectHelperClass())bundleHelpers.Refuse(HelperFailure::Resources);
            bundleSchedule.spawnAt=GetTickCount64()+100;return; // independent read-only catalogue callback
        }
        if(bundleHelpers.stage==HelperStage::Project&&bundleHelperSelected&&
           bundleCounselorStartWarm.state==cfgkillerstart::BootstrapState::Ready&&
           bundleCounselorStarts.CurrentState()==cfgcounselorstart::State::Ready){
            ProjectHelperStart();return;
        }
        bundleReport.helperClassSerial=bundleHelperTemplate.classSlot.serial;bundleReport.helperDefaultSerial=bundleHelperTemplate.defaultSlot.serial;
        if(!HelperResources()){bundleHelpers.Refuse(HelperFailure::Resources);return;}
        if(bundleCounselorStartWarm.state!=cfgkillerstart::BootstrapState::Ready){
            bundleReport.helperPreparation=1;
            if(!WarmCounselorStarts()){bundleHelpers.Refuse(HelperFailure::Resources);return;}
            bundleSchedule.spawnAt=GetTickCount64()+100;return;
        }
        if(bundleCounselorStarts.CurrentState()!=cfgcounselorstart::State::Ready){
            bundleReport.helperPreparation=1;
            const bool okay=bundleCounselorStarts.CurrentState()==cfgcounselorstart::State::Empty?
                bundleCounselorStarts.Begin(r_,*this,plan.module,plan.world):bundleCounselorStarts.Step(r_,*this);
            if(!okay){bundleHelpers.Refuse(HelperFailure::Projection);return;}
            bundleSchedule.spawnAt=GetTickCount64()+100;return;
        }
        if(!bundleHelperSelected){
            bundleReport.helperPreparation=2;
            if(!SelectHelperClass()){bundleHelpers.Refuse(HelperFailure::Resources);return;}
            bundleSchedule.spawnAt=GetTickCount64()+100;return;
        }
        bundleReport.helperPreparation=3;
        if(!HelperClassCurrent()){
            // Resident class ownership belongs to the engine. A copied class is
            // never treated as a GC root: discard it and re-resolve the same
            // strongly owned catalogue source if GC ran before the spawn.
            if(bundleHelperStaleRetries++<2){
                bundleHelperResidentPass.Retry(bundleHelperEntry);bundleHelperSelected=false;bundleHelperTemplate={};
                bundleSchedule.spawnAt=GetTickCount64()+100;return;
            }
            bundleHelpers.Refuse(HelperFailure::Resources);return;
        }
        cfgcounselorstart::Marker freshMarker{};
        bundleReport.helperPreparation=5;
        if(!HelperMarkerCurrent(bundleHelperStart,freshMarker)){
            bundleHelpers.Refuse(HelperFailure::Projection);return;
        }
        cfgspawn::SpawnAIParameters p;
        if(!cfgspawn::BuildParameters(plan.world,bundleHelperTemplate.pawnClass,spawnAssets.primary.counselorTree,bundleHelpers.location,{},p)){
            bundleHelpers.Refuse(HelperFailure::Packet);return;
        }
        if(!Context()||!Jason()){bundleHelpers.Refuse(HelperFailure::Resources);return;}
        DispatchHelperBirth(p);
    }
#include "AdditionalJasonRuntimeMembers.hpp"
#include "HostedJasonMembers.hpp"
#include "HostedLocalViewMembers.hpp"
#include "JasonRageDifficultyRuntimeMembers.hpp"
#include "NativeSummaryRuntimeMembers.hpp"
#include "DonorJasonPreparationRuntimeMembers.hpp"
#include "DonorJasonActionRuntimeMembers.hpp"
#include "DonorHeldGrabKillNativeMembers.hpp"
#include "DonorCounselorConvergenceNativeMembers.hpp"
#include "LeanConvergenceNativeMembers.hpp"
#include "DonorJasonHuntingHostMembers.hpp"
#include "DonorJasonRosterRuntimeMembers.hpp"
#include "CounselorPracticeRuntimeMembers.hpp"
};
#include "DonorJasonHuntingService.hpp"
#include "CounselorPracticeHooks.hpp"
bool BundleNativeJasonLabelHint(){
    Reader hint;hint.cap=12;std::uintptr_t camera=0,active=0,pending=0,spectator=0,attached=0;
    if(!BundleGameplayStarted()||!bundleSchedule.active||!(bundleCompletion.terminalMask&1u))return false;
    if(!hint.Read(plan.controller+cfgspectatorview::disk::ControllerCamera,camera)||!cfgprobe::ObjectPointer(camera)||
       !hint.Read(camera+cfgspectatorview::disk::Active,active)||
       !hint.Read(camera+cfgspectatorview::disk::Pending,pending)||pending)return false;
    if(active==bundleJason.pawn.object)return true;
    return hint.Read(plan.controller+cfgprobe::ControllerSpectatorPawn,spectator)&&cfgprobe::ObjectPointer(spectator)&&
        active==spectator&&hint.Read(spectator+0x3F8,attached)&&attached==bundleJason.pawn.object;
}
bool BundleRefreshJasonStatus(std::uint64_t now){
    static BundleAccess::NativeLabelEvent resolved;
    const bool jasonHint=BundleNativeJasonLabelHint();
    if(!bundleSpectatorLabelPending&&now>=bundleSpectatorLabelAt&&(jasonHint||bundleJasonLabelOwned)){
        bundleSpectatorLabelPending=true;bundleSpectatorLabelAttempts=0;
    }
    if(!bundleSpectatorLabelPending||now<bundleSpectatorLabelAt)return false;
    if(!jasonHint&&!bundleJasonLabelOwned){bundleSpectatorLabelPending=false;return false;}
    bundleSpectatorLabelAt=now+1000;
    if(++bundleSpectatorLabelAttempts>=4)bundleSpectatorLabelPending=false;
    // A silent native selection CAS does not emit a HUD status event. Refresh
    // once on the successful follow edge, with bounded retries and fresh exact
    // HUD/function/selection proof. Never rename PlayerState or write FString.
    auto text=bundleReport.spectatorStatusText;auto mode=bundleReport.spectatorStatusNextMode;
    const auto count=[](const auto& buffer){int n=0;while(n<static_cast<int>(buffer.size())&&buffer[n])++n;return n<static_cast<int>(buffer.size())?n+1:0;};
    const int textCount=count(text),modeCount=count(mode);
    if(!textCount||!modeCount||!text[0]||!bundleSpectatorStatusFunction||!bundleSpectatorStatusWidgetHint)return true;
    BundleAccess status;
    std::uintptr_t currentWidget=0;
    if(!status.NativeLabelWidget(currentWidget)){
        cfgclosedtrace::Mark("spectator-label-hud-refused",bundleSpectatorStatusWidgetHint,status.Reads());return true;
    }
    if(currentWidget!=bundleSpectatorStatusWidgetHint){
        // Refresh the dispatcher hint, discard the old receiver proof, and
        // resolve the new receiver's actual override on a separate callback.
        // Never drive the accepted camera just to repair a replaced HUD label.
        bundleSpectatorStatusWidgetHint=currentWidget;resolved={};
        bundleSpectatorLabelAttempts=0;return true;
    }
    if(resolved.widget.object!=bundleSpectatorStatusWidgetHint||!status.NativeLabelEventIdentityCurrent(resolved)){
        // One bounded discovery phase, never stacked with the full event proof
        // and native dispatch. A later phase rechecks both positive identities.
        const auto ready=status.ResolveNativeLabelEvent(bundleSpectatorStatusWidgetHint,resolved);
        cfgclosedtrace::Mark(ready?"spectator-label-event-ready":"spectator-label-event-refused",bundleSpectatorStatusWidgetHint,status.Reads());
        bundleReport.reads+=status.Reads();bundleReport.maxReads=(std::max)(bundleReport.maxReads,status.Reads());
        return true;
    }
    cfgspectatorstatus::Frame caller{{reinterpret_cast<std::uintptr_t>(text.data()),textCount,textCount},
        {reinterpret_cast<std::uintptr_t>(mode.data()),modeCount,modeCount}},replacement{};
    const bool labelled=jasonHint?status.NativeJasonLabel(bundleSpectatorStatusWidgetHint,resolved.function.object,
        reinterpret_cast<std::uintptr_t>(&caller),replacement):status.NativeNeutralLabel(bundleSpectatorStatusWidgetHint,
        resolved.function.object,reinterpret_cast<std::uintptr_t>(&caller),replacement);
    if(!labelled)cfgclosedtrace::Mark("spectator-label-proof-refused",bundleSpectatorStatusWidgetHint,
        (static_cast<std::uint64_t>(status.nativeLabelProofStage_)<<32)|status.Reads());
    const bool eventCurrent=labelled&&status.NativeLabelEventCurrent(resolved);
    if(labelled&&!eventCurrent){resolved={};cfgclosedtrace::Mark("spectator-label-event-changed",currentWidget,status.Reads());}
    if(eventCurrent&&status.Invoke(bundleSpectatorStatusWidgetHint,resolved.function.object,&replacement,false)){
        bundleSpectatorLabelPending=false;bundleJasonLabelOwned=jasonHint;
    }
    bundleReport.reads+=status.Reads();bundleReport.calls+=status.Calls();
    bundleReport.maxReads=(std::max)(bundleReport.maxReads,status.Reads());
    bundleReport.maxCalls=(std::max)(bundleReport.maxCalls,status.Calls());
    return true;
}
bool CallOriginalThrowKnife(void* killer)noexcept{
    if(!bundleKnifeNotifyHook.original)return false;
    __try{reinterpret_cast<void(__fastcall*)(void*)>(bundleKnifeNotifyHook.original)(killer);return true;}
    __except(EXCEPTION_EXECUTE_HANDLER){return false;}
}
void ObserveKnifeBoundary(unsigned kind,std::uintptr_t subject,std::uintptr_t pc)noexcept{
    if(!bundleKnifeBoundaryObserveEnabled||GetCurrentThreadId()!=plan.threadId||!BundleGameplayStarted()||!bundleSchedule.active||!subject||!pc)return;
    if(kind==1&&subject!=bundleJason.pawn.object)return;
    if(kind==2&&!bundleKnife.Busy())return;
    const bool own=(bundleReport.generation&1u)==0;
    if(own)InterlockedIncrement(reinterpret_cast<volatile LONG*>(&bundleReport.generation));
    unsigned flags=0;
#ifdef CFG_RETAINED_GAMEPLAY_BUNDLE
    if(bundleRootLease)flags|=1u;
    if(bundleRootLease&&bundleRootLease->NativeEntered())flags|=2u;
#endif
    if(bundleKnifeReleaseDepth)flags|=4u;
    const auto stage=static_cast<unsigned>(bundleKnife.stage);
    auto index=bundleReport.knifeBoundaryCount;
    for(unsigned i=0;i<bundleReport.knifeBoundaryCount;++i){const auto& sample=bundleReport.knifeBoundaries[i];
        if(sample.kind==kind&&sample.returnPc==pc&&sample.flags==flags&&sample.knifeStage==stage){index=i;break;}}
    if(index==bundleReport.knifeBoundaryCount&&index<bundleReport.knifeBoundaries.size()){
        bundleReport.knifeBoundaries[index]={pc,subject,kind,flags,stage,0};++bundleReport.knifeBoundaryCount;
    }
    if(index<bundleReport.knifeBoundaries.size()){
        auto& sample=bundleReport.knifeBoundaries[index];sample.subject=subject;if(sample.count!=UINT32_MAX)++sample.count;
    }else if(bundleReport.knifeBoundaryOverflow!=UINT32_MAX)++bundleReport.knifeBoundaryOverflow;
    if(own){MemoryBarrier();InterlockedIncrement(reinterpret_cast<volatile LONG*>(&bundleReport.generation));}
}
void __fastcall BundleNativeThrowKnife(void* killer){
    if(bundleKnifeReleaseAdapterEnabled){
        const auto pc=reinterpret_cast<std::uintptr_t>(_ReturnAddress());
        ObserveKnifeBoundary(1,reinterpret_cast<std::uintptr_t>(killer),pc);
        auto* frame=bundleKnifeReleaseFrame;
#ifdef CFG_RETAINED_GAMEPLAY_BUNDLE
        const bool admitted=frame&&frame->Enter(reinterpret_cast<std::uintptr_t>(killer),bundleRootLease,
            GetCurrentThreadId(),bundleRootLease&&bundleRootLease->NativeEntered(),bundleKnifeReleaseDepth,pc);
#else
        const bool admitted=false;
#endif
        if(admitted){
            const bool wasBusy=bundleKnifeNotifyBusy;bundleKnifeNotifyBusy=true;
            const bool returned=CallOriginalThrowKnife(killer); // exact original ONCE, no getters/retention here
            bundleKnifeNotifyBusy=wasBusy;frame->Leave(returned);
        }else (void)CallOriginalThrowKnife(killer);
        return;
    }
    if(bundleKnifeBoundaryObserveEnabled){
        ObserveKnifeBoundary(1,reinterpret_cast<std::uintptr_t>(killer),reinterpret_cast<std::uintptr_t>(_ReturnAddress()));
        (void)CallOriginalThrowKnife(killer);return; // exactly one original, no corrective native work
    }
    if(!bundleKnifeNotifyEnabled||bundleKnifeNotifyBusy||GetCurrentThreadId()!=plan.threadId){
        (void)CallOriginalThrowKnife(killer);return;
    }
    // Only the positively owned AI transaction gets a completion window.
    // Foreign/native human throws retain exactly one original call unchanged.
    struct ReportScope {
        bool publish=(bundleReport.generation&1u)==0;
        ReportScope(){if(publish)InterlockedIncrement(reinterpret_cast<volatile LONG*>(&bundleReport.generation));}
        ~ReportScope(){if(publish){MemoryBarrier();InterlockedIncrement(reinterpret_cast<volatile LONG*>(&bundleReport.generation));}}
    } reportScope;
    cfgknife::NotifyWindow window;BundleAccess before;
    const bool admitted=before.KnifeNotifyBefore(reinterpret_cast<std::uintptr_t>(killer),window);
    if(reinterpret_cast<std::uintptr_t>(killer)==bundleJason.pawn.object&&bundleKnife.stage==cfgknife::Stage::Notify){
        ++bundleKnifeNotifyEntries;bundleKnifeNotifyMaxReads=(std::max)(bundleKnifeNotifyMaxReads,before.Reads());
        if(!admitted)++bundleKnifeNotifyAdmissionRefused;
    }
    if(!admitted){(void)CallOriginalThrowKnife(killer);return;}
    struct Scope {
        Scope(cfgknife::NotifyWindow& w){bundleKnifeNotifyBusy=true;bundleKnifeNotifyWindow=&w;}
        ~Scope(){bundleKnifeNotifyWindow=nullptr;bundleKnifeNotifyBusy=false;}
    } scope(window);
    const bool returned=CallOriginalThrowKnife(killer); // original ONCE, no retry
    if(returned){BundleAccess after(cfgbundle::ReadCap-before.Reads());
        const bool corrected=after.KnifeNotifyAfter(window);
        bundleKnifeNotifyMaxReads=(std::max)(bundleKnifeNotifyMaxReads,before.Reads()+after.Reads());
        if(corrected){++bundleKnifeNotifyCorrections;return;}
    }
    // A witnessed real native launch needs no duplicate. Preserve its flight;
    // any stop/ambiguous outcome is retired by the ordinary bounded cleanup.
    if(returned&&window.launched&&!window.stopped){
        if(window.geometryUsed)++bundleKnifeNotifyCorrections;
        ++bundleReport.knifeNativeForwards;
        (void)bundleKnife.CompleteNativeNotify(window.driver,GetTickCount64());
    }else {++bundleKnifeNotifyRefused;bundleKnife.Abort(GetTickCount64());}
}
void __fastcall BundleNativeKnifeMulticast(void* driver,void* character,const cfgspawn::Vec3* velocity,const cfgspawn::Vec3* location){
    if(bundleKnifeReleaseAdapterEnabled){
        bool active=false;
#ifdef CFG_RETAINED_GAMEPLAY_BUNDLE
        active=bundleKnifeReleaseFrame&&bundleKnifeReleaseFrame->Geometry(bundleRootLease,GetCurrentThreadId(),
            bundleRootLease&&bundleRootLease->NativeEntered(),bundleKnifeReleaseDepth);
#endif
        if(!active){
            if(bundleKnifeMulticastHook.original)reinterpret_cast<void(__fastcall*)(void*,void*,const cfgspawn::Vec3*,const cfgspawn::Vec3*)>(
                bundleKnifeMulticastHook.original)(driver,character,velocity,location);
            return;
        }
    }
    cfgspawn::Vec3 correctedVelocity{},correctedLocation{};
    if(bundleKnifeNotifyWindow&&bundleKnifeNotifyWindow->driver.object==reinterpret_cast<std::uintptr_t>(driver)){
        auto& owned=*bundleKnifeNotifyWindow;
        if(GetCurrentThreadId()==plan.threadId&&owned.UseNativeGeometry(
            reinterpret_cast<std::uintptr_t>(driver),reinterpret_cast<std::uintptr_t>(character))){
            correctedLocation={owned.geometry.origin.x,owned.geometry.origin.y,owned.geometry.origin.z};
            correctedVelocity={owned.geometry.velocity.x,owned.geometry.velocity.y,owned.geometry.velocity.z};
            location=&correctedLocation;velocity=&correctedVelocity;
        }
        bundleKnifeNotifyWindow->multicast=true;
        const auto& window=*bundleKnifeNotifyWindow;
        if(window.armed&&!window.consumed&&GetCurrentThreadId()==plan.threadId&&
           window.jason.object==reinterpret_cast<std::uintptr_t>(character)){
            const bool publish=(bundleReport.generation&1u)==0;
            if(publish)InterlockedIncrement(reinterpret_cast<volatile LONG*>(&bundleReport.generation));
            cfgmenu::promotion::CurrentProcessReader reader;cfgspawn::Vec3 point{},speed{};
            if(reader.Read(reinterpret_cast<std::uintptr_t>(location),point)&&cfgspawn::Finite(point)&&
               reader.Read(reinterpret_cast<std::uintptr_t>(velocity),speed)&&cfgspawn::Finite(speed)){
                bundleReport.knifeNativeLaunchPoint=point;bundleReport.knifeNativeLaunchVelocity=speed;
                ++bundleReport.knifeNativeLaunchSamples;
            }else ++bundleReport.knifeNativeLaunchRefused;
            MemoryBarrier();if(publish)InterlockedIncrement(reinterpret_cast<volatile LONG*>(&bundleReport.generation));
        }
    }
    if(bundleKnifeMulticastHook.original)
        reinterpret_cast<void(__fastcall*)(void*,void*,const cfgspawn::Vec3*,const cfgspawn::Vec3*)>(bundleKnifeMulticastHook.original)(driver,character,velocity,location);
}
void __fastcall BundleNativeKnifeLaunch(void* driver,const cfgspawn::Vec3* velocity){
    bool observe=!bundleKnifeReleaseAdapterEnabled;
#ifdef CFG_RETAINED_GAMEPLAY_BUNDLE
    if(bundleKnifeReleaseAdapterEnabled)observe=bundleKnifeReleaseFrame&&bundleRootLease&&
        bundleKnifeReleaseFrame->lease==bundleRootLease&&bundleRootLease->NativeEntered()&&GetCurrentThreadId()==plan.threadId;
#endif
    auto* frame=bundleKnifeReleaseFrame;
    cfgknife::NativeFlightObservation before{},after{};Reader diagnostic;diagnostic.cap=128;
    const auto sample=[&](cfgknife::NativeFlightObservation& value){cfgprobe::lifetime::ArrayHeader objects{};
        return observe&&frame&&frame->window.driver.object==reinterpret_cast<std::uintptr_t>(driver)&&
            cfgprobe::lifetime::ReadArrayHeader(diagnostic,plan.module+cfgprobe::lifetime::ObjectArrayRva,objects)&&
            cfgknife::ObserveNativeFlight(diagnostic,objects,frame->window.driver,frame->window.jason.object,
                plan.module+cfgknife::disk::ProjectileMovementTable,value);};
    const bool sampledBefore=sample(before);
    if(observe&&bundleKnifeNotifyWindow&&bundleKnifeNotifyWindow->driver.object==reinterpret_cast<std::uintptr_t>(driver))
        bundleKnifeNotifyWindow->launched=true;
    if(bundleKnifeLaunchHook.original)
        reinterpret_cast<void(__fastcall*)(void*,const cfgspawn::Vec3*)>(bundleKnifeLaunchHook.original)(driver,velocity);
    if(sampledBefore&&frame==bundleKnifeReleaseFrame&&sample(after)){
        const bool publish=(bundleReport.generation&1u)==0;
        if(publish)InterlockedIncrement(reinterpret_cast<volatile LONG*>(&bundleReport.generation));
        ++bundleReport.knifeNativeOriginSamples;
        bundleReport.knifeNativeOrigin={before.flight.x,before.flight.y,before.flight.z};
        bundleReport.knifeNativePost={after.flight.x,after.flight.y,after.flight.z};
        bundleReport.knifeNativePostSpeed=after.flight.speed;
        bundleReport.knifeNativePostUpdated=after.flight.updated;
        // Preserve the actual pre-launch location as the diagnostic baseline.
        // The previous first sample at31ms could miss an entire short flight.
        if(bundleKnife.stage==cfgknife::Stage::Notify&&bundleKnife.release.driver==frame->window.driver&&
           !bundleKnife.flightOriginValid){
            bundleKnife.flightOrigin=before.flight;bundleKnife.flightOriginValid=true;++bundleKnife.flightSamples;
        }
        MemoryBarrier();if(publish)InterlockedIncrement(reinterpret_cast<volatile LONG*>(&bundleReport.generation));
    }
}
void BundleKnifeStopDiagnostic(std::uintptr_t throwable){
    const bool publish=(bundleReport.generation&1u)==0;
    if(publish)InterlockedIncrement(reinterpret_cast<volatile LONG*>(&bundleReport.generation));
    BundleAccess diagnostic;diagnostic.KnifeLifecycleSample(throwable,3);
    if(publish){MemoryBarrier();InterlockedIncrement(reinterpret_cast<volatile LONG*>(&bundleReport.generation));}
}
void __fastcall BundleNativeKnifeStop(void* throwable,const void* hit){
    if(bundleKnifeReleaseAdapterEnabled){
        ObserveKnifeBoundary(2,reinterpret_cast<std::uintptr_t>(throwable),reinterpret_cast<std::uintptr_t>(_ReturnAddress()));
        // Latch across the whole lexical Release, including BEFORE notify.
        // Never suppress a Windup/flight stop using Release-only evidence.
        bool current=false;
#ifdef CFG_RETAINED_GAMEPLAY_BUNDLE
        current=bundleKnifeReleaseFrame&&bundleRootLease&&bundleKnifeReleaseFrame->lease==bundleRootLease;
#endif
        if(current&&GetCurrentThreadId()==plan.threadId&&
           bundleKnifeReleaseFrame->window.driver.object==reinterpret_cast<std::uintptr_t>(throwable)){
            bundleKnifeReleaseFrame->window.stopped=true;++bundleKnifeNotifyStops;
        }
        if(bundleKnifeStopOriginal)bundleKnifeStopOriginal(throwable,hit);
        return;
    }
    if(bundleKnifeBoundaryObserveEnabled){
        ObserveKnifeBoundary(2,reinterpret_cast<std::uintptr_t>(throwable),reinterpret_cast<std::uintptr_t>(_ReturnAddress()));
        if(bundleKnifeStopOriginal)bundleKnifeStopOriginal(throwable,hit);
        return; // never suppress a held/flight stop in this witness
    }
    if(bundleKnifeNotifyWindow&&bundleKnifeNotifyWindow->driver.object==reinterpret_cast<std::uintptr_t>(throwable)){
        bundleKnifeNotifyWindow->stopped=true;++bundleKnifeNotifyStops;
    }
    const auto now=GetTickCount64();
    if(GetCurrentThreadId()==plan.threadId&&BundleGameplayStarted()&&bundleSchedule.active&&bundleKnife.Busy()){
        BundleKnifeStopDiagnostic(reinterpret_cast<std::uintptr_t>(throwable));
    }
    if(GetCurrentThreadId()==plan.threadId&&BundleGameplayStarted()&&bundleSchedule.active&&bundleKnife.HeldStopCandidate(now)){
        BundleAccess access;bool suppressed=false;
        __try{suppressed=access.KnifeHeldStop(reinterpret_cast<std::uintptr_t>(throwable),now);}
        __except(EXCEPTION_EXECUTE_HANDLER){suppressed=false;}
        bundleKnifeStopMaxReads=(std::max)(bundleKnifeStopMaxReads,access.Reads());
        if(suppressed){++bundleKnifeHeldStops;return;}
        ++bundleKnifeHeldRefused;
    }
    if(bundleKnifeStopOriginal)bundleKnifeStopOriginal(throwable,hit); // real flight/foreign/stale stop exactly once
}
void __fastcall BundleNativeSweaterCleanup(void* counselor){
    static bool busy=false;
    const bool owned=BundleGameplayStarted()&&bundleSchedule.active&&GetCurrentThreadId()==plan.threadId&&
        reinterpret_cast<std::uintptr_t>(counselor)==bundleSweaterOwner.object;
    if(!owned||busy){if(bundleSweaterCleanupOriginal)bundleSweaterCleanupOriginal(counselor);return;}
    busy=true;
    struct Exit {bool& flag;~Exit(){flag=false;}} exit{busy};
    cfgsweater::Retention token;cfgprobe::lifetime::SlotObservation identity;BundleAccess access;
    const bool captured=access.CaptureSweaterCleanup(reinterpret_cast<std::uintptr_t>(counselor),token,identity);
    // Exact native cleanup ONCE, including its visual/timer work. Never block
    // activation, never retain an unknown player's ability or resurrect a slot.
    if(bundleSweaterCleanupOriginal)bundleSweaterCleanupOriginal(counselor);
    const bool publish=(bundleReport.generation&1u)==0;
    if(publish)InterlockedIncrement(reinterpret_cast<volatile LONG*>(&bundleReport.generation));
    ++bundleReport.sweaterRetentionChecks;
    if(captured&&access.RestoreSweaterCleanup(token,identity))++bundleReport.sweaterRetentionRestored;
    else {++bundleReport.sweaterRetentionRefused;if(!captured)bundleReport.sweaterRetentionFailure=1;}
    bundleReport.sweaterRetentionMaxReads=(std::max)(bundleReport.sweaterRetentionMaxReads,access.Reads());
    MemoryBarrier();if(publish)InterlockedIncrement(reinterpret_cast<volatile LONG*>(&bundleReport.generation));
}
void BundleTrapEventAfter(std::uintptr_t victim,std::uintptr_t trap){
    if(!BundleGameplayStarted()||!bundleSchedule.active||!victim||!trap)return;
    ++bundleTrapEvents;BundleAccess access;
    const auto now=GetTickCount64();
    if(access.TrapEvent(victim,trap,now)){
        ++bundleTrapAccepted;
        if(CFG_J214_TRAP_PRIORITY)bundleDonorPrimary.trapIntent.Post(victim,now,bundleDonorPrimary.chase.LastMorphTeleportAt);
    }else ++bundleTrapRefused;
    bundleTrapMaxReads=(std::max)(bundleTrapMaxReads,access.Reads());
}
cfgdifficulty::TakeDamageOriginal bundleDamageOriginal=nullptr;
#include "AdditionalConstructionHook.hpp"
bool __fastcall BundleNativeSummaryFilter(void* query,void* state){
    // Preserve the lobby predicate for every native call. The exception is UI
    // eligibility only; it never acknowledges travel or changes PlayerState.
    const bool native=bundleSummaryFilterOriginal&&bundleSummaryFilterOriginal(query,state);
    if(native||!BundleSummaryStarted()||GetCurrentThreadId()!=plan.threadId||
       !bundleSummaryBots.Contains(reinterpret_cast<std::uintptr_t>(state)))return native;
    BundleAccess access;bool allowed=false;
    __try{allowed=access.SummaryBotEligible(reinterpret_cast<std::uintptr_t>(query),reinterpret_cast<std::uintptr_t>(state));}
    __except(EXCEPTION_EXECUTE_HANDLER){allowed=false;}
    if(allowed){if(++bundleSummaryBots.overrides==1)cfgclosedtrace::Mark("summary-offline-bot-filter-used",reinterpret_cast<std::uintptr_t>(state),access.Reads());}
    else if(++bundleSummaryBots.refused==1)cfgclosedtrace::Mark("summary-offline-bot-filter-scope-refused",reinterpret_cast<std::uintptr_t>(state),access.Reads());
    return allowed;
}
bool PrepareBundleSummaryFilter(std::uintptr_t module){
    using namespace cfgsummaryfilter::disk;
    if(bundleSummaryFilterTarget)return bundleSummaryFilterEnabled&&bundleSummaryFilterTarget==module+Filter;
    Reader r;r.cap=64;
    if(!cfgbundle::Code(r,module,Filter,FilterCode)||!cfgbundle::Code(r,module,Refresh,RefreshCode))return false;
    void* original=nullptr;const auto target=module+Filter;
    if(MH_CreateHook(reinterpret_cast<void*>(target),reinterpret_cast<void*>(&BundleNativeSummaryFilter),&original)!=MH_OK)return false;
    bundleSummaryFilterOriginal=reinterpret_cast<SummaryFilterOriginal>(original);bundleSummaryFilterTarget=target;MemoryBarrier();
    if(MH_EnableHook(reinterpret_cast<void*>(target))==MH_OK){bundleSummaryFilterEnabled=true;return true;}
    MH_DisableHook(reinterpret_cast<void*>(target));return false;
}
bool __fastcall BundleNativeSummaryRowReady(void* row){
    const bool native=bundleSummaryRowReadyOriginal&&bundleSummaryRowReadyOriginal(row);
    if(native||!BundleGameplayStarted()||GetCurrentThreadId()!=plan.threadId)return native;
    BundleAccess access;bool ready=false;
    __try{ready=access.SummaryRowReady(reinterpret_cast<std::uintptr_t>(row));}
    __except(EXCEPTION_EXECUTE_HANDLER){ready=false;}
    return ready; // native outcomes/portraits remain authoritative; no PS writes
}
bool PrepareBundleSummaryRowReady(std::uintptr_t module){
    using namespace cfgsummaryrow::disk;
    if(bundleSummaryRowReadyTarget)return bundleSummaryRowReadyEnabled&&bundleSummaryRowReadyTarget==module+Ready;
    Reader r;r.cap=64;if(!cfgbundle::Code(r,module,Ready,ReadyCode))return false;
    void* original=nullptr;const auto target=module+Ready;
    if(MH_CreateHook(reinterpret_cast<void*>(target),reinterpret_cast<void*>(&BundleNativeSummaryRowReady),&original)!=MH_OK)return false;
    bundleSummaryRowReadyOriginal=reinterpret_cast<SummaryRowReadyOriginal>(original);bundleSummaryRowReadyTarget=target;MemoryBarrier();
    if(MH_EnableHook(reinterpret_cast<void*>(target))==MH_OK){bundleSummaryRowReadyEnabled=true;return true;}
    MH_DisableHook(reinterpret_cast<void*>(target));return false;
}
#include "SummaryPresentationRuntime.hpp"
bool PrepareBundleSummaryPresentation(std::uintptr_t module){
    using namespace cfgsummarypresentation::disk;
    if(bundleSummaryPresentationEnabled)return true;
    Reader r;r.cap=64;
    if(!cfgbundle::Code(r,module,StateReady,StateReadyCode)||!cfgbundle::Code(r,module,StatusText,StatusTextCode)||
       !cfgbundle::Code(r,module,StatusPanel,StatusPanelCode)||!cfgbundle::Code(r,module,CharacterPanel,CharacterPanelCode))return false;
    const std::array<std::uintptr_t,4> rvas{{StatusText,StatusPanel,CharacterPanel,StateReady}};
    const std::array<void*,4> detours{{reinterpret_cast<void*>(&BundleNativeSummaryText),reinterpret_cast<void*>(&BundleNativeSummaryPanel),
        reinterpret_cast<void*>(&BundleNativeSummaryCharacter),reinterpret_cast<void*>(&BundleNativeSummaryStateReady)}};
    std::array<void*,4> originals{};unsigned created=0;
    for(;created<rvas.size();++created)if(MH_CreateHook(reinterpret_cast<void*>(module+rvas[created]),detours[created],&originals[created])!=MH_OK)break;
    if(created!=rvas.size()){
        for(unsigned i=0;i<created;++i)MH_RemoveHook(reinterpret_cast<void*>(module+rvas[i]));return false;}
    bundleSummaryTextOriginal=reinterpret_cast<SummaryTextOriginal>(originals[0]);
    bundleSummaryPanelOriginal=reinterpret_cast<SummaryPanelOriginal>(originals[1]);
    bundleSummaryCharacterOriginal=reinterpret_cast<SummaryPanelOriginal>(originals[2]);
    bundleSummaryStateReadyOriginal=reinterpret_cast<SummaryStateReadyOriginal>(originals[3]);MemoryBarrier();
    // Proven native-startup overlap only: this 502-byte body is the same
    // StateReady body (whose summary pin includes ten trailing CC bytes).
    static_assert(StateReady==cfgmenu::populationdisk::HasFullyTraveled);
    static_assert([]()constexpr{for(unsigned i=0;i<cfgmenu::populationdisk::HasFullyTraveledCode.size();++i)
        if(StateReadyCode[i]!=cfgmenu::populationdisk::HasFullyTraveledCode[i])return false;return true;}());
    cfgownednative::CurrentProcess hookReader;
    const auto readyTarget=module+StateReady;
    if(!cfgownednative::resident.Prepare(hookReader,module,readyTarget,cfgmenu::populationdisk::HasFullyTraveledCode,
        reinterpret_cast<std::uintptr_t>(originals[3]),reinterpret_cast<std::uintptr_t>(detours[3]),6)){
        for(unsigned i=0;i<rvas.size();++i)MH_RemoveHook(reinterpret_cast<void*>(module+rvas[i]));return false;}
    unsigned enabled=0;for(;enabled<rvas.size();++enabled)if(MH_EnableHook(reinterpret_cast<void*>(module+rvas[enabled]))!=MH_OK)break;
    if(enabled==rvas.size()&&cfgownednative::resident.Enable(hookReader,readyTarget)){bundleSummaryPresentationEnabled=true;return true;}
    cfgownednative::resident.Disable(readyTarget);
    for(unsigned i=0;i<rvas.size();++i){MH_DisableHook(reinterpret_cast<void*>(module+rvas[i]));MH_RemoveHook(reinterpret_cast<void*>(module+rvas[i]));}
    return false;
}
std::uintptr_t bundleDifficultyHookTarget=0;
bool bundleDifficultyHookEnabled=false;
unsigned bundleDifficultyScaled=0;
unsigned bundleDifficultyMaxReads=0;
#ifdef CFG_FRONTEND_BUNDLE
float FrontendHumanJasonDifficultyDamage(std::uintptr_t,float,std::uintptr_t,std::uintptr_t)noexcept;
#ifdef CFG_H1_JASON
float HostedDifficultyDamage(std::uintptr_t,float,std::uintptr_t,std::uintptr_t)noexcept;
#endif
#endif
float __fastcall BundleNativeCounselorDamage(void* victim,float incoming,void* event,void* instigator,void* causer){
    float applied=incoming;
#ifdef CFG_FRONTEND_BUNDLE
    // Human-Jason owns an independent native OfflineBots witness. Never adopt
    // bundleJason or apply both difficulty routes to the same hit.
    applied=FrontendHumanJasonDifficultyDamage(reinterpret_cast<std::uintptr_t>(victim),incoming,
        reinterpret_cast<std::uintptr_t>(instigator),reinterpret_cast<std::uintptr_t>(causer));
#ifdef CFG_H1_JASON
    if(applied==incoming)applied=HostedDifficultyDamage(reinterpret_cast<std::uintptr_t>(victim),incoming,
        reinterpret_cast<std::uintptr_t>(instigator),reinterpret_cast<std::uintptr_t>(causer));
#endif
#endif
    if(applied==incoming&&GetCurrentThreadId()==plan.threadId&&BundleGameplayStarted()&&bundleSchedule.active&&bundleConfiguration.difficulty!=1){
        BundleAccess access;
        applied=access.DifficultyDamage(bundleConfiguration.difficulty,reinterpret_cast<std::uintptr_t>(victim),incoming,
            reinterpret_cast<std::uintptr_t>(instigator),reinterpret_cast<std::uintptr_t>(causer));
        bundleDifficultyMaxReads=(std::max)(bundleDifficultyMaxReads,access.Reads());
        if(applied!=incoming){++bundleDifficultyScaled;
            if(CFG_J3_HARD_MOVEMENT){if(bundleConfiguration.difficulty==0)++bundleReport.j3EasyDamageScaled;
                if(bundleConfiguration.difficulty==2)++bundleReport.j3HardDamageScaled;}}
    }
#if CFG_H283_F1_KILL_DIAG
    // H2.83 report only: the same single original call with the same arguments; a Jason causer adds two guarded health reads around it.
    {const auto c=reinterpret_cast<std::uintptr_t>(causer);
     const unsigned role=!c?2u:c==bundleJason.pawn.object?0u:c==bundleAdditional.actor.pawn.object?1u:2u;
     if(role<2&&bundleDamageOriginal&&GetCurrentThreadId()==plan.threadId){
        Reader probe;probe.cap=2;float before=-1,after=-1;const auto at=reinterpret_cast<std::uintptr_t>(victim)+cfgdonorgrab::h283::Health;
        if(!probe.Read(at,before))before=-1;
        const float result=bundleDamageOriginal(victim,applied,event,instigator,causer);
        if(!probe.Read(at,after))after=-1;
        auto& k=bundleReport.h283Kill[role];++k.hits;k.hitsZero+=!(result>0.f);
        if(incoming>=1000.f){++k.kills;++(result>0.f?k.killsApplied:k.killsZero);k.lastKillIncoming=incoming;k.lastKillResult=result;
            k.lastKillHealthBefore=before;k.lastKillHealthAfter=after;
            k.lastKillOwnInstigator=reinterpret_cast<std::uintptr_t>(instigator)==(role?bundleAdditional.actor.killerController.object:bundleJason.killerController.object)?1u:0u;
            cfgclosedtrace::Mark(result>0.f?"h283-kill-damage-applied":"h283-kill-damage-zero",c,reinterpret_cast<std::uintptr_t>(victim));}
        return result;}}
#endif
    return bundleDamageOriginal?bundleDamageOriginal(victim,applied,event,instigator,causer):0.f;
}
bool PrepareBundleDifficultyHook(std::uintptr_t module){
    if(bundleDifficultyHookTarget)return bundleDifficultyHookEnabled&&bundleDifficultyHookTarget==module+cfgdifficulty::disk::TakeDamage;
    Reader r;r.cap=64;
    if(!cfgbundle::Code(r,module,cfgdifficulty::disk::TakeDamage,cfgdifficulty::disk::TakeDamageEntry))return false;
    void* original=nullptr;const auto target=module+cfgdifficulty::disk::TakeDamage;
    if(MH_CreateHook(reinterpret_cast<void*>(target),reinterpret_cast<void*>(&BundleNativeCounselorDamage),&original)!=MH_OK)return false;
    bundleDamageOriginal=reinterpret_cast<cfgdifficulty::TakeDamageOriginal>(original);bundleDifficultyHookTarget=target;MemoryBarrier();
    if(MH_EnableHook(reinterpret_cast<void*>(target))==MH_OK){bundleDifficultyHookEnabled=true;return true;}
    MH_DisableHook(reinterpret_cast<void*>(target));return false;
}
int __fastcall BundleSweaterCanInteract(void* item,void* counselor,const cfgkill::EligibilityVector* location,const cfgkill::EligibilityVector* direction){
    using Predicate=int(__fastcall*)(void*,void*,const cfgkill::EligibilityVector*,const cfgkill::EligibilityVector*);
    const auto module=reinterpret_cast<std::uintptr_t>(GetModuleHandleW(nullptr));
    const auto original=reinterpret_cast<Predicate>(module+cfgsweater::pickupdisk::Original);
    auto& s=bundleSweaterPickup;
    if(GetCurrentThreadId()!=plan.threadId||!bundleSchedule.active||s.busy||
       !cfgsweater::PickupScope(s,module,plan.world,reinterpret_cast<std::uintptr_t>(item)))
        return original(item,counselor,location,direction);
    s.busy=true;s.lastInteractor=reinterpret_cast<std::uintptr_t>(counselor);BundleAccess access;bool allowed=false;
    __try{allowed=access.SweaterPickupAllowed(reinterpret_cast<std::uintptr_t>(item),reinterpret_cast<std::uintptr_t>(counselor));}
    __except(EXCEPTION_EXECUTE_HANDLER){allowed=false;}
    // Only the gender restriction is removed; native ownership, inventory,
    // busy and disabled predicates still run. Never report unconditional success.
    const auto predicate=allowed?reinterpret_cast<Predicate>(module+cfgsweater::pickupdisk::Base):original;
    if(allowed)++s.accepted;else ++s.refused;
    int result=0;
    __try{result=predicate(item,counselor,location,direction);}
    __finally{s.busy=false;}
    s.lastResult=result;
    if(allowed){if(result)++s.basePassed;else{++s.baseDenied;s.lastFailure=11;}}
    return result;
}
int __fastcall BundleKillEligibility(void* component,void* counselor,const cfgkill::EligibilityVector* location,const cfgkill::EligibilityVector* direction){
    const auto original=bundleKillOriginal;if(!original)return 0;
    const auto c=reinterpret_cast<std::uintptr_t>(component),pawn=reinterpret_cast<std::uintptr_t>(counselor);
    // One owned native component only. All retired/unrelated callers pass through.
    if(GetCurrentThreadId()!=plan.threadId||c!=bundleKillHookComponent||!bundleKill.policy.Exclusive(GetTickCount64()))
        return original(component,counselor,location,direction);
    if(!bundleKillEligibilityAccess){
        if(bundleKill.policy.PamelaAccepted()){
            BundleAccess native(1536);int allowed=0;
            if(native.NativePamelaEligibility(c,pawn,original,location,direction,allowed))return allowed;
        }
        const auto result=original(component,counselor,location,direction);
        if(result&&bundleKill.policy.PamelaAccepted()&&!bundleKill.pamelaStock.pawn){
            // Observation of a REAL stock query, not a synthetic approval or
            // a relaxed human query. Weak evidence is fully revalidated before
            // selection; this callback performs no native call or override.
            BundleAccess observed(384);
            observed.ObservePamelaStock(c,pawn,result,GetTickCount64());
        }
        return result;
    }
    if(!cfgkill::MayRelax(bundleKill.policy,bundleKillTicket,c,pawn,GetTickCount64())||
        !bundleKillEligibilityAccess->EligibilityCurrent(c,pawn))
        return original(component,counselor,location,direction);
    if(bundleKill.policy.PamelaAccepted()){
        int result=0;
        if(bundleKillEligibilityAccess->PamelaEligibilityCall(bundleKill.policy,c,pawn,original,location,direction,result))return result;
        return original(component,counselor,location,direction); // unknown/changed list cannot grant the override
    }
    return cfgkill::CallEligibilityWithHunter(original,component,counselor,location,direction,
        reinterpret_cast<volatile char*>(pawn+cfgkill::disk::bIsHunter));
}
void BundleDamageAfter(void* object,void* function,void* parameters,bool completed){
    if(!completed||GetCurrentThreadId()!=plan.threadId||!BundleGameplayStarted()||
        reinterpret_cast<std::uintptr_t>(object)!=bundleJason.pawn.object||
        (bundleKill.policy.stage!=cfgkill::Stage::AwaitHit&&bundleKill.policy.stage!=cfgkill::Stage::NativeContext&&
         !(bundleKill.policy.pamela&&GetTickCount64()<bundleKill.policy.until)))return;
    if(bundleKill.policy.pamela){
        const bool ownsPublication=(bundleReport.generation&1u)==0;
        if(ownsPublication)InterlockedIncrement(reinterpret_cast<volatile LONG*>(&bundleReport.generation));
        BundleAccess access;
        access.TracePamelaDamageFunction(reinterpret_cast<std::uintptr_t>(function),GetTickCount64());
        bundleKill.ObservePamelaEvent(access,reinterpret_cast<std::uintptr_t>(object),reinterpret_cast<std::uintptr_t>(function),reinterpret_cast<std::uintptr_t>(parameters),GetTickCount64(),true);
        if(bundleKill.policy.PamelaAccepted()&&!bundleKill.policy.pamelaNativeContextOnly&&bundleKill.policy.wearer==bundleSweaterOwner.object&&
           bundleKill.policy.until<UINT64_MAX&&bundleSweaterPolicy.rearmAt<=bundleKill.policy.until)
            bundleSweaterPolicy.rearmAt=bundleKill.policy.until+1;
        bundleReport.killAcceptedHits=bundleKill.accepted;bundleReport.killStage=static_cast<unsigned>(bundleKill.policy.stage);
        bundleReport.reads+=access.Reads();bundleReport.calls+=access.Calls();
        bundleReport.maxReads=(std::max)(bundleReport.maxReads,access.Reads());
        bundleReport.maxCalls=(std::max)(bundleReport.maxCalls,access.Calls());
        if(ownsPublication){MemoryBarrier();InterlockedIncrement(reinterpret_cast<volatile LONG*>(&bundleReport.generation));}
        return;
    }
    if(!bundleDamageGate.Matches(reinterpret_cast<std::uintptr_t>(function))){
        // Observe only the independently cached native damage event, not all
        // unrelated ProcessEvents. No factory is called to populate this hint.
        Reader hint;hint.cap=1;std::uintptr_t native=0;
        if(hint.Read(plan.module+cfgkill::disk::ReceiveAnyDamage.cache,native)&&native&&native==reinterpret_cast<std::uintptr_t>(function)){
            BundleAccess diagnostic;cfgkilldiag::Hit hit;hit.function=native;hit.clause=2;diagnostic.TraceKillHit(hit);
        }
        return;
    }
    // A native call can deliver a nested damage notification while the outer
    // AI phase is already publishing. Keep its seqlock odd until that outer
    // phase finishes; never expose a half-updated report as stable.
    const bool ownsPublication=(bundleReport.generation&1u)==0;
    if(ownsPublication)InterlockedIncrement(reinterpret_cast<volatile LONG*>(&bundleReport.generation));
    LARGE_INTEGER started{},finished{},frequency{};QueryPerformanceCounter(&started);QueryPerformanceFrequency(&frequency);
    BundleAccess access;
    bundleKill.ObserveDamage(access,reinterpret_cast<std::uintptr_t>(object),reinterpret_cast<std::uintptr_t>(function),
        reinterpret_cast<std::uintptr_t>(parameters),GetTickCount64(),true);
    bundleReport.killAcceptedHits=bundleKill.accepted;bundleReport.killStage=static_cast<unsigned>(bundleKill.policy.stage);
    bundleReport.reads+=access.Reads();bundleReport.calls+=access.Calls();
    bundleReport.maxReads=(std::max)(bundleReport.maxReads,access.Reads());bundleReport.maxCalls=(std::max)(bundleReport.maxCalls,access.Calls());
    QueryPerformanceCounter(&finished);
    if(frequency.QuadPart>0){const auto elapsed=static_cast<std::uint64_t>((finished.QuadPart-started.QuadPart)*1000000/frequency.QuadPart);
        bundleReport.maxMicroseconds=(std::max)(bundleReport.maxMicroseconds,elapsed);}
    if(ownsPublication){MemoryBarrier();InterlockedIncrement(reinterpret_cast<volatile LONG*>(&bundleReport.generation));}
}
bool BundleNativeMeleeContact(void* object){
    if(GetCurrentThreadId()!=plan.threadId||!BundleGameplayStarted()||bundleKill.policy.stage!=cfgkill::Stage::NativeContext||
       bundleKill.policy.interactionAttempted||bundleKill.policy.finishingStrike||bundleKill.policy.nativeActiveObserved||
       reinterpret_cast<std::uintptr_t>(object)!=bundleKill.policy.finisher)return false;
    const auto now=GetTickCount64();if(now<bundleMeleeNativeTracker.next)return false;
    bundleMeleeNativeTracker.next=now+17;
    cfgkillcontact::Proof proof;BundleAccess traceAccess;
    if(!traceAccess.BeginMeleeTrace(reinterpret_cast<std::uintptr_t>(object),0,proof))return true;
    const bool contact=bundleMeleeNativeTracker.Consider(proof);
    static unsigned baselines=0;if(!contact&&baselines++<2)cfgclosedtrace::Mark("MeleeNativeBaseline",proof.source.object,traceAccess.Reads());
    const bool ownsPublication=(bundleReport.generation&1u)==0;
    if(ownsPublication)InterlockedIncrement(reinterpret_cast<volatile LONG*>(&bundleReport.generation));
    BundleAccess access;const bool accepted=contact&&access.MeleeContinuationBudget(proof)&&bundleKill.ObserveMeleeContact(access,proof,now);
    bundleReport.killAcceptedHits=bundleKill.accepted;bundleReport.killStage=static_cast<unsigned>(bundleKill.policy.stage);
    const auto reads=traceAccess.Reads()+access.Reads()+1;
    bundleReport.reads+=reads;bundleReport.calls+=access.Calls();
    bundleReport.maxReads=(std::max)(bundleReport.maxReads,reads);bundleReport.maxCalls=(std::max)(bundleReport.maxCalls,access.Calls());
    static unsigned records=0;if(contact&&records++<24)cfgclosedtrace::Mark(accepted?"MeleeNativeContactAccepted":"MeleeNativeContactRefused",proof.source.object,reads);
    if(ownsPublication){MemoryBarrier();InterlockedIncrement(reinterpret_cast<volatile LONG*>(&bundleReport.generation));}
    return true; // Do not stack reflected trace observation on this callback's native sample.
}
void BundleMeleeContactBefore(void* object,void* function,cfgkillcontact::Proof& proof){
    if(GetCurrentThreadId()!=plan.threadId||!BundleGameplayStarted()||
       bundleKill.policy.stage!=cfgkill::Stage::NativeContext||bundleKill.policy.finishingStrike||
       bundleKill.policy.interactionAttempted||reinterpret_cast<std::uintptr_t>(object)!=bundleKill.policy.finisher)return;
    Reader hint;hint.cap=1;std::uintptr_t trace=0;
    if(!hint.Read(plan.module+cfgkill::contactdisk::AttackTrace.cache,trace)||!trace||trace!=reinterpret_cast<std::uintptr_t>(function))return;
    BundleAccess access;
    const bool armed=access.BeginMeleeTrace(reinterpret_cast<std::uintptr_t>(object),trace,proof);
    static unsigned records=0;if(records++<24)cfgclosedtrace::Mark(armed?"MeleeTraceBefore":"MeleeTraceBeforeRefused",
        reinterpret_cast<std::uintptr_t>(object),access.Reads());
}
void BundleMeleeContactAfter(cfgkillcontact::Proof& proof,bool completed){
    if(!completed||!proof.armed||GetCurrentThreadId()!=plan.threadId||!BundleGameplayStarted())return;
    const bool ownsPublication=(bundleReport.generation&1u)==0;
    if(ownsPublication)InterlockedIncrement(reinterpret_cast<volatile LONG*>(&bundleReport.generation));
    BundleAccess access;const auto now=GetTickCount64();
    const bool contact=access.FinishMeleeTrace(proof);
    const bool accepted=contact&&bundleKill.ObserveMeleeContact(access,proof,now);
    bundleReport.killAcceptedHits=bundleKill.accepted;bundleReport.killStage=static_cast<unsigned>(bundleKill.policy.stage);
    const unsigned reads=proof.beforeReads+access.Reads()+1;
    bundleReport.reads+=reads;bundleReport.calls+=access.Calls();
    bundleReport.maxReads=(std::max)(bundleReport.maxReads,reads);bundleReport.maxCalls=(std::max)(bundleReport.maxCalls,access.Calls());
    static unsigned records=0;if(records++<24)cfgclosedtrace::Mark(accepted?"MeleeContactAccepted":contact?"MeleeContactRefused":"MeleeNoNewTargetContact",
        proof.source.object,reads);
    if(ownsPublication){MemoryBarrier();InterlockedIncrement(reinterpret_cast<volatile LONG*>(&bundleReport.generation));}
}
void BundleStop(cfgbundle::Stop reason){
    bundleMeleeNativeTracker.Reset();
    bundleObjectiveRetiring=true;
    {BundleAccess cleanup;cleanup.RetireSweaterPickup();}
    {BundleAccess cleanup;cleanup.PracticeCleanup();cleanup.SoundBlipPhase(true);}
    {BundleAccess cleanup;cleanup.RBClose(!bundleKill.policy.Exclusive(GetTickCount64())&&!bundleDeathEnd.confirmedAt);}
    // Terminal cleanup has a fresh bounded reader per owned brain and no actor scan.
    for(unsigned i=0;i<bundleReport.bots;++i)if(bundleConvergenceTravel[i].policy.owned){
        BundleAccess cleanup;if(cleanup.Context())cleanup.TravelResume(i,GetTickCount64(),false);
        else {bundleConvergenceTravel[i]={};++bundleTravelLost;}
    }
    for(unsigned i=0;i<bundleReport.bots;++i)if(BundleEmoteHeld(i)){
        BundleAccess cleanup;cleanup.MirrorResume(i);
    }
    if(bundleCarSpeed.active){BundleAccess cleanup;if(!cleanup.CarBoost(false))++bundleVehicleActions.refusals;}
    for(unsigned i=0;i<4&&bundleFootBurst.Owned();++i){BundleAccess cleanup;cleanup.FootBurstService(GetTickCount64(),true);}
    bundleFootBurstRequestAt=0;
    bundleSchedule.active=false;bundleReport.state=static_cast<unsigned>(cfgbundle::State::Stopped);bundleReport.stop=static_cast<unsigned>(reason);
    bundleTrapPriority.Clear();bundleTargetLockUntil=0;
}
void BundleObjectiveReport(){
    const auto& s=bundleObjectives.Status();
    bundleReport.objectiveScanned=s.scanned;bundleReport.objectiveProcessed=s.processed;
    bundleReport.objectiveChanged=s.changed;bundleReport.objectiveAlreadyBase=s.alreadyBase;bundleReport.objectiveRefused=s.refused;
    bundleReport.objectiveRestored=s.restored;bundleReport.objectiveRestoreRefused=s.restoreRefused;
    bundleReport.objectiveDeferred=s.deferred;bundleReport.objectivePeakReads=s.peakReads;
    bundleReport.objectiveFlags=(s.discoveryDone?1u:0u)|(s.expired?2u:0u)|(s.retired?4u:0u)|(s.indeterminate?8u:0u)|(bundleObjectiveRetiring?16u:0u);
    bundleReport.objectiveProcessedMask=s.processedMask;bundleReport.objectiveChangedMask=s.changedMask;
    bundleReport.objectiveBaseMask=s.baseMask;bundleReport.objectiveRefusedMask=s.refusedMask;
    const auto& r=bundleObjectiveRepair.report;
    bundleReport.objectiveRepairStage=static_cast<unsigned>(r.stage);bundleReport.objectiveRepairFailure=static_cast<unsigned>(r.failure);
    bundleReport.objectiveRepairHostFailure=bundleObjectiveRepairFailure;bundleReport.objectiveRepairTarget=r.target;
    bundleReport.objectiveRepairAttempts=r.attempts;bundleReport.objectiveRepairConfirmed=r.confirmed;bundleReport.objectiveRepairPreserved=r.preserved;
    bundleReport.objectiveRepairDeferred=r.deferred;bundleReport.objectiveRepairAttemptedMask=r.attemptedMask;
    bundleReport.objectiveRepairConfirmedMask=r.confirmedMask;bundleReport.objectiveRepairPreservedMask=r.preservedMask;
    bundleReport.objectiveRepairRefusedMask=r.refusedMask;bundleReport.objectiveInstances=bundleObjectives.InstanceCount();
    bundleReport.objectiveCandidate=bundleObjectiveCandidate;
}
void BundleObjectiveWidgetAfter(void* object,void* function,bool completed)noexcept{
    if(!completed||GetCurrentThreadId()!=plan.threadId||!bundleSchedule.active||bundleObjectiveRetiring||
       bundleObjectiveWidget.object||bundleObjectiveRepair.report.attemptedMask||!object||!function)return;
    // Native dispatcher supplied a UObject. A single guarded class-pointer
    // comparison cheaply rejects unrelated events; no per-event registry scan.
    std::uintptr_t type=0,owner=0;
    __try{type=*reinterpret_cast<const std::uintptr_t*>(reinterpret_cast<std::uintptr_t>(object)+cfgprobe::ObjectClass);
        if(type!=bundleObjectives.RootClass())owner=*reinterpret_cast<const std::uintptr_t*>(reinterpret_cast<std::uintptr_t>(function)+0x20);}
    __except(EXCEPTION_EXECUTE_HANDLER){return;}
    if(type!=bundleObjectives.RootClass()&&(!bundleObjectives.NeedsEventOwner()||owner!=type||bundleObjectives.EventTypeSeen(type)))return;
    const bool publish=(bundleReport.generation&1u)==0;
    if(publish)InterlockedIncrement(reinterpret_cast<volatile LONG*>(&bundleReport.generation));
    BundleAccess access;
    __try{access.ObjectiveWidgetEvent(reinterpret_cast<std::uintptr_t>(object),reinterpret_cast<std::uintptr_t>(function),GetTickCount64());}
    __except(EXCEPTION_EXECUTE_HANDLER){/* No native call or mutation attempted. */}
    BundleObjectiveReport();
    if(publish){MemoryBarrier();InterlockedIncrement(reinterpret_cast<volatile LONG*>(&bundleReport.generation));}
}
bool BundleSpectatorStatusBefore(void* object,void* function,void* parameters,cfgspectatorstatus::Frame& replacement)noexcept{
    if(!bundleSchedule.active||!BundleGameplayStarted()||GetCurrentThreadId()!=plan.threadId||
       !(bundleCompletion.terminalMask&1u)||!bundleSpectatorStatusNameReady||
       bundleSpectatorStatusWidgetHint!=reinterpret_cast<std::uintptr_t>(object))return false;
    Reader probe;probe.cap=2;cfgnames::Words eventName{};
    if(!probe.Read(reinterpret_cast<std::uintptr_t>(function)+cfgnames::disk::ObjectName,eventName)||!(eventName==bundleSpectatorStatusName))return false;
    const bool replacing=BundleNativeJasonLabelHint(); // hint only; full text/HUD/owner proof below
    const auto now=GetTickCount64();
    // Diagnostic pacing must never suppress a semantic label update.
    if(!replacing&&(now<bundleSpectatorStatusAt||bundleReport.spectatorStatusEvents+bundleReport.spectatorStatusRefusals>=64))return false;
    bundleSpectatorStatusAt=now+500;
    const bool publication=(bundleReport.generation&1u)==0;
    if(publication)InterlockedIncrement(reinterpret_cast<volatile LONG*>(&bundleReport.generation));
    BundleAccess access;bool accepted=false;
    __try{accepted=replacing?
        access.NativeJasonLabel(reinterpret_cast<std::uintptr_t>(object),reinterpret_cast<std::uintptr_t>(function),reinterpret_cast<std::uintptr_t>(parameters),replacement):
        access.SpectatorStatus(reinterpret_cast<std::uintptr_t>(object),reinterpret_cast<std::uintptr_t>(function),reinterpret_cast<std::uintptr_t>(parameters),nullptr);
        if(!accepted)
        ++bundleReport.spectatorStatusRefusals;}
    __except(EXCEPTION_EXECUTE_HANDLER){++bundleReport.spectatorStatusRefusals;}
    if(accepted)bundleJasonLabelOwned=replacing;
    // The caller forwards exactly once, with a separate synchronous frame.
    const unsigned reads=access.Reads()+probe.reads;
    bundleReport.reads+=reads;bundleReport.maxReads=(std::max)(bundleReport.maxReads,reads);
    if(publication){MemoryBarrier();InterlockedIncrement(reinterpret_cast<volatile LONG*>(&bundleReport.generation));}
    return replacing&&accepted;
}
Dispatch bundleGenericOriginal=nullptr;
thread_local bool bundleWidgetNested=false;
bool BundleEndingSummaryService(std::uint64_t now,unsigned readDebt=0)noexcept{
    if(bundleEndingSummary.servicing||!BundleSummaryStarted()||
       readDebt>=cfgbundle::ReadCap||!bundleEndingSummary.Probe(now))return false;
    struct EndSummaryGuard {bool& busy;EndSummaryGuard(bool& b):busy(b){busy=true;}~EndSummaryGuard(){busy=false;}}
        summaryGuard(bundleEndingSummary.servicing);
    BundleAccess phase((std::min)(192u,cfgbundle::ReadCap-readDebt));unsigned failure=0;
    if(!phase.SummaryEndingPhase(&failure)){
        if(failure==1&&bundleEndingSummary.scopeTraces++<4)
            cfgclosedtrace::Mark("ending-summary-scope-refused",plan.mode,phase.Reads());
        return false;
    }
    if(!cfgsummaryending::Admission(BundleSummaryStarted(),bundleSchedule.active,bundleSchedule.pressed,true))return false;
    if(!bundleEndingSummary.Due(now,true))return false;
    if(bundleEndingSummary.phaseTraces++<4)cfgclosedtrace::Mark("ending-summary-native-phase",plan.mode,
        static_cast<unsigned>(bundleSchedule.active)|(static_cast<unsigned>(bundleSchedule.pressed)<<1));
    InterlockedIncrement(reinterpret_cast<volatile LONG*>(&bundleReport.generation));
    BundleAccess presentation(cfgbundle::ReadCap-readDebt-phase.Reads());presentation.SummaryPhase(now,true);
    const auto reads=readDebt+phase.Reads()+presentation.Reads();
    bundleReport.reads+=reads;bundleReport.calls+=presentation.Calls();
    bundleReport.maxReads=(std::max)(bundleReport.maxReads,reads);
    MemoryBarrier();InterlockedIncrement(reinterpret_cast<volatile LONG*>(&bundleReport.generation));
    return true; // presentation only, never gameplay scheduling/input
}
void BundleSummaryWidgetOpportunity(void* object,bool nested,bool completed)noexcept{
    if(nested||!completed||!BundleSummaryStarted()||bundleEndingSummary.servicing||GetCurrentThreadId()!=plan.threadId)return;
    const auto now=GetTickCount64();if(now<bundleEndingSummary.probeAt||bundleEndingSummary.finished)return;
    Reader hint;hint.cap=16;
    if(cfgsummaryending::WidgetCandidate(hint,plan.controller,reinterpret_cast<std::uintptr_t>(object)))
        BundleEndingSummaryService(now,hint.reads);
}
#ifdef CFG_FRONTEND_BUNDLE
// Defined after the frontend host. Native widget returns are the donor's
// frontend-event fallback; never call gameplay scheduling from this bridge.
void FrontendEpochWidgetOpportunity(void*,void*,bool,bool)noexcept;
void BundleWidgetForwardForEpoch(void* object,void* function,void* parameters){
    const bool nested=bundleWidgetNested||observerNested;
    bundleWidgetNested=true;bool completed=false;
    __try{bundleGenericOriginal(object,function,parameters);completed=true;}
    __finally{
        FrontendEpochWidgetOpportunity(object,function,nested,completed);
        bundleWidgetNested=false;
        BundleSummaryWidgetOpportunity(object,nested,completed);
    }
}
#endif
void __fastcall BundleWidgetDispatcher(void* object,void* function,void* parameters){
    // Actor ProcessEvent does not transport UserWidget events. This bridge
    // never schedules gameplay, edits unrelated arguments or scans the world.
    if(!bundleGenericOriginal)return;
    if(bundleWidgetNested||GetCurrentThreadId()!=observerOwnerThread||!object||!function){bundleGenericOriginal(object,function,parameters);return;}
    const bool status=bundleSpectatorStatusWidgetHint==reinterpret_cast<std::uintptr_t>(object);
    bool hostedWorld=HostedRoutineActive();
    // Driver ownership intentionally ends before the hosted outro does. Use
    // the world's network-driver identity so pre-adoption and end-frozen
    // hosted widget events never fall back into offline J2.19 consumers.
    Reader worldProof;worldProof.cap=1;std::uintptr_t netDriver=0;
    if(plan.world&&worldProof.Read(plan.world+cfgprobe::WorldNetDriver,netDriver))hostedWorld|=netDriver!=0;
    const auto hostedRoute=cfghostedwidget::Select(hostedWorld,status);
    if(hostedRoute==cfghostedwidget::Route::ForwardOnly){
        bundleGenericOriginal(object,function,parameters);
        if(cfghostedwidget::SummaryOpportunity(hostedRoute,HostedSummaryWindow()))BundleSummaryWidgetOpportunity(object,false,true);
        return;}
    if(hostedRoute==cfghostedwidget::Route::SpectatorStatusOnly){
        if(!BundleGameplayStarted()||!bundleSchedule.active){bundleGenericOriginal(object,function,parameters);return;}
        bundleWidgetNested=true;cfgspectatorstatus::Frame replacement{};
        __try{
            const bool replaced=BundleSpectatorStatusBefore(object,function,parameters,replacement);
            cfgspectatorstatus::Forward(bundleGenericOriginal,object,function,parameters,replacement,replaced);
        }__finally{bundleWidgetNested=false;}
        return;
    }
    if(!BundleGameplayStarted()||!bundleSchedule.active){
#ifdef CFG_FRONTEND_BUNDLE
        BundleWidgetForwardForEpoch(object,function,parameters);
#else
        bundleGenericOriginal(object,function,parameters);
        BundleSummaryWidgetOpportunity(object,false,true);
#endif
        return;
    }
    if(J215HideHealth(object,function,parameters)){
        std::uint8_t hidden=0;
#ifdef CFG_FRONTEND_BUNDLE
        BundleWidgetForwardForEpoch(object,function,&hidden);
#else
        bundleGenericOriginal(object,function,&hidden);
#endif
        return;
    }
    bool objective=false;
    if(!status&&!bundleObjectiveRetiring&&!bundleObjectiveWidget.object&&!bundleObjectiveRepair.report.attemptedMask){
        std::uintptr_t type=0,owner=0;
        __try{type=*reinterpret_cast<const std::uintptr_t*>(reinterpret_cast<std::uintptr_t>(object)+cfgprobe::ObjectClass);
            owner=*reinterpret_cast<const std::uintptr_t*>(reinterpret_cast<std::uintptr_t>(function)+0x20);}
        __except(EXCEPTION_EXECUTE_HANDLER){type=owner=0;}
        objective=type&&((type==bundleObjectives.RootClass())||
            (owner==type&&bundleObjectives.NeedsEventOwner()&&!bundleObjectives.EventTypeSeen(type)));
    }
    if(!status&&!objective){
#ifdef CFG_FRONTEND_BUNDLE
        BundleWidgetForwardForEpoch(object,function,parameters);
#else
        bundleGenericOriginal(object,function,parameters);
        BundleSummaryWidgetOpportunity(object,false,true);
#endif
        return;
    }
    bundleWidgetNested=true;bool completed=false;cfgspectatorstatus::Frame replacement{};
    __try{
        const bool replaced=status&&BundleSpectatorStatusBefore(object,function,parameters,replacement);
        cfgspectatorstatus::Forward(bundleGenericOriginal,object,function,parameters,replacement,replaced);
        completed=true;
    }__finally{
        if(objective)BundleObjectiveWidgetAfter(object,function,completed);
#ifdef CFG_FRONTEND_BUNDLE
        FrontendEpochWidgetOpportunity(object,function,observerNested,completed);
#endif
        bundleWidgetNested=false;
        BundleSummaryWidgetOpportunity(object,observerNested,completed);
    }
}
bool PrepareBundleWidgetDispatcher(std::uintptr_t module){
    if(cfgdispatch::enabled.load(std::memory_order_acquire)){Reader r;r.cap=16;return cfgdispatch::Proof(r,module);}
    Reader r;r.cap=32;cfgdispatch::Hook proof{};void* original=nullptr;
    const auto target=module+cfgspawn::GenericDispatcherRva;
    if(!cfgdispatch::Proof(r,module)||MH_CreateHook(reinterpret_cast<void*>(target),
       reinterpret_cast<void*>(&BundleWidgetDispatcher),&original)!=MH_OK)return false;
    if(!cfgdispatch::Prepare(r,module,reinterpret_cast<std::uintptr_t>(original),
       reinterpret_cast<std::uintptr_t>(&BundleWidgetDispatcher),proof)){
        MH_RemoveHook(reinterpret_cast<void*>(target));return false;}
    bundleGenericOriginal=reinterpret_cast<Dispatch>(original);cfgdispatch::owned=proof;
    cfgdispatch::enabled.store(true,std::memory_order_release);
    if(MH_EnableHook(reinterpret_cast<void*>(target))==MH_OK&&cfgdispatch::Proof(r,module))return true;
    MH_DisableHook(reinterpret_cast<void*>(target));cfgdispatch::enabled.store(false,std::memory_order_release);
    MH_RemoveHook(reinterpret_cast<void*>(target));bundleGenericOriginal=nullptr;return false;
}
void BundleNativeCycleCompleted(void* controller,std::int32_t direction)noexcept{
    if(GetCurrentThreadId()!=plan.threadId)return;
    bundleSpectatorCycleController=reinterpret_cast<std::uintptr_t>(controller);bundleSpectatorCycleDirection=direction;
    bundleSpectatorCycleRejection=2;if(!BundleGameplayStarted()||!bundleSchedule.active||DonorFinalVictimOwnsCamera())return;
    bundleSpectatorCycleRejection=3;if(!cfgbundle::OfflineSpectatorCameraAllowed(HostedRoutineActive(),bundleCompletion.terminalMask))return;
    bundleSpectatorCycleRejection=4;if(reinterpret_cast<std::uintptr_t>(controller)!=plan.controller)return;
    bundleSpectatorCycleRejection=5;if(direction!=1&&direction!=-1)return;
    Reader r;r.cap=128;
    // Observation only; the camera consumer independently revalidates the
    // complete terminal/local-owner/live-roster scope before publication.
    BundleAccess observation;
    bundleSpectatorCycleRejection=6;if(!observation.SpectatorCycleObservationScope(
        reinterpret_cast<std::uintptr_t>(controller),direction))return;
    bundleSpectatorCycleRejection=7;if(!cfgobserver::Same(r,plan.viewport+cfgprobe::ViewportWorld,plan.world))return;
    bundleSpectatorCycleRejection=8;if(!cfgobserver::Same(r,plan.world+cfgprobe::WorldNetDriver,std::uintptr_t{}))return;
    bundleSpectatorCycleRejection=0;
    ++bundleSpectatorCycleEvents;++bundleSpectatorNativeCycles;
    // Coherent, bounded diagnostics after native completion; never authority
    // for a camera change. This includes stock spectator attachment identity.
    std::uintptr_t selected=0,camera=0,active=0,pending=0,spectator=0,viewing=0;
    if(r.Read(plan.controller+cfgspectator::disk::SpectatingPlayer,selected)&&
       r.Read(plan.controller+cfgspectatorview::disk::ControllerCamera,camera)&&cfgprobe::ObjectPointer(camera)&&
       r.Read(camera+cfgspectatorview::disk::Active,active)&&r.Read(camera+cfgspectatorview::disk::Pending,pending)){
        cfgclosedtrace::Mark("spectator-native-cycle-selected",selected,bundleSpectatorCycleEvents);
        cfgclosedtrace::Mark("spectator-native-cycle-camera",active,pending);
    }
    if(r.Read(plan.controller+cfgprobe::ControllerSpectatorPawn,spectator)&&cfgprobe::ObjectPointer(spectator)&&
       r.Read(spectator+0x3F8,viewing))cfgclosedtrace::Mark("spectator-native-cycle-attached",spectator,viewing);
}
void __fastcall BundleNativeViewAPlayer(void* controller,std::int32_t direction){
    InterlockedIncrement(&bundleSpectatorNativeEntries); // raw entry only; no camera authority
    const bool owned=GetCurrentThreadId()==plan.threadId&&BundleGameplayStarted()&&bundleSchedule.active&&
        reinterpret_cast<std::uintptr_t>(controller)==plan.controller;
    if(!owned||bundleSpectatorViewBusy){
        if(GetCurrentThreadId()==plan.threadId){bundleSpectatorCycleRejection=4;
            bundleSpectatorCycleController=reinterpret_cast<std::uintptr_t>(controller);bundleSpectatorCycleDirection=direction;}
        bundleSpectatorViewOriginal(controller,direction);return;
    }
    bundleSpectatorViewBusy=true;
    __try {bundleSpectatorViewOriginal(controller,direction);BundleNativeCycleCompleted(controller,direction);}
    __finally {bundleSpectatorViewBusy=false;}
}
void BundleSpectatorCycleAfter(void* object,void* function,bool nested,bool completed,bool nativeObserved=false)noexcept{
    if(!completed||nested||nativeObserved||!function||GetCurrentThreadId()!=plan.threadId||!bundleSchedule.active||
       !cfgbundle::OfflineSpectatorCameraAllowed(HostedRoutineActive(),bundleCompletion.terminalMask)||reinterpret_cast<std::uintptr_t>(object)!=plan.controller)return;
    const auto event=reinterpret_cast<std::uintptr_t>(function);
    for(const auto other:bundleSpectatorOtherFunctions)if(other==event)return;
    Reader reader;reader.cap=256;bool cycle=false;cfgnames::Text observedName{};
    BundleAccess observation;
    __try{if(!observation.SummaryObservationScope())return;
        cycle=cfgspectator::CompletedCycleEvent(reader,plan.module,plan.controller,event,&observedName);}
    __except(EXCEPTION_EXECUTE_HANDLER){return;}
    if(!cycle){
        std::array<char,96> label{};
        if(bundleSpectatorAlternateTraceCount<16&&cfgspectator::AlternateCycleDiagnostic(observedName.View(),label)){
            ++bundleSpectatorAlternateTraceCount;
            cfgclosedtrace::Mark(label.data(),event,bundleReport.spectatorObservedState);
        }
        if(reader.reads<reader.cap)bundleSpectatorOtherFunctions[bundleSpectatorOtherCursor++%bundleSpectatorOtherFunctions.size()]=event;
        return;
    }
    // Original native Next/Previous has already completed exactly once. This
    // monotonic hint performs no native call/write; the camera lane must still
    // prove the current offline owner, terminal human, roster and living Jason.
    ++bundleSpectatorCycleEvents;
}
bool BundleObjectiveLifecycle(bool nested){
    // Metadata restoration only. Never write into an old/new world or reuse a
    // stale UFunction: RetireOne repeats exact registry, owner and script proof.
    if(nested||!bundleObjectives.PendingRestore()||GetCurrentThreadId()!=bundleObjectiveThread||
       bundleObjectiveModule!=reinterpret_cast<std::uintptr_t>(GetModuleHandleW(nullptr)))return false;
    const auto now=GetTickCount64();if(now<bundleObjectiveCheck)return false;
    bundleObjectiveCheck=now+(bundleObjectiveRetiring?25:250);
    Reader reader;reader.cap=cfgbundle::ReadCap;
    if(!bundleObjectiveRetiring){
        if(bundleSchedule.active&&cfgobserver::Same(reader,bundleObjectiveViewport+cfgprobe::ViewportWorld,bundleObjectiveWorld)&&
           cfgobserver::Same(reader,bundleObjectiveWorld+cfgprobe::WorldNetDriver,std::uintptr_t{}))return false;
        bundleObjectiveRetiring=true;
    }
    const bool publication=(bundleReport.generation&1u)==0;
    if(publication)InterlockedIncrement(reinterpret_cast<volatile LONG*>(&bundleReport.generation));
    LARGE_INTEGER started{},finished{},frequency{};QueryPerformanceCounter(&started);QueryPerformanceFrequency(&frequency);
    struct Host{Reader& reader;
        bool Current(){return GetCurrentThreadId()==bundleObjectiveThread&&
            bundleObjectiveModule==reinterpret_cast<std::uintptr_t>(GetModuleHandleW(nullptr))&&
            cfgprimary::LookupSafe(reader,bundleObjectiveModule);}
        bool Exchange(std::uintptr_t at,std::uintptr_t expected,std::uintptr_t value){return cfgobjective::CompareWrite(at,expected,value);}
    }host{reader};
    bundleObjectives.RetireOne(reader,bundleObjectiveModule,host);
    BundleObjectiveReport();bundleReport.reads+=reader.reads;
    bundleReport.maxReads=(std::max)(bundleReport.maxReads,reader.reads);bundleReport.error=reader.error;
    QueryPerformanceCounter(&finished);
    if(frequency.QuadPart>0)bundleReport.maxMicroseconds=(std::max)(bundleReport.maxMicroseconds,
        static_cast<std::uint64_t>((finished.QuadPart-started.QuadPart)*1000000/frequency.QuadPart));
    if(publication){MemoryBarrier();InterlockedIncrement(reinterpret_cast<volatile LONG*>(&bundleReport.generation));}
    return true; // This callback cannot stack restoration with gameplay work.
}
void BundleEmoteReport(){
    const auto& r=bundleEmotes.report;
    bundleReport.emotePrepared=r.prepared;bundleReport.emoteStarts=r.starts;bundleReport.emoteQueued=r.queued;
    bundleReport.emoteAttempts=r.attempts;bundleReport.emoteReturns=r.returns;bundleReport.emoteStarted=r.started;
    bundleReport.emoteStartedUnchanged=r.startedUnchanged;bundleReport.emoteStartedOtherMontage=r.startedOtherMontage;
    bundleReport.emoteStartedReadRefused=r.startedReadRefused;
    bundleReport.emoteBindClause=r.functionBindClause;bundleReport.emoteBindReads=r.functionBindReads;
    bundleReport.emoteFailSite=r.failSite;bundleReport.emoteFailClause=r.failClause;bundleReport.emoteFailBindReads=r.failBindReads;
    bundleReport.emoteSkipped=r.skipped;bundleReport.emoteCancels=r.cancels;bundleReport.emoteRefused=r.refused;bundleReport.emoteFailure=r.failure;
    bundleReport.emoteCancelAttempts=r.cancelAttempts;bundleReport.emoteCancelReturns=r.cancelReturns;
    bundleReport.emoteStopped=r.stopped;bundleReport.emoteOwnershipLost=r.ownershipLost;
#if CFG_J21_EMOTE_MIRROR
    bundleReport.j2EmoteDeduplicated=bundleEmotes.deduplicated;bundleReport.j2EmoteExpiries=bundleEmotes.playbackExpiries;
    {static unsigned seen=0;if(bundleEmotes.cancelClears!=seen){seen=bundleEmotes.cancelClears;cfgclosedtrace::Mark("h258-emote-queue-cancel-cleared",bundleEmotes.report.queued,bundleEmotes.cancelClearMs);}}
    bundleReport.j2EmoteWorldCancels=bundleEmotes.worldCancels;bundleReport.j2EmoteRefusals=bundleEmotes.refusals;
    bundleReport.j23EmoteReadDeferrals=bundleEmotes.readDeferrals;bundleReport.j23EmoteBudgetDropped=bundleEmotes.budgetDropped;
    const auto& selected=bundleEmotes.Selection();
    const std::array<cfgemotemirror::Slot,4> assets{{selected.type,selected.defaults,selected.male,selected.female}};
    for(unsigned i=0;i<assets.size();++i)if(bundleReport.j2EmoteAssets[i]!=assets[i].object){
        bundleReport.j2EmoteAssets[i]=assets[i].object;bundleReport.j2EmotePaths[i]={};
        Reader proof;proof.cap=512;cfgprobe::lifetime::ArrayHeader registry{};cfgprobe::lifetime::SlotObservation fresh{};
        if(assets[i].object&&cfgprobe::lifetime::ReadArrayHeader(proof,plan.module+cfgprobe::lifetime::ObjectArrayRva,registry)&&
            cfgspawn::ObjectSlot(proof,registry,assets[i].object,fresh)&&fresh==assets[i])
            cfgobjectpath::Observe(proof,plan.module,assets[i].object,bundleReport.j2EmotePaths[i]);
    }
#endif
}
void BundleEmoteAccessReport(const BundleAccess& access){
    bundleReport.reads+=access.Reads();bundleReport.calls+=access.Calls();
    bundleReport.maxReads=(std::max)(bundleReport.maxReads,access.Reads());
    bundleReport.maxCalls=(std::max)(bundleReport.maxCalls,access.Calls());bundleReport.error=access.Error();
    BundleEmoteReport();
}
#ifdef CFG35_URINATION_NATIVE_PORT
void BundleUrinationSelection(void* object,void* function,void* parameters,bool nested)noexcept;
#endif
#if !CFG_J21_EMOTE_MIRROR
void BundleEmoteBefore(void* object,void* function,void* parameters,bool nested)noexcept{
    if(bundleRBDispatch&&reinterpret_cast<std::uintptr_t>(object)==bundleHumanTarget.object&&
        bundleEmotes.IsPlayFunction(reinterpret_cast<std::uintptr_t>(function)))nested=false;
#ifdef CFG35_URINATION_NATIVE_PORT
    BundleUrinationSelection(object,function,parameters,nested);
#endif
    if(!bundleSchedule.active||!BundleGameplayStarted()||GetCurrentThreadId()!=plan.threadId||
       !bundleEmotes.Candidate(reinterpret_cast<std::uintptr_t>(object),reinterpret_cast<std::uintptr_t>(function),nested))return;
    const bool ownsPublication=(bundleReport.generation&1u)==0;
    if(ownsPublication)InterlockedIncrement(reinterpret_cast<volatile LONG*>(&bundleReport.generation));
    Reader reader;reader.cap=cfgbundle::ReadCap;
#if CFG_H245_EMOTE && defined(CFG_H1_JASON)
    reader.cap=cfghostedbudget::EmoteReadCap(HostedRoutineActive(),reader.cap); // one shared parameterised proof budget, H2.45
#endif
    cfgemoteobservation::Access<Reader> host(reader,plan);
    auto input=BundleAccess{}.MirrorInputs();
    cfgemotemirror::NativeAccess<Reader,decltype(host)> access(reader,host,input);
    __try{bundleEmotes.Before(access,reinterpret_cast<std::uintptr_t>(object),reinterpret_cast<std::uintptr_t>(function),
        reinterpret_cast<std::uintptr_t>(parameters),nested);
        if(bundleEmotes.ObservedPlay(reinterpret_cast<std::uintptr_t>(object),reinterpret_cast<std::uintptr_t>(function)))
            bundleRBEmote.ObserveSelection(reinterpret_cast<std::uintptr_t>(object));}
    __except(EXCEPTION_EXECUTE_HANDLER){++bundleEmotes.report.refused;bundleEmotes.report.failure=1;}
    bundleReport.emoteHookPeakReads=(std::max)(bundleReport.emoteHookPeakReads,reader.reads);bundleReport.reads+=reader.reads;bundleReport.maxReads=(std::max)(bundleReport.maxReads,reader.reads);BundleEmoteReport();
    if(ownsPublication){MemoryBarrier();InterlockedIncrement(reinterpret_cast<volatile LONG*>(&bundleReport.generation));}
}
#else
void BundleEmoteBefore(void* object,void* function,void* parameters,bool nested)noexcept{
    if(bundleRBDispatch&&reinterpret_cast<std::uintptr_t>(object)==bundleHumanTarget.object&&
        bundleEmotes.IsPlayFunction(reinterpret_cast<std::uintptr_t>(function)))nested=false;
#ifdef CFG35_URINATION_NATIVE_PORT
    BundleUrinationSelection(object,function,parameters,nested);
#endif
    if(!cfgj21::EmotesEnabled){
        // Preserve natural RB selection suppression without warming/pumping C.
        // This is read-only, only while the local wheel is owned, on its exact
        // native PlayEmote event. No recipients, retention or native dispatch.
        if(BundleGameplayStarted()&&bundleSchedule.active&&GetCurrentThreadId()==plan.threadId&&
           bundleRBEmote.policy.Owned()&&reinterpret_cast<std::uintptr_t>(object)==bundleRBEmote.policy.Owner().pawn.object){
            Reader reader;reader.cap=cfgbundle::ReadCap;
#if CFG_H245_EMOTE && defined(CFG_H1_JASON)
    reader.cap=cfghostedbudget::EmoteReadCap(HostedRoutineActive(),reader.cap); // one shared parameterised proof budget, H2.45
#endif
            cfgemoteobservation::Access<Reader> host(reader,plan);
            cfgemotemirror::Inputs input;input.module=plan.module;input.world=plan.world;input.controller=plan.controller;
            input.controllerSlot=plan.objects[2];input.counselorClass=bundleCounselorClass;
            cfgemotemirror::NativeAccess<Reader,decltype(host)> access(reader,host,input);
            __try{cfgemotemirror::Slot source{};cfgemotemirror::Function play{};cfgemotemirror::Catalogue assets{};std::uintptr_t selected=0;
                if(access.Source(source)&&source==bundleRBEmote.policy.Owner().pawn&&access.Bind(0,play,false)&&
                   play.object==reinterpret_cast<std::uintptr_t>(function)&&access.Parameter(reinterpret_cast<std::uintptr_t>(parameters),selected)&&
                   access.CatalogueFor(source,selected,assets))bundleRBEmote.ObserveSelection(source.object);
            }__except(EXCEPTION_EXECUTE_HANDLER){}
            bundleReport.reads+=reader.reads;bundleReport.maxReads=(std::max)(bundleReport.maxReads,reader.reads);
        }return;
    }
    if(!bundleSchedule.active||!BundleGameplayStarted()||GetCurrentThreadId()!=plan.threadId||
       !bundleEmotes.Candidate(reinterpret_cast<std::uintptr_t>(object),reinterpret_cast<std::uintptr_t>(function),nested))return;
    const bool ownsPublication=(bundleReport.generation&1u)==0;
    if(ownsPublication)InterlockedIncrement(reinterpret_cast<volatile LONG*>(&bundleReport.generation));
    Reader reader;reader.cap=cfgbundle::ReadCap;
#if CFG_H245_EMOTE && defined(CFG_H1_JASON)
    reader.cap=cfghostedbudget::EmoteReadCap(HostedRoutineActive(),reader.cap); // one shared parameterised proof budget, H2.45
#endif
    cfgemoteobservation::Access<Reader> host(reader,plan);
    auto input=BundleAccess{}.MirrorInputs();
    cfgemotemirror::NativeAccess<Reader,decltype(host)> access(reader,host,input);
    __try{bundleEmotes.Before(access,reinterpret_cast<std::uintptr_t>(object),reinterpret_cast<std::uintptr_t>(function),
        reinterpret_cast<std::uintptr_t>(parameters),nested);
        if(bundleEmotes.ObservedPlay(reinterpret_cast<std::uintptr_t>(object),reinterpret_cast<std::uintptr_t>(function)))
            bundleRBEmote.ObserveSelection(reinterpret_cast<std::uintptr_t>(object));}
    __except(EXCEPTION_EXECUTE_HANDLER){++bundleEmotes.report.refused;bundleEmotes.report.failure=1;}
    bundleReport.emoteHookPeakReads=(std::max)(bundleReport.emoteHookPeakReads,reader.reads);bundleReport.reads+=reader.reads;bundleReport.maxReads=(std::max)(bundleReport.maxReads,reader.reads);BundleEmoteReport();
    if(ownsPublication){MemoryBarrier();InterlockedIncrement(reinterpret_cast<volatile LONG*>(&bundleReport.generation));}
}
#endif
#if !CFG_J21_EMOTE_MIRROR
void BundleEmoteAfter(void* object,void* function,bool nested,bool completed)noexcept{
    if(bundleRBDispatch&&reinterpret_cast<std::uintptr_t>(object)==bundleHumanTarget.object&&
        bundleEmotes.IsPlayFunction(reinterpret_cast<std::uintptr_t>(function)))nested=false;
    if(!bundleSchedule.active||!BundleGameplayStarted()||GetCurrentThreadId()!=plan.threadId||nested||
       !bundleEmotes.Candidate(reinterpret_cast<std::uintptr_t>(object),reinterpret_cast<std::uintptr_t>(function),false))return;
    const bool ownsPublication=(bundleReport.generation&1u)==0;
    if(ownsPublication)InterlockedIncrement(reinterpret_cast<volatile LONG*>(&bundleReport.generation));
    Reader reader;reader.cap=cfgbundle::ReadCap;
#if CFG_H245_EMOTE && defined(CFG_H1_JASON)
    reader.cap=cfghostedbudget::EmoteReadCap(HostedRoutineActive(),reader.cap); // one shared parameterised proof budget, H2.45
#endif
    cfgemoteobservation::Access<Reader> host(reader,plan);
    auto input=BundleAccess{}.MirrorInputs();
    cfgemotemirror::NativeAccess<Reader,decltype(host)> access(reader,host,input);
    __try{bundleEmotes.After(access,reinterpret_cast<std::uintptr_t>(object),reinterpret_cast<std::uintptr_t>(function),nested,completed,GetTickCount64());}
    __except(EXCEPTION_EXECUTE_HANDLER){++bundleEmotes.report.refused;bundleEmotes.report.failure=1;}
    bundleReport.emoteHookPeakReads=(std::max)(bundleReport.emoteHookPeakReads,reader.reads);bundleReport.reads+=reader.reads;bundleReport.maxReads=(std::max)(bundleReport.maxReads,reader.reads);BundleEmoteReport();
    if(ownsPublication){MemoryBarrier();InterlockedIncrement(reinterpret_cast<volatile LONG*>(&bundleReport.generation));}
}
#else
void BundleEmoteAfter(void* object,void* function,bool nested,bool completed)noexcept{
    if(!cfgj21::EmotesEnabled)return;
    if(bundleRBDispatch&&reinterpret_cast<std::uintptr_t>(object)==bundleHumanTarget.object&&
        bundleEmotes.IsPlayFunction(reinterpret_cast<std::uintptr_t>(function)))nested=false;
    if(!bundleSchedule.active||!BundleGameplayStarted()||GetCurrentThreadId()!=plan.threadId||nested||
       !bundleEmotes.Candidate(reinterpret_cast<std::uintptr_t>(object),reinterpret_cast<std::uintptr_t>(function),false))return;
    const bool ownsPublication=(bundleReport.generation&1u)==0;
    if(ownsPublication)InterlockedIncrement(reinterpret_cast<volatile LONG*>(&bundleReport.generation));
    Reader reader;reader.cap=cfgbundle::ReadCap;
#if CFG_H245_EMOTE && defined(CFG_H1_JASON)
    reader.cap=cfghostedbudget::EmoteReadCap(HostedRoutineActive(),reader.cap); // one shared parameterised proof budget, H2.45
#endif
    cfgemoteobservation::Access<Reader> host(reader,plan);
    auto input=BundleAccess{}.MirrorInputs();
    cfgemotemirror::NativeAccess<Reader,decltype(host)> access(reader,host,input);
    __try{bundleEmotes.After(access,reinterpret_cast<std::uintptr_t>(object),reinterpret_cast<std::uintptr_t>(function),nested,completed,GetTickCount64());}
    __except(EXCEPTION_EXECUTE_HANDLER){++bundleEmotes.report.refused;bundleEmotes.report.failure=1;}
    bundleReport.emoteHookPeakReads=(std::max)(bundleReport.emoteHookPeakReads,reader.reads);bundleReport.reads+=reader.reads;bundleReport.maxReads=(std::max)(bundleReport.maxReads,reader.reads);BundleEmoteReport();
    if(ownsPublication){MemoryBarrier();InterlockedIncrement(reinterpret_cast<volatile LONG*>(&bundleReport.generation));}
}
#endif
void BundleAbandonRelease(){bundleSchedule.Released();bundleReport.releaseAbandoned=1;}
bool BundleCounselorWindowInput(std::uint64_t now,bool foreground,bool& pressed)noexcept{
    (void)now;(void)foreground;pressed=false;return false; // donor async-key path
}
void BundleExtraHelperInput(std::uint64_t now){
    const bool hosted=HostedRoutineActive();
    if(!cfgbundle::AdditionalJasonLaneEnabled(bundleSchedule.active,hosted)||now<bundleInputAt)return;bundleInputAt=now+100;
    DWORD foregroundPid=0;const auto foreground=GetForegroundWindow();
    if(foreground)GetWindowThreadProcessId(foreground,&foregroundPid);
    const bool foregroundGame=foregroundPid==GetCurrentProcessId();
    if(bundleSpectatorInput.Take(now,foregroundGame&&cfgbundle::OfflineSpectatorCameraAllowed(hosted,bundleCompletion.terminalMask)&&!bundleKill.policy.Exclusive(now)&&!bundleDeathEnd.confirmedAt))
        {bundleSpectatorInputPending=true;bundleSpectatorInputPendingAt=now;}
    if(!foregroundGame||hosted)bundleSpectatorInputPending=false;
    const auto f3=GetAsyncKeyState(VK_F3);
    ++bundleF3Polls;if(foregroundGame)++bundleF3Foreground;
    if(foregroundGame&&(f3&0x8000))++bundleF3DownSamples;
    if(foregroundGame&&(f3&1))++bundleF3ShortSamples;
    const bool f1=foregroundGame&&(GetAsyncKeyState(VK_F1)&0x8000)!=0;
    const bool f1Edge=f1&&!bundleAdditional.policy.down;
    bool pressed=false;
    if(!hosted&&!BundleCounselorWindowInput(now,foregroundGame,pressed))pressed=cfginput::ForegroundKeyEdge(foregroundGame,f3,bundleF3Down);
    if(!f1Edge)bundleAdditional.policy.down=f1;
    if(!pressed&&!bundleF3Queue.pending&&!f1Edge)return;
    InterlockedIncrement(reinterpret_cast<volatile LONG*>(&bundleReport.generation));
    BundleAccess access;bool alive=false;
    // Explicit foreground keypress only. No automatic growth, no native work
    // in an idle poll, no recovery of uncertain/exhausted spawn transactions.
    const bool scope=access.HelperContext();(void)alive;
    if(f1Edge){
#if defined(CFG_H1_JASON)&&CFG_H265_F1_RETRY
        // A clean refusal unwound everything it made (allocationAttempted stays false), so a new press starts a fresh attempt.
        if(HostedRoutineActive()&&bundleAdditional.policy.stage==cfgadditional::Stage::Refused&&!bundleAdditional.allocationAttempted&&!bundleAdditional.journal.uncertain){
            const auto keep=bundleAdditional.policy;bundleAdditional=decltype(bundleAdditional){};
            bundleAdditional.policy=keep;bundleAdditional.policy.stage=cfgadditional::Stage::Idle;
            cfgclosedtrace::Mark("f1-h265-retry-armed",bundleJason.pawn.object,keep.failures);
        }
#endif
#if CFG_H286_F1_NEW_MATCH
        // Live H2.85: the previous match's F1 stayed Active into the next world, so every new press was refused (stage 10).
        // Its actors died with that world; a different world means nothing of it can still be ours. Counters are kept.
        if(bundleAdditional.world&&bundleAdditional.world!=plan.world&&bundleAdditional.policy.stage!=cfgadditional::Stage::Idle){
            auto keep=bundleAdditional.policy;const auto from=static_cast<unsigned>(keep.stage);bundleAdditional=decltype(bundleAdditional){};
            keep.stage=cfgadditional::Stage::Idle;bundleAdditional.policy=keep;++bundleReport.h286F1NewMatchResets;
            cfgclosedtrace::Mark("h286-f1-new-match-reset",plan.world,from);
        }
#endif
        const bool admitted=bundleAdditional.policy.Request(f1,foregroundGame,scope&&cfgadditional::LifecycleProved,now);
        if(admitted)bundleAdditional.world=plan.world; // H2.86: the world this F1 belongs to
#ifdef CFG_H1_JASON
        if(hosted)HostedF1CaptureRequest(admitted);
        if(admitted&&HostedRoutineActive())hostedJasonExtraCountConsumed=true;
#endif
        if(scope&&!cfgadditional::LifecycleProved)cfgclosedtrace::Mark("f1-blocked-unproved-extra-lifecycle",bundleJason.pawn.object,now);
        cfgclosedtrace::Mark(admitted?"f1-jason-queued":"f1-jason-request-refused",bundleJason.pawn.object,
            static_cast<unsigned>(bundleAdditional.policy.stage));
    }
    if(pressed){
        if(scope&&bundleF3Queue.Request(bundleSchedule.requestedBots)){
            ++bundleReport.additionalHelperRequests;
            cfgclosedtrace::Mark("f3-counselor-queued",plan.pawn,bundleF3Queue.pending);
        }else ++bundleReport.additionalHelperRefusals;
    }
    if(!scope||bundleHelpers.stage==cfgbundle::HelperStage::Unavailable||
       bundleHelpers.stage==cfgbundle::HelperStage::Indeterminate)bundleF3Queue.Cancel();
    if(scope&&bundleF3Queue.Ready(bundleSchedule.spawned,bundleSchedule.requestedBots)&&
       bundleHelpers.Reinforcement(bundleReport.spawned,bundleSchedule.requestedBots,cfgbundle::MaximumBots)&&
       bundleSchedule.AdditionalHelper(now)){
        bundleF3Queue.Started();bundleHelperReinforcement=true;
        bundleReport.requestedBots=bundleSchedule.requestedBots;
        bundleReport.helperStage=static_cast<unsigned>(bundleHelpers.stage);bundleReport.helperFailure=0;
    }
    bundleReport.reads+=access.Reads();bundleReport.maxReads=(std::max)(bundleReport.maxReads,access.Reads());
    bundleReport.error=access.Error();MemoryBarrier();InterlockedIncrement(reinterpret_cast<volatile LONG*>(&bundleReport.generation));
}
// H2.47: lease-mode legacy Morph/burst veto from the donor combat-busy predicate. site: 1 queue, 2 service, 3 burst.
// Offline (no lease) never vetoes here: J2.19 behaviour is unchanged.
bool H247MorphBusyVeto(BundleAccess& a,bool leasePublished,std::uint64_t now,unsigned site){
    if(!CFG_H247_MORPH_BUSY||!leasePublished)return false;
    std::uint32_t causes=0;
    if(!cfgh247::MorphBusyVeto(leasePublished,a.H247LegacyCombatBusy(now,&causes)))return false;
#ifdef CFG_H1_JASON
    auto& t=hostedDriverTrace;t.h247LastBusyCauses=causes;
    if(site==1)++t.h247MorphQueueVetoes;else if(site==2)++t.h247MorphServiceVetoes;else ++t.h247BurstVetoes;
#else
    (void)site;
#endif
    return true;
}
void BundlePhase(BundleAccess& a,cfgbundle::Lane lane,std::uint64_t now,bool continuePublication=false,bool continueAxes=false){
#ifdef CFG_H1_JASON
    const bool nativeDeathOwnsMotion=HostedAdoptionOwnsDriver()&&hostedNativeDeathMotion&&lane!=cfgbundle::Lane::Release;
#else
    constexpr bool nativeDeathOwnsMotion=false;
#endif
    // D2's single native-Tick path is the sole Jason motion owner during an
    // ordinary hold. Paired seat cleanup and cinematic/owed release stay live.
    // H2.46: a lease-mode burst must always be restored, even while the donor hold / native death owns motion.
    if(CFG_H246_BURST&&bundleFootBurst.Owned()&&(nativeDeathOwnsMotion||DonorPrimaryOrdinaryHeldHint()))a.FootBurstService(now,true);
    if(!nativeDeathOwnsMotion&&lane!=cfgbundle::Lane::Release&&!bundleCompletionReady&&!bundleKill.policy.Exclusive(now)&&DonorPrimaryOrdinaryHeldHint()){
        // Already-detached seat retirement is value bookkeeping, not a motion
        // action. Do not strand the accepted car episode behind D's hold gate.
        if(bundleHeldRecovery.car&&now>=bundleHeldRecovery.next)a.HeldRecoveryPhase(now);
        return;
    }
    if(bundleDeathEnd.confirmedAt||bundleCompletionReady)bundleEscapeGuard.ClearOwnership();
    struct VehicleContinuationHost {std::uint64_t now;
        cfgbundle::Schedule& VehicleSchedule(){return bundleSchedule;}
        bool UrgentVehicleContinuation(){return !bundleKill.policy.Exclusive(now)&&!bundleDeathEnd.confirmedAt&&
            !bundleCompletionReady&&((bundleVehicleActions.car&&bundleVehicleActions.driver&&bundleVehicleActions.extractionCommitted)||
                bundleCarActionPending.active||bundleRoadMorph.stage!=0);}
    } vehicleContinuationHost{now};
    cfgvehiclecontinuation::Turn<VehicleContinuationHost> vehicleContinuation(vehicleContinuationHost,lane,now);
    if(!nativeDeathOwnsMotion){
    if(lane!=cfgbundle::Lane::Release&&bundleFootBurst.Due(now,
       bundleFootObstacle.stage||bundleCarPursuit.Active(now)||bundleReport.target!=bundleFootBurst.target||
       bundleKill.policy.Exclusive(now)||bundleDeathEnd.confirmedAt||bundleCompletionReady||bundleKnife.Busy()||
       bundleHiding.Active()||bundleTraps.Busy()||bundleDoorRecovery.breaking||bundleDoorRecovery.phase||bundleSchedule.meleeUntil>now)){
        a.FootBurstService(now,bundleFootObstacle.stage||bundleCarPursuit.Active(now)||
            bundleReport.target!=bundleFootBurst.target||bundleKill.policy.Exclusive(now)||bundleDeathEnd.confirmedAt||
            bundleCompletionReady||bundleKnife.Busy()||bundleHiding.Active()||bundleTraps.Busy()||
            bundleDoorRecovery.breaking||bundleDoorRecovery.phase||bundleSchedule.meleeUntil>now);
        return; // independent one-field proof/CAS; no5x/25x handoff into car lease
    }
    if(lane==cfgbundle::Lane::Vehicle&&bundleFootBurst.Owned()){
        a.FootBurstVehicleTurn(now);return; // bounded deferral, never overlapping car/foot speed leases
    }
    if(lane==cfgbundle::Lane::Hunt&&bundleEscapeGuard.morphPending){a.EscapeGuardMorphService(now);return;}
    if(lane==cfgbundle::Lane::Hunt&&bundleCounselorMorphTarget){
        const auto target=bundleCounselorMorphTarget;bundleCounselorMorphTarget=0;
#ifdef CFG_H1_JASON
        const bool hostedPublished=HostedAdoptionOwnsDriver();
        const bool donorActive=NativePrimaryHuntingActive();
        ++hostedDriverTrace.morphLeaseChecks;if(hostedPublished)++hostedDriverTrace.morphLeaseHosted;
        if(donorActive)++hostedDriverTrace.morphLeaseActive;
#endif
        if(!cfgh246::LeaseAdmits(DonorNativeMovementLeasePublished())||(DonorNativeMovementLeasePublished()&&(DonorNavPendingForLegacy()||DonorTrapIntentForLegacy()))){
#ifdef CFG_H1_JASON
            if(cfgh246::LeaseAdmits(true)&&DonorNavPendingForLegacy())++hostedDriverTrace.h246MorphLeaseNavBlocked;
#endif
            bundlePursuitProgress.Clear();bundlePursuit.Clear();return;
        }
        if(!bundleKill.policy.Exclusive(now)&&!bundleDeathEnd.confirmedAt&&!bundleCompletionReady&&
           !bundleCarPursuit.Active(now)&&!bundleVehicleActions.StopOwnsEpisode()&&
           !bundleKnife.Busy()&&!bundleHiding.Active()&&!bundleTraps.Busy()&&!bundleDoorRecovery.breaking&&!bundleDoorRecovery.phase&&
           !H247MorphBusyVeto(a,DonorNativeMovementLeasePublished(),now,2))
        {
            const auto morphClockBefore=bundleCarMorph.next;
            a.CounselorMorphService(now,target);
            // H2.46: a native Morph consumed the SAME 20 s shared clock (imported into the donor chase every
            // donor service); give the donor its own post-Morph chase reset, as DonorResetChaseAfterMorph does.
            if(CFG_H246_BURST&&bundleCarMorph.next!=morphClockBefore&&DonorNativeMovementLeasePublished()){
                DonorChaseResetAfterLegacyMorph();
#ifdef CFG_H1_JASON
                ++hostedDriverTrace.h246MorphLeaseTeleported;
#endif
            }
        }
        else {bundlePursuitProgress.Clear();bundlePursuit.Clear();}
        return;
    }
    if(lane==cfgbundle::Lane::Hunt&&bundleFootObstacleTarget){
        if(bundleRun.Step(a,now,bundleFootBurst.Owned()||bundleCarSpeed.active,1.f))return;
        if(!bundleKill.policy.Exclusive(now)&&!bundleCarPursuit.Active(now)&&!bundleVehicleActions.StopOwnsEpisode()&&
           !bundleKnife.Busy()&&!bundleHiding.Active()&&!bundleDoorRecovery.breaking&&!bundleDoorRecovery.phase)
            a.FootCarObstacle(now,bundleFootObstacleTarget);
        else {bundleFootObstacle.Reset();bundleFootObstacleTarget=0;}
        return;
    }
    if(lane!=cfgbundle::Lane::Release&&bundleFootBurstRequestAt){
        const auto requested=bundleFootBurstRequestAt;bundleFootBurstRequestAt=0;
#ifdef CFG_H1_JASON
        const bool hostedPublished=HostedAdoptionOwnsDriver();
        const bool donorActive=NativePrimaryHuntingActive();
        ++hostedDriverTrace.burstLeaseChecks;if(hostedPublished)++hostedDriverTrace.burstLeaseHosted;
        if(donorActive)++hostedDriverTrace.burstLeaseActive;
#endif
        const bool burstUnderLease=DonorNativeMovementLeasePublished();
        if(!cfgh246::LeaseAdmits(burstUnderLease))return;
        if(now>=requested&&now-requested<=1000&&lane!=cfgbundle::Lane::Vehicle&&
           !bundleCarPursuit.Active(now)&&!bundleCarSpeed.active&&!bundleKill.policy.Exclusive(now)&&!bundleDeathEnd.confirmedAt&&
           !bundleCompletionReady&&!bundleKnife.Busy()&&
           (!bundleKnifeProbePending||(CFG_H285_CHASE_BURST&&bundleFootBurstSquared>=0.f&&bundleFootBurstSquared<=cfgfootburst::ChaseRangeCm*cfgfootburst::ChaseRangeCm))&&
           !bundleHiding.Active()&&!bundleTraps.Busy()&&
           !bundleDoorRecovery.breaking&&!bundleDoorRecovery.phase&&!H247MorphBusyVeto(a,burstUnderLease,now,3)){a.FootBurstBegin(now,burstUnderLease);
#ifdef CFG_H1_JASON
            hostedDriverTrace.h246BurstLeaseStarts=bundleFootBurst.leaseStarts;
#endif
            return;}
        ++bundleReport.h285BeginBlocked;
    }
    bundleKill.policy.Expire(now);
    if(lane!=cfgbundle::Lane::Release&&!bundleKill.policy.Exclusive(now)&&!bundleDeathEnd.confirmedAt&&
       !NativePrimaryHuntingActive()&&!bundleGrab.held&&!bundleKnife.Busy()&&!bundleHiding.Active()&&a.ChaseRecoveryPhase(now))return;
    if(lane!=cfgbundle::Lane::Release&&!bundleKill.policy.Exclusive(now)&&!bundleDeathEnd.confirmedAt&&
       !bundleKnife.Busy()&&!bundleHiding.Active()&&
       (bundleGrab.held||bundleVehicleActions.extractionCommitted||bundleHeldRecovery.attemptAt)&&
       now>=bundleHeldRecovery.next&&a.HeldRecoveryPhase(now))return;
    if(lane!=cfgbundle::Lane::Release&&!bundleKill.policy.Exclusive(now)&&!bundleDeathEnd.confirmedAt&&
       bundleVehicleProgress.pending&&a.VehicleProgressPhase(now))return;
    if(lane!=cfgbundle::Lane::Release&&bundleKill.policy.RecoveryPending(now)){
        if(now>=bundleKillAt){bundleKillAt=now+250;a.KillRecovery(now);}
        return; // native cleanup has its own fresh budget before helper/hunt work
    }
    const auto sweaterContinuation=cfgsweater::ChooseContinuation(bundleSweaterPolicy,now,
        lane==cfgbundle::Lane::Convergence,bundleSweaterFreshRequested,bundleSweaterClassPending);
    // Publication already changed owned gameplay state. Finish its finite HUD
    // handoff in a fresh callback before optional class capture/helper work.
    // Release and native kill/death transactions retain exclusive ownership.
    if(lane!=cfgbundle::Lane::Release&&!(bundleCompletion.terminalMask&1u)&&
       !bundleKill.policy.Exclusive(now)&&!bundleDeathEnd.confirmedAt&&
       sweaterContinuation==cfgsweater::Continuation::Hud){
        if(a.Context())a.SweaterPhase(now);else ++bundleReport.sweaterUnavailable;
        if(bundleSweaterPolicy.stage==cfgsweater::Stage::Refused)
            cfgbundle::RecordSweaterFirstRefusal(bundleReport,101,bundleSweaterPolicy.lastFailure,
                static_cast<unsigned>(bundleSweaterPolicy.stage),bundleSweaterOwner.object,now,bundleReport.sweaterFreshFailure);
        return;
    }
    // Completion is an owed terminal transaction, not an optional empty-target
    // sweep behind helper publication, retained car paths or road preparation.
    // All tracked terminal bits PLUS fresh native roster/Tommy checks remain.
    if(lane!=cfgbundle::Lane::Release&&!bundleKill.policy.Exclusive(now)&&!bundleDeathEnd.confirmedAt){
        if(!bundleCompletionReady&&bundleNativeTargetHint&&
           (lane==cfgbundle::Lane::Select||lane==cfgbundle::Lane::Hunt)){
            // Native terminal veto found a counselor absent from the fixed
            // helper registry. Adopt only through fresh owned native proof;
            // this is read-only acquisition, not a native cleanup or ending.
            if(a.Context()&&a.Jason())a.NativePursuitAcquire();
            return;
        }
        if(bundleCompletionReady){
            if(bundleKnife.Busy()){{J21Timing timing(cfgj21::Knife);a.KnifeTick(now);}LatchBundleKnifeFirstAbort();return;} // drain owed native throw cleanup first
            if(bundleCarSpeed.active){a.CarBoost(false);return;}
            if(a.TravelCleanup(now))return;
            if(!bundleEnding.Finished()){a.EndingMetadataPhase();return;}
            a.PracticeCleanup();bundleCompletionReady=false;
            if(a.Context())a.CompleteMatch();else bundleReport.completionFailure=1;
            if(bundleCompletion.dispatched)BundleStop(cfgbundle::Stop::NoTargets);
            else bundleCompletion.candidateSince=0;
            return; // independent budget; never inherit run/door/health proofs
        }
        const bool helpersFinished=cfgbundle::HelpersSettled(bundleReport.spawned,bundleSchedule.requestedBots,bundleHelpers.stage);
        if(bundleCompletion.CheckDue(bundleReport.bots,helpersFinished,now)){
            ++bundleReport.completionProbes;
            if(a.Context()&&a.Jason()){
                if(bundleCompletion.Ready(bundleReport.bots,helpersFinished,a.NativeTerminal(),now))bundleCompletionReady=true;
            }
            return;
        }
    }
    if(lane!=cfgbundle::Lane::Release&&!bundleKill.policy.Exclusive(now)&&!bundleDeathEnd.confirmedAt&&
       !bundleSoundBlips.Attempted()&&a.SoundBlipPhase())return; // clear initial blips before slow startup metadata
    if(bundleHiding.Active()&&!bundleKill.policy.Exclusive(now)&&!bundleDeathEnd.confirmedAt&&
       (lane==cfgbundle::Lane::Hunt||lane==cfgbundle::Lane::Select||lane==cfgbundle::Lane::Vehicle)){
        // A started hiding interaction owns these motion lanes, not the latest
        // selector/car/grab gate. HUD, helpers, owed release and death still run.
        // Dedicated fresh scope: do not stack native cleanup on maintenance.
        if(a.Context()&&a.Jason())a.HidingPhase(bundleHiding.counselor,now);
        else {++bundleHidingRefused;HostedLaneRefuse(cfgh1::Lane::Hiding,1);}
        return;
    }
    // Finite startup metadata gets a fresh budget, rather than spending most
    // of it on unrelated run/health/completion proofs before its own Scope.
    // Owed input and kill/death ownership still take precedence.
    if(lane!=cfgbundle::Lane::Release&&!bundleKill.policy.Exclusive(now)&&!bundleDeathEnd.confirmedAt){
        if(lane==cfgbundle::Lane::Settings){
            // Optional arrival timer gets the same independent budget as the
            // native-Jason two-part path, before hunt/held/run/trap maintenance.
            if(!a.SettingsPhase(now)){bundleReport.resourceFailure=10;BundleStop(cfgbundle::Stop::Resources);}
            return;
        }
        if(!bundleRageDifficulty.attempted&&a.RageDifficultyPhase(bundleRageDifficulty,bundleJason.pawn))return;
        if(lane==cfgbundle::Lane::Spawn){
            // The queued donor birth/start survey has its OWN fresh context,
            // tree and resource checks. Do not stack hunt/health/held/run/trap
            // maintenance before them within the same1536-read/12-call slice.
            // No read/call limit or birth authorization is relaxed here.
            a.SpawnCounselor();bundleSchedule.helpersActive=bundleHelpers.Active();
            bundleReport.helperStage=static_cast<unsigned>(bundleHelpers.stage);
            bundleReport.helperFailure=static_cast<unsigned>(bundleHelpers.failure);
            bundleReport.helperCandidate=bundleHelpers.candidate;bundleReport.helperReads=a.Reads();
            bundleReport.helperStartState=static_cast<unsigned>(bundleCounselorStarts.CurrentState());
            bundleReport.helperStartCount=bundleCounselorStarts.Count();bundleReport.helperCatalogueCursor=bundleRandomCounselors.cursor;
            bundleReport.helperSelectedEntry=bundleHelperEntry;bundleReport.helperSelectedClass=bundleHelperTemplate.pawnClass;return;
        }
        // Underwater admission/action needs eleven warm native calls. Ordinary
        // hunt busy/stun/distance checks used to spend six BEFORE SwimPhase,
        // making a positive action unreachable inside the unchanged12-call cap.
        // A water pointer is only a scheduling hint; full current target/world,
        // living/held/manager and native eligibility proofs still own dispatch.
        if(lane==cfgbundle::Lane::Hunt&&a.SwimDue(bundleReport.target,now)){
            HostedLaneEnter(cfgh1::Lane::Swim);
            if(!bundleHiding.Active()&&!bundleKnife.Busy()&&!bundleKnifeProbePending&&!bundleTraps.Busy()&&
               !bundleCarPursuit.Active(now)&&!bundleDoorRecovery.breaking&&!bundleDoorRecovery.phase){
                std::uintptr_t target=0,held=0;bool alive=false;
                if(a.Context()&&a.Jason()&&a.Known(bundleTargetIndex,target)&&target==bundleReport.target&&
                   a.Alive(bundleJason.pawn.object,alive)&&alive&&a.Held(held)&&!held&&a.NativeBindings()&&a.Manager())
                    a.SwimPhase(target,now);
                else HostedLaneRefuse(cfgh1::Lane::Swim,1); // hosted Context/Jason/target/held/binding gate
                return; // no stacked hunt/door/input work on a refused swim phase
            }else HostedLaneRefuse(cfgh1::Lane::Swim,2); // established exclusive motion owner; normal hunt continues
        }
        // Dedicated road/detour callbacks must not inherit CarNativeDrain's
        // occupancy/manager debt. Each phase independently guards its native
        // lock, current driver and offline world immediately before effects.
        if(lane==cfgbundle::Lane::Vehicle&&bundleCarObstacle.stage){a.CarObstaclePhase(now);return;}
        if(lane==cfgbundle::Lane::Vehicle&&bundleRoadMorph.stage){a.CarRoadPhase(now);return;}
        if(lane==cfgbundle::Lane::Vehicle&&bundleVehicleActions.car){bool extracting=false;
            if(!a.CarNativeDrain(now,extracting)){++bundleVehicleActions.refusals;return;}
            if(extracting)return;}
        if(!bundleEnding.Finished()){a.EndingMetadataPhase();return;}
        if(bundleTraps.report.stage==cfgtrap::startup::Stage::Discover&&bundleTraps.Due(now)){{J21Timing timing(cfgj21::Traps);a.TrapPhase(now);}return;}
        if(bundleKnife.Busy()){{J21Timing timing(cfgj21::Knife);a.KnifeTick(now);}LatchBundleKnifeFirstAbort();return;}
        if(bundleKnifeProbePending){
            if(bundleKnifeRefresh.active){const auto refreshed=bundleKnifeRefresh.Step(a,now,bundleKnifeProbeIndex);
                if(refreshed==cfgknife::RefreshResult::Unavailable)bundleKnifeProbePending=false;
                return;}
            bundleKnifeProbePending=false;bundleKnife.Begin(a,bundleKnifeProbeIndex,now);LatchBundleKnifeFirstAbort();return;}
        if(sweaterContinuation==cfgsweater::Continuation::Fresh){
            a.FreshSweaterContinuation(now);
            if(bundleSweaterPolicy.stage==cfgsweater::Stage::Refused)
                cfgbundle::RecordSweaterFirstRefusal(bundleReport,102,bundleSweaterPolicy.lastFailure,
                    static_cast<unsigned>(bundleSweaterPolicy.stage),bundleSweaterOwner.object,now,bundleReport.sweaterFreshFailure);
            return; // refresh incarnation before consuming its one-shot rearm
        }
        if(sweaterContinuation==cfgsweater::Continuation::Class){
            bundleSweaterClassPending=false;a.CaptureEarnedSweaterClass();return;
        }
        const bool sweaterOwnerChanged=!(bundleSweaterOwner==bundleHumanTarget);
        const bool sweaterOwnerPickupRetry=bundleSweaterPolicy.stage==cfgsweater::Stage::Refused&&
            !bundleSweaterPolicy.lastUse&&!bundleSweaterPolicy.publicationStarted&&!bundleSweaterFreshGrantAttempted&&
            (!bundleSweaterAbility.object||(bundleSweaterPolicy.lastFailure>=112&&bundleSweaterPolicy.lastFailure<=114));
        const bool sweaterPollDue=cfgsweaterowner::PollDue(
            bundleSweaterPolicy.CanPoll()||sweaterOwnerPickupRetry,
            bundleSweaterPolicy.stage==cfgsweater::Stage::Refused&&!bundleLateMaskRetryUsed&&sweaterOwnerPickupRetry,
            sweaterOwnerChanged,bundleKill.policy.Exclusive(now),bundleSweaterPolicy.publicationStarted,
            bundleSweaterPolicy.next,now);
        const auto convergenceWork=lane==cfgbundle::Lane::Convergence&&!continuePublication&&!continueAxes?
            bundleFeatureTurns.Choose(sweaterPollDue,bundleConvergence.active):cfgconvergence::FeatureTurns::Work::Other;
        if(convergenceWork==cfgconvergence::FeatureTurns::Work::Sweater){
            if(a.RetryLateMask(now))return;
            if(a.Context())a.SweaterPhase(now);
            else ++bundleReport.sweaterUnavailable;
            if(bundleSweaterPolicy.stage==cfgsweater::Stage::Refused)
                cfgbundle::RecordSweaterFirstRefusal(bundleReport,103,bundleSweaterPolicy.lastFailure,
                    static_cast<unsigned>(bundleSweaterPolicy.stage),bundleSweaterOwner.object,now,bundleReport.sweaterFreshFailure);
            return; // observe/grant/rearm MUST precede active helper early-return
        }
        // Donor helper travel is independent of killer hunt/run/door upkeep.
        // Do not spend those proofs before the helper's owned brain/path call.
        if(convergenceWork==cfgconvergence::FeatureTurns::Work::Helpers){
            if(bundleCarPursuit.Active(now)||bundleVehicleActions.StopOwnsEpisode())return;
            if(a.Context()&&a.Jason()){
                if(a.ConvergenceFearPhase()||a.ConvergenceTravel(now))return;
                if(a.BotNamePhase())return;
                if((!cfgj21::EmotesEnabled||!DonorHeldDecisionInstalled())&&bundleEmotes.report.prepared<2&&!bundleEmotes.report.refused){a.MirrorPrepare();return;}
                if(bundleFeatureTurns.Damage()&&a.WarmDamageGate())return;
                if(!a.Convergence(now))++bundleReport.convergenceUnavailable;
            }
            return; // no second Context/maintenance traversal when travel yields to publication
        }
        if(lane==cfgbundle::Lane::Vehicle&&!bundleDoorRecovery.breaking&&!bundleDoorRecovery.phase&&
           !bundleTrapPriority.Active(now)&&(!bundleTraps.Busy()||bundleTraps.Paused())&&!bundleHelpers.Active()){
            if(bundleCarObstacle.stage){a.CarObstaclePhase(now);return;}
            if(bundleRoadMorph.stage){a.CarRoadPhase(now);return;}
            if(bundleRoads.stage!=cfgroad::Stage::Ready&&bundleRoads.stage!=cfgroad::Stage::Refused){a.PrepareRoads();return;}
        }
    }
    } // Native death skips new optional motion, never terminal/kill cleanup below.
    bundleReport.resourceFailure=0;
    // AwaitContext/NativeContext prohibit Jason hunt/Morph, NOT the rescue
    // team's native swings. Donor retires helper convergence only once the
    // final stock interaction commits; clearing brains at the kneel was early.
    if(!nativeDeathOwnsMotion&&lane==cfgbundle::Lane::Convergence&&bundleKill.policy.Exclusive(now)&&
       cfgconvergence::CombatAllowed(bundleKill.policy.interactionAttempted,bundleCompletionReady,bundleDeathEnd.confirmedAt!=0)){
        if(a.Context()&&a.Jason()){
            if(continueAxes){a.ConvergenceAxePhase(now);return;}
            if(!continuePublication&&(a.ConvergenceFearPhase()||a.ConvergenceTravel(now)))return;
            if(!a.Convergence(now))++bundleReport.convergenceUnavailable;
        }
        return;
    }
    if(lane!=cfgbundle::Lane::Release&&
       (nativeDeathOwnsMotion||(bundleKill.policy.Exclusive(now)&&bundleKill.policy.interactionAttempted)||bundleCompletionReady)&&a.TravelCleanup(now))return;
    if(lane==cfgbundle::Lane::Release&&bundleKill.policy.interactionAttempted){
        bool dying=false;
        if(a.DeathScope(dying)&&dying){
            bundleDeathEnd.Confirm(bundleKill.policy,true,now);
            BundleAbandonRelease();return; // native death may already detach AI; never input into a stale controller
        }
    }
    if(lane!=cfgbundle::Lane::Release&&bundleCarSpeed.active&&
       (nativeDeathOwnsMotion||!bundleCarPursuit.Active(now)||bundleKill.policy.interactionAttempted||bundleCompletionReady)){
        if(a.CarBoost(false))return; // restore in its own bounded slice before ending/hunting
        ++bundleVehicleActions.refusals;
    }
    if(lane!=cfgbundle::Lane::Release){const auto death=a.DeathCompletion(now);
        if(death){if(death>=2)BundleStop(death==2?cfgbundle::Stop::NoTargets:cfgbundle::Stop::Context);return;}}
    // The native death context owns this Jason while a proved final hit runs.
    // Never hunt/Morph/attack over its stance or kill montage. Owed releases above
    // remain first, and one bounded native kill phase runs at most every250ms.
    bundleKill.policy.Expire(now);
    if(lane!=cfgbundle::Lane::Release&&bundleKill.policy.Exclusive(now)){
        if(now>=bundleKillAt){bundleKillAt=now+250;bundleKill.Tick(a,now);}
        bundleReport.killStage=static_cast<unsigned>(bundleKill.policy.stage);
        bundleReport.killStances=bundleKill.stanceDispatches;bundleReport.killInteractions=bundleKill.interactionDispatches;
        bundleReport.killUnavailable=bundleKill.unavailable;
        bundleReport.killFailure=static_cast<unsigned>(bundleKill.failure);
        bundleReport.killInteractionDeferrals=bundleKill.interactionDeferrals;return;
    }
    // Runtime owns its own fresh Scope/DeathContext validation. Do not spend a
    // duplicate outer Context/primary proof before its native predicate reserve.
    if(nativeDeathOwnsMotion){
        if(bundleCompletionReady){
            if(!bundleEnding.Finished()){a.EndingMetadataPhase();return;}
            a.PracticeCleanup();bundleCompletionReady=false;
            if(a.Context())a.CompleteMatch();else bundleReport.completionFailure=1;
            if(bundleCompletion.dispatched)BundleStop(cfgbundle::Stop::NoTargets);
            else bundleCompletion.candidateSince=0;
        }
        return; // The native kill keeps ownership after all owed lifecycle work.
    }
    if(!a.Context()){
        // Release keeps its immediate stop: never leave a held input pending.
        if(lane!=cfgbundle::Lane::Release&&a.ContextTransient())return;
        bundleReport.resourceFailure=1;BundleStop(cfgbundle::Stop::Context);if(lane==cfgbundle::Lane::Release)BundleAbandonRelease();return;}
    if(!a.Jason()){bundleReport.resourceFailure=2;BundleStop(cfgbundle::Stop::JasonIdentity);if(lane==cfgbundle::Lane::Release)BundleAbandonRelease();return;}
    if(lane==cfgbundle::Lane::Release){
        // No CanAttack, stun, target or health gate may silently abandon release.
        if(!a.ReleaseBinding()||!a.Input(true)){BundleStop(cfgbundle::Stop::Input);BundleAbandonRelease();return;}
        ++bundleReport.releases;bundleSchedule.Released();return;
    }
    if(a.SweaterKillArmPhase(now))return; // fresh budget; never discard a detected native sweater-use edge
    if(bundleKnife.Busy()){
        {J21Timing timing(cfgj21::Knife);a.KnifeTick(now);}
        LatchBundleKnifeFirstAbort();
        return; // The stock throw owns its animation until launch/cleanup.
    }
    if(bundleKnifeProbePending){
        if(bundleKnifeRefresh.active){const auto refreshed=bundleKnifeRefresh.Step(a,now,bundleKnifeProbeIndex);
            if(refreshed==cfgknife::RefreshResult::Unavailable)bundleKnifeProbePending=false;
            return;}
        bundleKnifeProbePending=false;
        bundleKnife.Begin(a,bundleKnifeProbeIndex,now);
        LatchBundleKnifeFirstAbort();
        return; // Fresh bounded callback, even if native entry gates refuse.
    }
    if(bundleCompletionReady){
        if(!bundleEnding.Finished()){a.EndingMetadataPhase();return;}
        bundleCompletionReady=false;a.CompleteMatch();
        if(bundleCompletion.dispatched)BundleStop(cfgbundle::Stop::NoTargets);
        else bundleCompletion.candidateSince=0;
        return; // fresh terminal confirmation/native completion has its own budget
    }
    bool living=false;if(!a.Alive(bundleJason.pawn.object,living)){bundleReport.resourceFailure=3;BundleStop(cfgbundle::Stop::Resources);return;}
    if(!living){BundleStop(cfgbundle::Stop::NoTargets);return;}
    std::uintptr_t held=0;if(!a.Held(held)){bundleReport.resourceFailure=4;BundleStop(cfgbundle::Stop::Resources);return;}
    if((lane==cfgbundle::Lane::Hunt||lane==cfgbundle::Lane::Vehicle||lane==cfgbundle::Lane::Select)&&
       (bundleDoorRecovery.breaking||bundleDoorRecovery.phase)){
        if(bundleDoorRecovery.DecisionExpired(now)){
            // A pre-dispatch refusal must not own hunt/select/vehicle forever.
            // This releases ONLY local decision state, not native locks,
            // intents or a reciprocal owned breakdown animation.
            bundleDoorRecovery.YieldDecision(now);bundleSchedule.selectAt=now;
            bundleSchedule.huntAt=now;bundleMoveAt=now;return;
        }
        if(bundleDoorRecovery.ContinuationDue(now))a.DoorRecovery(now,1000000.f);
        return; // native breakdown owns Jason motion; helpers/HUD/kill/end remain independent
    }
    if(a.PracticePhase(now))return; // bounded optional presentation; Jason Brain remains on native Tick
    a.HeldGrab(held,now);
    if(held){++bundleReport.heldSkips;return;}
    if(bundleRun.Step(a,now,bundleFootBurst.Owned()||bundleCarSpeed.active||bundleCarPursuit.Active(now),1.f))return;
    if(a.TrapPriorityPhase(now))return;
    // Complete the donor's one-time killer ownership/presentation transaction
    // before normal hunting, even when no custom duration/skill was selected.
    if(!bundleEnding.Finished()){a.EndingMetadataPhase();return;}
    if(a.SoundBlipPhase())return; // one native visibility transaction, not a tick scan
    if(lane==cfgbundle::Lane::Hunt&&!bundleGrabUnavailable&&bundleGrabWarm<6){a.GrabWarm();return;}
    if(lane==cfgbundle::Lane::Loadout){a.LoadoutPhase(now);return;}
    if(lane==cfgbundle::Lane::Convergence){
        if(continueAxes){a.ConvergenceAxePhase(now);return;}
        if(continuePublication){
            if(bundleConvergencePublication.Active()&&!a.Convergence(now))++bundleReport.convergenceUnavailable;
            return;
        }
        if(bundleCarPursuit.Active(now)||bundleVehicleActions.StopOwnsEpisode())return;
        if(a.ConvergenceFearPhase())return;
        if(a.BotNamePhase())return;
        if((!cfgj21::EmotesEnabled||!DonorHeldDecisionInstalled())&&bundleEmotes.report.prepared<2&&!bundleEmotes.report.refused){a.MirrorPrepare();return;}
        if(bundleFeatureTurns.Damage()&&a.WarmDamageGate())return; // cold metadata cannot starve sweater/convergence
        if(!a.Convergence(now))++bundleReport.convergenceUnavailable;return;
    }
    if(bundleTraps.Busy()){
        // A moving, positively owned occupied-car pursuit is the donor's
        // urgent exception to setup. Suspend, never abandon, unpaid traps;
        // consumption/animation confirmation must drain before any override.
        if(lane==cfgbundle::Lane::Vehicle&&bundleTraps.CanSuspend()){
            a.CarPursuit(now);
            if(bundleCarPursuit.Active(now)||bundleCarActionPending.active||bundleRoadMorph.stage)
                bundleTraps.PauseForVehicle(now);
            else if(bundleTraps.Paused())bundleTraps.Resume(now);
            return; // pursuit proof/action and trap commands never share a slice
        }
        if(bundleTraps.Paused()){
            if(bundleCarPursuit.Active(now)||bundleCarActionPending.active||bundleRoadMorph.stage)return;
            if(bundleTrapPriority.Active(now)){
                if(lane==cfgbundle::Lane::Vehicle){a.CarPursuit(now);return;}
            }else {
            if(!bundleTraps.Resume(now))return;
            bundleFollowPath.Clear();bundlePathCooldown.Clear();bundleDoorRecovery={};bundleDoorBreakSelection={};
            }
        }
        if(!bundleTraps.Paused()){
        if(lane==cfgbundle::Lane::Hunt||lane==cfgbundle::Lane::Vehicle||lane==cfgbundle::Lane::Select){
            const auto stage=bundleTraps.report.stage;
            if((stage>=cfgtrap::startup::Stage::Travel&&stage<=cfgtrap::startup::Stage::Attempt)||stage==cfgtrap::startup::Stage::ApproachFace||stage==cfgtrap::startup::Stage::ApproachProbe||stage==cfgtrap::startup::Stage::ApproachStop){
                bool allowed=false;
                if(!a.TrapMotionAllowed(allowed)){bundleReport.resourceFailure=5;BundleStop(cfgbundle::Stop::Resources);return;}
                if(!allowed)return;
            }
            // Consumption readback and animation hold must still drain while
            // stunned/holding; only new motion/placement is gated above.
            if(now>=bundleTrapDoorAt&&(stage==cfgtrap::startup::Stage::Travel||stage==cfgtrap::startup::Stage::ApproachFace||
               stage==cfgtrap::startup::Stage::ApproachProbe)){
                bundleTrapDoorAt=now+700;
                a.DoorRecovery(now,1000000.f);
                // A cold/unavailable door proof must not permanently starve
                // trap travel. The next slice advances the route; this finite
                // probe owns its budget and never stacks with native trap Move.
                return;
            }
            {J21Timing timing(cfgj21::Traps);a.TrapPhase(now);}
        }
        return; // no competing Jason chase, car intercept, door or attack during opening route
        }
    }
    if(lane==cfgbundle::Lane::Hunt||lane==cfgbundle::Lane::Vehicle){
        bool busy=false;if(!a.GrabBusy(busy)){++bundleReport.grabUnavailable;return;}
        if(busy){++bundleReport.grabBusySkips;return;}
    }
    if(lane==cfgbundle::Lane::Vehicle){if(!a.CarPursuit(now))++bundleReport.carUnavailable;return;}
    // Preserve a recently accepted occupied-car path instead of immediately
    // replacing it with a walking target. The lease expires without fresh
    // successful native pursuit; settings/helpers/combat releases still run.
    if((lane==cfgbundle::Lane::Hunt||lane==cfgbundle::Lane::Select)&&bundleCarPursuit.Active(now))return;
    if(lane==cfgbundle::Lane::Select){
        if(NativePrimaryHuntingActive()){
            // The native controller owns full-roster strict-nearest selection.
            // Preserve the accepted exit-guard observation sweep, without
            // TargetChoice scoring, chase fatigue or a six-second target lock.
            const unsigned count=bundleReport.bots+2;
            if(bundleSelectIndex>=count)bundleSelectIndex=0;
            const unsigned index=bundleSelectIndex>bundleReport.bots?BundleNativeTargetIndex:bundleSelectIndex;
            std::uintptr_t candidate=0;float squared=0;cfgspawn::Vec3 here{},there{};
            if(a.Known(index,candidate)&&a.Alive(candidate,living)&&living&&a.Distance(candidate,squared,here,there))
                a.EscapeGuardSample(bundleSelectIndex,candidate,there,now);
            const bool complete=++bundleSelectIndex>=count;
            if(complete){bundleSelectIndex=0;
                if(!bundleReport.target&&bundleCompletion.Ready(bundleReport.bots,!bundleHelpers.Active(),a.NativeTerminal(),now))
                    bundleCompletionReady=true;
            }
            bundleSchedule.ContinueSelection(now,complete);return;
        }
        std::uintptr_t candidate=0;float squared=0;
        const unsigned targetCount=bundleReport.bots+2; // local + F3 bots + native roster target
        if(bundleSelectIndex>=targetCount)bundleSelectIndex=0;
        if(!bundleSelectIndex)bundleChoice.Begin();
        const unsigned candidateIndex=bundleSelectIndex>bundleReport.bots?BundleNativeTargetIndex:bundleSelectIndex;
        cfgspawn::Vec3 sweepJason{},sweepAt{};
        if(a.Known(candidateIndex,candidate)&&a.HidingTargetAllowed(candidate,now)&&a.HeldRecoveryTargetAllowed(candidate,now)&&
           a.Alive(candidate,living)&&living&&a.Distance(candidate,squared,sweepJason,sweepAt))
            {const float at[2]{sweepAt.x,sweepAt.y};bundleChoice.Consider(bundleSelectIndex,squared,candidate==bundleReport.target,at);
             a.EscapeGuardSample(bundleSelectIndex,candidate,sweepAt,now);}
        if(++bundleSelectIndex>=targetCount){
            bundleSelectIndex=0;
            if(!bundleChoice.Found()){
                std::uintptr_t native=0;
                if(a.Known(BundleNativeTargetIndex,native)&&a.HidingTargetAllowed(native,now)&&a.HeldRecoveryTargetAllowed(native,now)){
                    bundleTargetIndex=BundleNativeTargetIndex;bundleReport.target=native;
                    bundleSchedule.ContinueSelection(now,true);return;
                }
                if(bundleReport.target&&!a.RetireTarget())return;
                if(bundleCompletion.Ready(bundleReport.bots,!bundleHelpers.Active(),a.NativeTerminal(),now)){
                    bundleCompletionReady=true;
                }
                // Missing/retired pointers are not terminal evidence. Keep the
                // owned-controller refresh alive for native Tommy possession.
                bundleSchedule.ContinueSelection(now,true);return;
            }
            const bool indexedTrap=bundleTrapPriority.Active(now);
            const auto logicalIndex=indexedTrap?bundleTrapPriority.index:bundleChoice.Index(now<bundleTargetLockUntil,bundleChaseFatigue.Slot(now));
            const auto index=logicalIndex>bundleReport.bots?BundleNativeTargetIndex:logicalIndex;std::uintptr_t winner=0;
            if(!a.Known(index,winner)||!a.Alive(winner,living)||!living){bundleSchedule.ContinueSelection(now,false);return;}
            if(indexedTrap&&!a.TrapPriorityCurrent(winner)){
                bundleTrapPriority.Clear();bundleSchedule.ContinueSelection(now,false);return;
            }
            if(bundleReport.target!=winner){bundlePursuit.Clear();bundleFollowPath.Clear();bundlePathCooldown.Clear();bundleMoveAt=now;bundleTargetLockUntil=now+6000;}
            bundleTargetIndex=index;bundleReport.target=winner;++bundleReport.targetSweeps;
            bundleSchedule.ContinueSelection(now,true);
        }
        else bundleSchedule.ContinueSelection(now,false);
        return;
    }
    std::uintptr_t target=0;float squared=0;
    if(!a.Known(bundleTargetIndex,target)||!a.Alive(target,living)||!living){
        if(bundleReport.target)a.RetireTarget();
        bundleSchedule.selectAt=now;bundleTargetIndex=0;return;
    }
    if(!a.HeldRecoveryTargetAllowed(target,now)){bundleSchedule.selectAt=now;return;}
    if(bundleTrapPriority.Active(now)&&target!=bundleTrapPriority.victim){
        bundleTrapPriority.Clear();bundleSchedule.selectAt=now;return;
    }
    if(!a.NativeBindings()){bundleReport.resourceFailure=5;BundleStop(cfgbundle::Stop::Resources);return;}
    const auto manager=a.ManagerForAction(now);
    if(manager!=BundleManagerGateResult::Ready){
        bundleReport.resourceFailure=6;
        if(manager==BundleManagerGateResult::Deferred)return; // a later callback gets a fresh bounded observation
        BundleStop(cfgbundle::Stop::Resources);return;
    }
    auto hidingTarget=target;
    if(HostedAdoptionOwnsDriver()){
        const auto donor=bundleDonorPrimary.stallTarget;
        const auto index=donor?bundleDonorRoster.roster.DonorFindJasonAITargetIndex(donor):-1;
        bool donorLive=false;
        const bool current=index>=0&&bundleDonorPrimary.frame.ObservedLive(donor)&&a.Alive(donor,donorLive)&&donorLive;
        hidingTarget=cfgh241hiding::HidingTarget(true,donor,bundleDonorPrimary.stallTargetAt,now,current,target);
#if CFG_H264_HIDING_ROSTER
        if(const auto hider=a.HostedNearestHider(now))hidingTarget=hider;
#endif
    }
    if(a.HidingPhase(hidingTarget,now))return; // hosted identity comes from the fresh donor target
    bool stunned=false;if(!a.NativeBool(cfghunter::disk::Stunned,stunned)){BundleStop(cfgbundle::Stop::NativeException);return;}
    if(stunned){++bundleReport.stunSkips;return;}
    bool attacking=false;if(!a.NativeBool(cfghunter::disk::Attacking,attacking)){BundleStop(cfgbundle::Stop::NativeException);return;}
    if(attacking){++bundleReport.attackAnimations;
        if(now<bundleSchedule.meleeUntil){bundleSchedule.meleeUntil=now+1000;bundleSchedule.huntAt=now+25;}
        return;} // readiness polling never overwrites an active native swing
    if(a.EscapeGuardObserve(now))return;
    cfgspawn::Vec3 jasonAt{},targetAt{};
    if(!a.Distance(target,squared,jasonAt,targetAt)){bundleReport.resourceFailure=7;BundleStop(cfgbundle::Stop::Resources);return;}
    if(!NativePrimaryHuntingActive()){
    bundleHuntTracker.Observe(bundleTargetIndex,target,{targetAt.x,targetAt.y,targetAt.z},now);
    bundleHuntProgress.Observe(target,squared,now);
    {const unsigned slot=bundleTargetIndex==BundleNativeTargetIndex?bundleReport.bots+1:bundleTargetIndex; // selection-slot space
     const auto before=bundleChaseFatigue.Slot(now);bundleChaseFatigue.Observe(slot,target,bundleHuntProgress,now);
     if(before!=bundleChaseFatigue.Slot(now)){bundleSchedule.selectAt=now; // re-choose on the next selection lane
        if(bundleFatigueTraces<16){++bundleFatigueTraces;cfgclosedtrace::Mark("hunt-chase-fatigue",target,slot);}}}
    }
    bundleSchedule.CloseCombat(now,squared);
    if(a.EscapeGuardAct(now,jasonAt,squared))return;
    // A proved selected door decides BEFORE optional Morph/car scans or a
    // new speed burst. Those probes may not repeatedly postpone the action.
    const auto door=a.DoorRecovery(now,squared,target);
    if(door==cfgdoor::Result::Fatal){
        BundleStop(bundleReport.resourceFailure==1?cfgbundle::Stop::Context:
            bundleReport.resourceFailure==2?cfgbundle::Stop::JasonIdentity:cfgbundle::Stop::Resources);return;
    }
    if(door==cfgdoor::Result::Acted)return;
    if(door==cfgdoor::Result::Unavailable){++bundleReport.doorUnavailable;
        if(bundleDoorRecovery.phase||bundleDoorRecovery.breaking||a.Reads()>cfgbundle::ReadCap-256)return;}
    const bool optionalProbe=bundleHuntProbe.MayProbe();
#if CFG_H245_PURSUIT && defined(CFG_H1_JASON)
    // Report-only: legacy Morph/Shift lanes that the movement lease closed.
    ++hostedDriverTrace.huntLaneReached;
    if(DonorNativeMovementLeasePublished()&&!CFG_H246_BURST){
        if(squared>=400.f*400.f&&squared<=2000.f*2000.f&&!bundleFootBurst.Owned()&&now>=bundleFootBurst.readyAt)
            ++hostedDriverTrace.burstWantedLeaseBlocked;
        if(optionalProbe&&now>=bundleCounselorMorphCheckAt&&now>=bundleCarMorph.next&&squared>2000.f*2000.f)
            ++hostedDriverTrace.morphWantedLeaseBlocked;
    }
#endif
    const bool leaseNow=DonorNativeMovementLeasePublished();
    cfgh246::MorphQueueInputs morphIn;
    morphIn.leasePublished=leaseNow;morphIn.optionalProbe=optionalProbe;
    if(leaseNow&&CFG_H246_BURST&&optionalProbe&&now>=bundleCounselorMorphCheckAt){
        // Keep the same stall sampler J2.19 runs inside the Morph service, without queueing (bit 10).
        a.MorphProgressSample(now,target);
        morphIn.probeMorphFirst=bundleHuntProbe.MorphFirst(cfgfootcar::Obstacle::InRange(squared));
        morphIn.donorNavPending=DonorNavPendingForLegacy();morphIn.donorTrapIntent=DonorTrapIntentForLegacy();
        morphIn.morphDueSuperset=cfgpursuit::MorphDue(bundleCarMorph,now,squared,false,bundlePursuitProgress.Stalled(),bundlePursuitProgress.Stalled());
        morphIn.doorGraceOver=now>=bundleDoorRecovery.traversalUntil;morphIn.retryOver=now>=bundlePursuit.retryAt;
        morphIn.moveOver=bundlePursuitProgress.Stalled()||now>=bundleMoveAt;
        morphIn.cabinsReady=bundleCabins.cache.stage==cfgcabin::Stage::Ready;
    }
    if(((!leaseNow&&optionalProbe&&bundleHuntProbe.MorphFirst(cfgfootcar::Obstacle::InRange(squared)))||(leaseNow&&cfgh246::MorphQueue(morphIn)))&&now>=bundleCounselorMorphCheckAt&&!bundleFootBurst.Owned()&&!bundleFootObstacle.stage&&
       !bundleCarPursuit.Active(now)&&!bundleVehicleActions.StopOwnsEpisode()&&!bundleCarSpeed.active&&
       !bundleTraps.Busy()&&!bundleHiding.Active()&&!bundleDoorRecovery.breaking&&!bundleDoorRecovery.phase&&!bundleKnife.Busy()&&
       !H247MorphBusyVeto(a,leaseNow,now,1)){
#ifdef CFG_H1_JASON
        if(leaseNow)++hostedDriverTrace.h246MorphLeaseQueued;
#endif
        bundleHuntProbe.Queued();bundleCounselorMorphCheckAt=now+1000;bundleCounselorMorphTarget=target;bundleSchedule.huntAt=now+100;return;
    }
    if(!cfgfootcar::Obstacle::InRange(squared)||(bundleFootObstacle.driver&&bundleFootObstacle.driver!=target))bundleFootObstacle.Reset();
    else if(bundleFootObstacle.Protected(now))return;
    if(optionalProbe&&cfgfootcar::Obstacle::InRange(squared)&&now>=bundleFootObstacleCheckAt&&
       !bundleCarPursuit.Active(now)&&!bundleVehicleActions.StopOwnsEpisode()&&!bundleCarSpeed.active&&
       !bundleTraps.Busy()&&!bundleFootBurst.Owned()&&!bundleHiding.Active()&&
       !bundleDoorRecovery.breaking&&!bundleDoorRecovery.phase&&!bundleKnife.Busy()&&!bundleKnifeProbePending){
        bundleHuntProbe.Queued(true);bundleFootObstacleCheckAt=now+1000;bundleFootObstacleTarget=target;bundleSchedule.huntAt=now+100;return;
    }
    // H2.85: within 30 m a pending knife OPPORTUNITY probe no longer vetoes the burst (a real throw still interrupts it via Knife.Busy).
    const bool chaseRange=CFG_H285_CHASE_BURST&&std::isfinite(squared)&&squared<=cfgfootburst::ChaseRangeCm*cfgfootburst::ChaseRangeCm;
    if(cfgh246::BurstWanted(DonorNativeMovementLeasePublished(),squared,bundleFootBurst.Owned(),now,bundleFootBurst.readyAt)){
        const unsigned why=bundleCarPursuit.Active(now)?1u:bundleCarSpeed.active?2u:bundleTraps.Busy()?3u:bundleHiding.Active()?4u:
            (bundleDoorRecovery.breaking||bundleDoorRecovery.phase)?5u:bundleKnife.Busy()?6u:(bundleKnifeProbePending&&!chaseRange)?7u:0u;
        if(why&&why<bundleReport.h285Veto.size())++bundleReport.h285Veto[why];
        if(bundleKnifeProbePending&&chaseRange&&!why)++bundleReport.h285Veto[0]; // probe pending, burst allowed by H2.85
    }
    if(cfgh246::BurstWanted(DonorNativeMovementLeasePublished(),squared,bundleFootBurst.Owned(),now,bundleFootBurst.readyAt)&&
       !bundleCarPursuit.Active(now)&&!bundleCarSpeed.active&&!bundleTraps.Busy()&&!bundleHiding.Active()&&
       !bundleDoorRecovery.breaking&&!bundleDoorRecovery.phase&&!bundleKnife.Busy()&&(!bundleKnifeProbePending||chaseRange)){
        // Start on a later independently budgeted callback, never inherit the
        // hunt's stun/manager/distance debt into four movement-field proofs.
        bundleFootBurstRequestAt=now;bundleFootBurstSquared=squared;++bundleReport.h285BurstRequests;return;
    }
    if(!NativePrimaryHuntingActive()&&now>=bundleKnifeProbeAt&&now>=bundleKnife.nextAt&&squared>=850.f*850.f&&squared<=1550.f*1550.f){
        bundleKnifeProbeAt=now+1000;bundleKnifeProbeIndex=bundleTargetIndex;
        // Donor combat refreshes nearest living counselor before throwing,
        // independently of the six-second CHASE target lock. Reuse only the
        // bounded known native helper/local roster, one candidate per fresh
        // callback. Begin revalidates owner, target, range and driver afterward.
        if(!bundleTrapPriority.Active(now))bundleKnifeRefresh.Start(bundleReport.bots,now);
        bundleKnifeProbePending=true;return;
    }
    // Exterior projection/Morph is serviced separately above. A failed/cold
    // optional proof must not suppress ordinary MoveTo or melee indefinitely.
    if(NativePrimaryHuntingActive())return; // no second slash/grab/MoveTo/intercept or old on-foot timing policy
    if(bundleSchedule.AttackDue(now,squared)){
        bool allowed=false;if(!a.NativeBool(cfghunter::disk::CanAttack,allowed)){BundleStop(cfgbundle::Stop::NativeException);return;}
        if(allowed){
            if(!a.Context()||!a.Jason()){BundleStop(cfgbundle::Stop::Input);return;}
            const auto attackManager=a.ManagerForAction(now);
            if(attackManager!=BundleManagerGateResult::Ready){
                bundleReport.resourceFailure=6;
                if(attackManager==BundleManagerGateResult::Deferred)return;
                BundleStop(cfgbundle::Stop::Input);return;
            }
            if(!a.Held(held)||held){BundleStop(cfgbundle::Stop::Input);return;}
            if(!bundleGrabUnavailable&&bundleGrabWarm==6&&bundleGrab.ChooseGrab(squared)){
                bundleSchedule.attackAt=now+2600;bundleMoveAt=now+1000;
                const auto attempts=bundleReport.grabAttempts;
                a.BeginGrab();
                // No native grab input occurred if range/resource proof
                // refused before Attempts. Keep full recovery after ANY
                // attempted call, including an ambiguous exception.
                if(bundleReport.grabAttempts==attempts)bundleSchedule.attackAt=now+100;
                return;
            }
            bundleSchedule.Pressed(now); // obligation published before potentially side-effecting native input
            if(!a.Input(false)){BundleStop(cfgbundle::Stop::Input);return;}
            ++bundleReport.pressReturns;bundleMoveAt=now+1000;return;
        }
        ++bundleReport.notReady;
    }
    // Cut off a mid-range runner instead of trailing it (native MoveToActor
    // otherwise follows its CURRENT position for 6 s). Close range, vehicles,
    // traps, hiding, doors and knife keep their existing owners.
    if(now>=bundleMoveAt&&now>=bundleInterceptAt&&bundlePathCooldown.Due(now)&&
       !bundleCarPursuit.Active(now)&&!bundleCarSpeed.active&&!bundleTraps.Busy()&&!bundleHiding.Active()&&
       !bundleDoorRecovery.breaking&&!bundleDoorRecovery.phase&&!bundleKnife.Busy()&&!bundleTrapPriority.Active(now)&&
       a.Reads()<cfgbundle::ReadCap-256){
        const auto* track=bundleHuntTracker.Current(bundleTargetIndex,target,now);cfghunt::V3 goal{};
        if(track&&cfghunt::Intercept::Plan({jasonAt.x,jasonAt.y,jasonAt.z},*track,bundleHuntProgress.Stalled(target,now),goal)){
            bundleInterceptAt=now+450;unsigned result=255;
            if(a.MoveToPoint({goal.x,goal.y,goal.z},result)){
                bundleReport.lastMoveResult=result;bundleFollowPath.Clear();
                if(cfgbundle::ClassifyMove(result)==cfgbundle::MoveOutcome::Refused){
                    ++bundleInterceptRefusals;bundleInterceptAt=now+1500; // fall back to actor follow below
                }else{
                    ++bundleIntercepts;++bundleReport.moves;
                    if(bundleInterceptTraces<16&&(bundleIntercepts==1||bundleIntercepts%64==0)){++bundleInterceptTraces;
                        cfgclosedtrace::Mark(bundleHuntProgress.Stalled(target,now)?"hunt-intercept-loop":"hunt-intercept",target,
                            (static_cast<std::uint64_t>(bundleIntercepts)<<32)|(static_cast<std::uint64_t>(std::sqrt(squared))&0xFFFFFFFFull));}
                    return;
                }
            }else{++bundleInterceptUnavailable;bundleInterceptAt=now+5000;} // binding unavailable: ordinary follow continues
        }
    }
    if(now>=bundleMoveAt&&bundlePathCooldown.Due(now)){
        if(bundleFollowPath.Preserve(now,target)){++bundleReport.pathPreserved;return;}
        if(!a.Move(target)){BundleStop(cfgbundle::Stop::Resources);return;}
        bundlePathCooldown.Completed(now,bundleReport.lastMoveResult);
        bundleFollowPath.Completed(now,target,bundleReport.lastMoveResult);
        bundleChaseRecovery.Observe(target,bundleReport.lastMoveResult,squared,now);
        if(bundleReport.lastMoveResult!=0)++bundleReport.moves;
        bundleMoveAt=bundlePathCooldown.next;
    }
}
#ifdef CFG_RETAINED_GAMEPLAY_BUNDLE
bool bundlePresentationPending=false;
#endif
bool ServiceBundlePresentation(void* object,bool wasNested)noexcept{
    if(wasNested||GetCurrentThreadId()!=plan.threadId)return false;
    const auto actor=reinterpret_cast<std::uintptr_t>(object);
    const bool gameplayActor=cfgbundle::OwnedCallback(actor,plan.pawn,bundleJason.pawn.object,BundleGameplayStarted(),
        plan.controller,(bundleCompletion.terminalMask&1u)!=0);
#ifndef CFG_RETAINED_GAMEPLAY_BUNDLE
    const auto now=GetTickCount64();
#endif
    // Native match completion need not pass through custom completion flags.
    // Mode/state/controller callbacks survive mortal-pawn teardown.
    if(gameplayActor||actor==plan.controller||actor==plan.mode||actor==spawnAssets.gameState){
#ifdef CFG_RETAINED_GAMEPLAY_BUNDLE
        // Pre-forward observations cannot execute under a completed-tick lease.
        // Keep the opportunity; native presentation runs in the existing queued
        // service after an independently verified return, never on this hook.
        bundlePresentationPending=true;
#else
        if(BundleEndingSummaryService(now))return true;
#endif
    }
    return false;
}
#ifdef CFG35_URINATION_NATIVE_PORT
bool BundleUrinationService(std::uint64_t now)noexcept;
bool BundleTimedLegacyUrination(std::uint64_t now)noexcept{J21Timing timing(cfgj21::LegacyUrination);return BundleUrinationService(now);}
bool bundleUrinationLastTurn=false;
#else
void BundleRBConfirmationFailed()noexcept{}
bool BundleRBCancelCurrent(std::uintptr_t)noexcept{return true;}
#endif
__declspec(noinline) void ResetBundleWorldState(std::uint64_t now)noexcept{
        bundleNativeTarget={};bundleNativeTargetHint=0;bundleNativeTargetHintIndex=0;
        bundleBotNames.seed=static_cast<unsigned>(now);
#if defined(CFG_RETAINED_GAMEPLAY_BUNDLE) && defined(CFG_FRONTEND_BUNDLE)
        if(CFG_J24_FAILED_START_RETIRE)J24WorldInitialized();
#endif
#if defined(CFG_RETAINED_GAMEPLAY_BUNDLE) && defined(CFG_FRONTEND_BUNDLE)
        bundleDonorRoster={};bundleDonorPrimary={};bundleDonorAdditional={};bundleDonorHeldPrimary={};bundleDonorHeldAdditional={};bundleDonorConvergence={};bundleDonorPostGrab={};
        bundleDonorSurveyBackoff={};bundleObjectiveTurnAt=0;bundleObjectiveWasLast=false;
        bundleJ21Lean={};
        bundleEmotes={}; // new world: no old selection, prepare refusal, queue or montage receipt survives
        EnsureBundleSweaterMatch(now);
#endif
        bundleDeadSpectatorRecovery={};bundleJasonLabelOwned=false;
        bundleSpectatorLabelPending=false;bundleSpectatorLabelAttempts=0;bundleSpectatorLabelAt=0;
        bundleKnife={};bundleKnifeFirstAbortTrace={};bundleKnifeRefresh={};bundleKnifeProbePending=false;bundleKnifeProbeAt=0;
        ResetBundleManagerRetry();
        bundlePostMatchRecovery={};bundlePostMatchWatch={};
        bundleObjectiveCarCache={};bundleObjectiveCarPublication={};bundleObjectiveCarProof={};
        bundleRadioObjective={};bundleRadioObjectiveSince=bundleRadioObjectiveNext=0;
        bundleRadioObjectiveBegun=bundleRadioObjectiveAttempted=bundleRadioObjectiveFinished=false;
        bundleObjectiveStackAt=0;bundleObjectiveStackLast={};bundleObjectiveStackTop=0;bundleObjectiveStackVisits=0;
        bundleTrapPriority.Clear();bundleTargetLockUntil=0;bundleTrapEvents=bundleTrapAccepted=bundleTrapRefused=bundleTrapMaxReads=0;
        bundleConvergenceContinuation={};bundleAxeContinuation={};
        bundleConvergenceTravel={};bundleConvergenceAxes={};bundleTravelCursor=bundleTravelAcquires=bundleTravelMoves=bundleTravelResumes=bundleTravelLost=0;
        bundleConvergenceFearPending=bundleConvergenceFearWrites=bundleConvergenceFearRefused=0;
        bundleTravelAt=0;bundleTravelTurn=false;bundleRun={};bundleFootBurst={};bundleFootMovement={};bundleFootBurstRequestAt=0;
        bundleCounselorMorphTarget=0;bundleCounselorMorphCheckAt=0;bundleEscapeGuard.Clear();
        bundleHuntProbe={};
        bundleEndingSummary={};
        bundleSweaterPickup={};bundleCabins={};bundleCabinsNext=0;bundleLateMaskRetryUsed=false;
        bundleCarPursuit={};bundleCarIntercept={};bundleRoads={};bundleRoadTrace={};bundleRoadMorph={};bundleRoadAttemptAt=0;bundleSweaterClass={};cfgh241mask::memo.Clear();cfgh241sweater::memo.Clear();bundleSweaterClassAttempted=false;bundleSweaterClassPending=false;bundleSweaterFreshPending=false;bundleSweaterFreshRequested=false;bundleSweaterFreshUse=0;bundleLoadoutResource={};bundleExpandedLoadout={};bundleVehicleActions={};bundleCarSpeed={};bundleCarActionPending={};bundleCarSpeedRetryAt=0;bundleSweaterOwner=plan.objects[3];bundleSweaterPolicy={};bundleSweaterAbility={};bundleSweaterSource={};bundleSweaterRetiredAbility={};bundleSweaterFreshGrantAttempted=false;bundleSweaterCursor=bundleSweaterSettingsCursor=0;bundleSweaterSourceIndex=~0u;bundleHiding={};bundleHidingRefused=0;bundleSpectatorStatusFunction=0;bundleSpectatorStatusWidgetHint=0;bundleSpectatorStatusName={};bundleSpectatorStatusNameReady=false;bundleSpectatorStatusAt=0;
}
#include "BundleInitializationRuntime.hpp"
void ServiceBundle(void* object,bool wasNested)noexcept{
    if(wasNested||GetCurrentThreadId()!=plan.threadId)return;
    const auto actor=reinterpret_cast<std::uintptr_t>(object);
    const bool gameplayActor=cfgbundle::OwnedCallback(actor,plan.pawn,bundleJason.pawn.object,BundleGameplayStarted(),
        plan.controller,(bundleCompletion.terminalMask&1u)!=0);
    const auto now=GetTickCount64();
    if(!gameplayActor)return;
#ifdef CFG_H1_JASON
    if(HostedRoutineActive()&&cfghostedlabel::RefreshDue(now,(bundleCompletion.terminalMask&1u)!=0,hostedSpectatorHintRetryAt)){
        BundleAccess label(1024);
        if(!label.HostedSpectatorLabelHint()){
            bundleSpectatorStatusWidgetHint=0;bundleSpectatorStatusNameReady=false;
        }
    }
    if(HostedRoutineActive()&&now>=hostedRoutineRosterAt){
        hostedRoutineRosterAt=now+1000;BundleAccess census(8192);(void)census.HostedRoutineRoster();
    }
    if(HostedRoutineActive()&&bundleReport.bots&&now>=hostedBrainAt){
        BundleAccess brains(8192);hostedBrainAt=now+500;(void)brains.HostedCounselorBrainPhase();
    }
#endif
    if(bundleCompletion.dispatched&&!bundleSchedule.active){
        if(bundlePostMatchWatch.Due(now)){
            InterlockedIncrement(reinterpret_cast<volatile LONG*>(&bundleReport.generation));
            BundleAccess observation;observation.ObservePostMatch();
            bundleReport.reads+=observation.Reads();bundleReport.maxReads=(std::max)(bundleReport.maxReads,observation.Reads());
            MemoryBarrier();InterlockedIncrement(reinterpret_cast<volatile LONG*>(&bundleReport.generation));
        }
        return; // finite read-only presentation witness; no gameplay restart
    }
    if(!BundleGameplayStarted()){
        if(spawnRuntime.report.state!=static_cast<unsigned>(cfgspawn::SpawnState::MoveSubmitted))return;
        bundleStarted=true;ResetBundleWorldState(now);ResetBundleManagerRetry();
        InterlockedIncrement(reinterpret_cast<volatile LONG*>(&bundleReport.generation));
        BundleAccess a;
        if(BundleInitializeCore(a,now)){
#ifdef CFG_FRONTEND_BUNDLE
            if(NativeOfflineWorld()){
                if(!BundleInitializeCheck(cfgbundle::InitializationStep::NativeSchedule,bundleSchedule.AdoptNativeHelpers(bundleReport.spawned))){BundleStop(cfgbundle::Stop::Resources);return;}
            }
#endif
            bundleSchedule.settingsActive=true;bundleSchedule.settingsAt=now+100;
            if(bundleConfiguration.startingWeapon||bundleConfiguration.startingInventory){
                if(bundleExpandedLoadout.Begin(cfgofflinesettings::FromConfiguration(bundleConfiguration),
                    {bundleConfiguration.startingWeapon,bundleConfiguration.startingInventory},now)){
                    bundleSchedule.backgroundLoadout=true;bundleSchedule.loadoutActive=bundleExpandedLoadout.Active();bundleSchedule.loadoutAt=now+3000;
                }
            }else if(bundleLoadout.Begin(cfgloadout::restricted::FromConfiguration(bundleConfiguration),now)){
                bundleSchedule.loadoutActive=bundleConfiguration.restrictedInventory!=0;bundleSchedule.loadoutAt=now+3000;
            }
            BundlePublishInitialized();
            bundleReport.requestedBots=bundleConfiguration.counselorBots;
            bundleReport.selectedJasonIndex=bundleConfiguration.jasonCatalogueIndex;
            bundleReport.submission=bundleConfiguration.submission;
            bundleReport.trapMapFamily=static_cast<unsigned>(bundleTrapFamily);
            for(unsigned i=0;i<bundleTrapMapPackage.count&&i+1<bundleReport.trapMapPackage.size();++i)
                bundleReport.trapMapPackage[i]=static_cast<char>(bundleTrapMapPackage.data[i]);
            bundleTrapInputs={plan.module,plan.world,plan.mode,spawnAssets.gameState,bundleJason.pawn.object,bundleJason.killerController.object,
                bundleJason.pawn,bundleJason.killerController,bundleTrapFamily,bundleTrapMapWorld==plan.world,
                cfgtrap::startup::NoCarFallbackMap(bundleTrapFamily,bundleTrapMapWorld==plan.world,bundleTrapMapPackage.count>0)};
            bundleTraps.Start(now,cfgtrap::startup::EffectiveFamily(bundleTrapFamily,bundleTrapInputs.fallbackMap),bundleTrapInputs.verifiedMap);
            bundleReport.trapStage=static_cast<unsigned>(bundleTraps.report.stage);
            bundleReport.trapFailure=static_cast<unsigned>(bundleTraps.report.failure);
#if defined(CFG_RETAINED_GAMEPLAY_BUNDLE) && defined(CFG_FRONTEND_BUNDLE)
            bundleNativeJasonDonorService=&ServiceDonorNativeHunting;
            if(!PrepareNativeJasonControllerTick(plan.module)){
                // Refuse the candidate rather than silently claiming that
                // the old ProcessEvent AI is the new donor controller port.
                cfgclosedtrace::Mark("donor-controller-hook-refused",plan.module,0);
                BundleStop(cfgbundle::Stop::Resources);
            }
#endif
        }
        else BundleStop(cfgbundle::Stop::Resources);
        bundleReport.error=a.Error();bundleReport.maxReads=a.Reads();bundleReport.maxCalls=a.Calls();
        MemoryBarrier();InterlockedIncrement(reinterpret_cast<volatile LONG*>(&bundleReport.generation));return;
    }
    if(bundleSchedule.active&&(now>=bundleSchedule.expires||bundleReport.phases>=cfgbundle::MaximumPhases)){
        InterlockedIncrement(reinterpret_cast<volatile LONG*>(&bundleReport.generation));BundleStop(cfgbundle::Stop::Limit);
        MemoryBarrier();InterlockedIncrement(reinterpret_cast<volatile LONG*>(&bundleReport.generation));
    }
    BundleExtraHelperInput(now);
#ifdef CFG_H1_JASON
    HostedH1ServiceAdditionalRequests(now);
    HostedH1ServicePrimaryPresentation(now);
#endif
    // Core traps, objectives, status, helpers and cleanup are never optional
    // B/C work. In particular, an unfinished trap may own the donor Hunt lane.
    const bool additionalWasLast=bundleAdditional.lastTurn;bundleAdditional.lastTurn=false;
#ifdef CFG35_URINATION_NATIVE_PORT
    const bool urinationWasLast=bundleUrinationLastTurn;bundleUrinationLastTurn=false;
#else
    constexpr bool urinationWasLast=false;
#endif
    const bool independentWasLast=additionalWasLast||bundleRBLastTurn||bundleSummaryLastTurn||bundleJasonLabelLastTurn||urinationWasLast||bundleObjectiveWasLast;
    bundleObjectiveWasLast=false;
    // Only terminal local humans pay for the camera lane. It gets its own
    // shared scheduling slice; never stacks native calls with AI maintenance.
    const bool spectatorDue=!bundleSchedule.pressed&&bundleSchedule.active&&!DonorFinalVictimOwnsCamera()&&cfgbundle::OfflineSpectatorCameraAllowed(HostedRoutineActive(),bundleCompletion.terminalMask)&&
        now>=bundleSpectatorAt&&!bundleKill.policy.Exclusive(now)&&!bundleDeathEnd.confirmedAt;
    // Fair even at low FPS: a previous camera slice cannot repeatedly win
    // against AI/completion. Due() consumes a gameplay scheduling turn only
    // when this callback actually considers gameplay.
    const bool knifeDue=((bundleKnife.Busy()&&now>=bundleKnife.pollAt)||bundleKnifeProbePending)&&
        !bundleKill.policy.Exclusive(now)&&!bundleDeathEnd.confirmedAt&&!bundleCompletionReady;
    const bool trapDue=bundleTraps.Due(now)&&!bundleTraps.Paused()&&
        !bundleKill.policy.Exclusive(now)&&!bundleDeathEnd.confirmedAt&&!bundleCompletionReady;
    const bool inputAllowed=!(bundleCompletion.terminalMask&1u)&&!bundleKill.policy.InputExclusive(now)&&!bundleDeathEnd.confirmedAt&&!bundleCompletionReady;
    // Sample BEFORE choosing/consuming a gameplay turn. A pending wheel step
    // gets one fair independent callback, never two consecutive turns. Owed
    // release, exclusive native action and active knife work remain ahead.
    bundleRBEmote.Sample(now,inputAllowed);
    if(!inputAllowed)bundleRBEmote.RequestRelease();
    const bool wheelTurn=cfgoptionalwork::WheelTurn(bundleSchedule.active,bundleSchedule.pressed,
        !inputAllowed||bundleKnife.Busy(),knifeDue,
        bundleRBEmote.edges.pending||
        (bundleRBEmote.policy.phase!=cfgrbemote::Phase::Idle&&
         (bundleRBEmote.policy.phase!=cfgrbemote::Phase::Open||!bundleRBEmote.edges.latest.down)),independentWasLast);
    const bool additionalTurn=!wheelTurn&&!independentWasLast&&!bundleSchedule.pressed&&
        cfgbundle::AdditionalJasonLaneEnabled(bundleSchedule.active,HostedRoutineActive())&&
        ((!bundleKill.policy.Exclusive(now)&&!bundleDeathEnd.confirmedAt)||bundleAdditional.policy.pressed)&&
        (bundleAdditional.policy.Due(now)||(bundleAdditional.rollback&&now>=bundleAdditional.policy.next));
    const bool labelTurn=!wheelTurn&&!additionalTurn&&!independentWasLast&&!bundleSchedule.pressed&&
        (bundleSchedule.active&&(bundleCompletion.terminalMask&1u))&&
        !bundleKill.policy.Exclusive(now)&&!bundleDeathEnd.confirmedAt&&
        bundleSpectatorLabelPending&&now>=bundleSpectatorLabelAt;
    const bool objectiveTurn=cfgj21::ObjectiveTurn(bundleSchedule.active,bundleSchedule.pressed,
        bundleKill.policy.Exclusive(now)||bundleDeathEnd.confirmedAt||bundleCompletionReady,knifeDue||trapDue,
        independentWasLast,wheelTurn||additionalTurn||labelTurn,now>=bundleObjectiveTurnAt);
    const bool summaryTurn=cfgoptionalwork::SummaryTurn(bundleSchedule.active,bundleSchedule.pressed,
        bundleKill.policy.Exclusive(now)||bundleDeathEnd.confirmedAt||bundleCompletionReady,
        knifeDue||trapDue,independentWasLast,wheelTurn||additionalTurn||labelTurn||objectiveTurn,now>=bundleSummary.next);
    const bool independentTurn=wheelTurn||additionalTurn||labelTurn||summaryTurn||objectiveTurn;
    const auto turn=independentTurn?cfgbundle::CameraTurn{}:
        cfgbundle::SelectActionTurn(bundleSchedule,now,spectatorDue,bundleSpectatorLast,knifeDue,trapDue);
    bundleRBLastTurn=wheelTurn;
    bundleSummaryLastTurn=summaryTurn;
    bundleJasonLabelLastTurn=labelTurn;
    bundleObjectiveWasLast=objectiveTurn;
    const bool spectator=turn.camera;
    auto lane=turn.gameplay;
    const bool input=wheelTurn||(!independentTurn&&!spectator&&lane==cfgbundle::Lane::None&&!bundleSchedule.pressed&&bundleSchedule.active);
    const bool mirror=!cfgj21::EmotesEnabled&&!independentTurn&&!spectator&&lane==cfgbundle::Lane::None&&!bundleSchedule.pressed&&bundleSchedule.active&&
        (bundleEmotes.Due(now)||BundleEmoteMaintenanceDue(now))&&!bundleKill.policy.InputExclusive(now)&&!bundleDeathEnd.confirmedAt;
    const bool acceptedCarPath=!spectator&&!mirror&&lane==cfgbundle::Lane::Vehicle&&!bundleSchedule.pressed&&bundleSchedule.active&&
        bundleCarPursuit.Active(now)&&bundleVehicleActions.moving&&bundleVehicleActions.haveHoodGoal&&bundleVehicleActions.phase==0&&
        !bundleCarActionPending.active&&!bundleRoadMorph.stage&&!bundleTraps.Busy()&&!bundleTrapPriority.Active(now)&&
        !bundleKnife.Busy()&&!bundleKnifeProbePending&&!bundleKill.policy.Exclusive(now)&&!bundleDeathEnd.confirmedAt&&!bundleCompletionReady;
    const bool continuePublication=bundleConvergenceContinuation.Take(now,bundleConvergencePublication.Active(),
        !independentTurn&&!spectator&&!mirror&&lane==cfgbundle::Lane::None&&!bundleSchedule.pressed&&bundleSchedule.active&&
        !bundleKill.policy.Exclusive(now)&&!bundleDeathEnd.confirmedAt,acceptedCarPath);
    if(continuePublication&&acceptedCarPath)bundleSchedule.vehicleAt=now+100;
    const bool continueAxes=bundleAxeContinuation.Take(now,bundleConvergence.active&&bundleConvergenceAxes.ContinuationDue(now),
        !independentTurn&&!continuePublication&&!bundleConvergencePublication.Active()&&!spectator&&!mirror&&lane==cfgbundle::Lane::None&&!bundleSchedule.pressed&&bundleSchedule.active&&
        !bundleKill.policy.Exclusive(now)&&!bundleDeathEnd.confirmedAt);
    if(continuePublication||continueAxes)lane=cfgbundle::Lane::Convergence;
    if(!independentTurn&&!spectator&&!mirror&&!bundleSchedule.pressed&&bundleSchedule.active&&lane==cfgbundle::Lane::None&&
       !bundleKill.policy.Exclusive(now)&&!bundleDeathEnd.confirmedAt&&
       bundleTraps.report.stage==cfgtrap::startup::Stage::Discover&&bundleTraps.Due(now))lane=cfgbundle::Lane::Hunt;
    const bool pamelaOpportunity=bundleKill.policy.pamela&&now>=bundleKill.policy.usedAt&&now<bundleKill.policy.until&&
        !bundleKill.policy.interactionAttempted&&!bundleSchedule.pressed;
    if(!spectator&&!mirror&&!input&&!independentTurn&&lane==cfgbundle::Lane::None&&!pamelaOpportunity)return;
    InterlockedIncrement(reinterpret_cast<volatile LONG*>(&bundleReport.generation));
    LARGE_INTEGER started{},finished{},frequency{};QueryPerformanceCounter(&started);QueryPerformanceFrequency(&frequency);
    BundleAccess a;
    const bool nativeContextWorked=cfgpamela::ContextTurn(pamelaOpportunity,spectator,independentTurn,mirror,input)&&
        a.PamelaNativeContextPhase(now);
    const bool inputWorked=!nativeContextWorked&&!objectiveTurn&&input&&a.RBStep(now,inputAllowed);
    if(!independentTurn)bundleSpectatorLast=spectator;
    if(nativeContextWorked){ /* fresh native context proof owns this bounded callback; owed releases remain first */ }
    else if(spectator){
        bundleSpectatorAt=now+50;DWORD pid=0;const auto foreground=GetForegroundWindow();
        if(foreground)GetWindowThreadProcessId(foreground,&pid);
        if(pid!=GetCurrentProcessId()||now<bundleSpectatorInputPendingAt||now-bundleSpectatorInputPendingAt>15000)bundleSpectatorInputPending=false;
        const bool f4=pid==GetCurrentProcessId()&&((GetAsyncKeyState(VK_F4)&0x8000)!=0||bundleSpectatorInputPending);
        const auto selections=bundleSpectator.report.selections;
        if(bundleSpectatorCycleHook.Due(now)){
            const bool ready=PrepareBundleSpectatorCycle(plan.module);
            bundleSpectatorCycleHook.Complete(ready,now);
            cfgclosedtrace::Mark(ready?"spectator-native-cycle-hook-ready":
                bundleSpectatorCycleHook.finished?"spectator-native-cycle-hook-exhausted":"spectator-native-cycle-hook-retry",
                plan.controller,bundleSpectatorCycleHook.attempts);
        }else if(!a.SpectatorWarm()){a.SpectatorStep(now,f4);bundleSpectatorInputPending=false;}
        if(bundleSpectator.Following()&&bundleSpectator.report.selections!=selections){
            bundleSpectatorLabelPending=true;bundleSpectatorLabelAttempts=0;bundleSpectatorLabelAt=now+1;
        }
        const auto& s=bundleSpectator.report;
        bundleReport.spectatorState=static_cast<unsigned>(s.result);bundleReport.spectatorSelections=s.selections;
        bundleReport.spectatorCameras=s.cameras;bundleReport.spectatorRestores=s.restores;bundleReport.spectatorRefusals=s.refusals;
        bundleReport.spectatorDecision=s.decision;
        bundleReport.spectatorLastRefusal.Capture(now,s.refusals,bundleReport.spectatorFailure,s.decision,
            bundleReport.spectatorLinkFailure,a.Reads(),a.Calls(),bundleReport.spectatorRosterCount,
            bundleReport.spectatorResourceBinding,bundleReport.spectatorResourceFailure,bundleReport.spectatorNativeBytes);
    }else if(inputWorked||wheelTurn){ /* One bounded wheel action/proof owns this callback. */ }
    else if(additionalTurn){a.AdditionalPhase(now);
#ifdef CFG_H1_JASON
        HostedH1ServiceAdditionalPresentation(); // J2.19 owns F1 birth and donor presentation on this turn
#endif
    }
    else if(labelTurn){BundleRefreshJasonStatus(now); /* bounded, fair native HUD refresh after spectator selection */ }
    else if(objectiveTurn){bundleObjectiveTurnAt=now+100;++bundleReport.j21ObjectiveTurns;a.ObjectivePhase(now);}
    else if(summaryTurn){J21Timing timing(cfgj21::Summary);a.SummaryPhase(now); /* fair native row maintenance */ }
    else if(lane==cfgbundle::Lane::None&&BundleRefreshJasonStatus(now)){ /* optional label work cannot discard an already-selected gameplay/release turn */ }
    else if(lane==cfgbundle::Lane::None&&cfgbundle::OfflineSpectatorCameraAllowed(HostedRoutineActive(),bundleCompletion.terminalMask)&&!bundleSchedule.pressed&&a.RecoverDeadSpectator(now)){ /* offline camera recovery only, not a consumed hunt/vehicle turn */ }
    else if(mirror){J21Timing timing(cfgj21::LegacyEmotes);a.MirrorService(now);}else if(lane!=cfgbundle::Lane::None)BundlePhase(a,lane,now,continuePublication,continueAxes);
#ifdef CFG35_URINATION_NATIVE_PORT
    else if(!cfgj21::EmotesEnabled&&!independentWasLast&&inputAllowed&&BundleTimedLegacyUrination(now)){bundleUrinationLastTurn=true; /* accepted J1a.2 optional urination service */ }
#endif
    else if(!bundleSchedule.pressed&&bundleSchedule.active&&!bundleKill.policy.Exclusive(now)&&!bundleDeathEnd.confirmedAt){
#if defined(CFG_RETAINED_GAMEPLAY_BUNDLE) && defined(CFG_FRONTEND_BUNDLE)
        if(now>=bundleDonorRoster.maintenanceAt&&bundleDonorRoster.censusComplete){
            cfgjasontick::Owner observation;observation.source=plan;
            a.DonorRosterSurvey(observation,now); // spare PE callback, never steals a donor decision
        }else
#endif
        if(a.CabinBoundsPhase(now)||a.ObjectivePhase(now))++bundleReport.phases;
    }
    a.KillNativeContextDiagnostic(now); // read-only; spare budget only, never takes a gameplay turn
    if(a.Exception())BundleStop(cfgbundle::Stop::NativeException);
    QueryPerformanceCounter(&finished);
    bundleReport.phase=static_cast<unsigned>(lane);if(!spectator&&(inputWorked||mirror||lane!=cfgbundle::Lane::None))++bundleReport.phases;
    bundleReport.reads+=a.Reads();bundleReport.calls+=a.Calls();bundleReport.error=a.Error();
#ifdef CFG_RETAINED_GAMEPLAY_BUNDLE
    bundleReport.actorRetentionPeakReads=(std::max)(bundleReport.actorRetentionPeakReads,a.bundleActorRetentionReads_);
#endif
    if(a.Reads()>bundleReport.maxReads)bundleReport.maxReads=a.Reads();if(a.Calls()>bundleReport.maxCalls)bundleReport.maxCalls=a.Calls();
    if(frequency.QuadPart>0){const auto elapsed=static_cast<std::uint64_t>((finished.QuadPart-started.QuadPart)*1000000/frequency.QuadPart);
        if(elapsed>bundleReport.maxMicroseconds)bundleReport.maxMicroseconds=elapsed;}
    bundleReport.releasePending=bundleSchedule.pressed?1u:0u;
    bundleReport.currentHumanTarget=bundleHumanTarget.object;
    bundleReport.sweaterOwner=bundleSweaterOwner.object;
    bundleReport.terminalCounselorMask=bundleCompletion.terminalMask;
    bundleReport.grabWarm=bundleGrabWarm;
    bundleReport.knifeStage=static_cast<unsigned>(bundleKnife.stage);
    bundleReport.endingRepairStage=static_cast<unsigned>(bundleEnding.stage);
    bundleReport.carHoodAttempts=bundleVehicleActions.hoodAttempts;bundleReport.carSeatAttempts=bundleVehicleActions.seatAttempts;
    bundleReport.carActionRefusals=bundleVehicleActions.refusals;bundleReport.carSpeedWrites=bundleVehicleActions.speedWrites;
    bundleReport.carSpeedRestores=bundleVehicleActions.speedRestores;bundleReport.carSpeedActive=bundleCarSpeed.active?1u:0u;
    bundleReport.carSlamObserved=bundleVehicleActions.slamObserved?1u:0u;
    bundleReport.carRouteCommitted=bundleVehicleActions.extractionCommitted?1u:0u;
    bundleReport.carExtractionCompleted=bundleVehicleActions.extractionCompleted;bundleReport.carSeatBatch=bundleVehicleActions.seatBatch;
    bundleReport.carExtractionCommitted=bundleVehicleActions.extractionCommitCount;
    bundleReport.hidingPhase=static_cast<unsigned>(bundleHiding.phase);bundleReport.hidingAttempts=bundleHiding.totalAttempts;
    bundleReport.hidingTimeouts=bundleHiding.timeouts;bundleReport.hidingRecoveries=bundleHiding.recoveries;bundleReport.hidingRefusals=bundleHidingRefused;
    bundleReport.hidingCounselor=bundleHiding.counselor;bundleReport.hidingSpot=bundleHiding.spot;bundleReport.hidingComponent=bundleHiding.component;
    bundleReport.deathConfirmedAt=bundleDeathEnd.confirmedAt;bundleReport.deathEndDue=bundleDeathEnd.due;
    bundleReport.defeatedOutroFlags=bundleDefeatedOutroFlags;bundleReport.defeatedOutroFinished=bundleDefeatedOutroCursor;
    bundleReport.endingRepairFailure=bundleEnding.failure;bundleReport.endingRepairCalls=bundleEnding.calls;
    bundleReport.sweaterReadDeferrals=bundleSweaterPolicy.readDeferrals;
    bundleReport.sweaterFailure=bundleSweaterPolicy.lastFailure;
    bundleReport.sweaterHudAttempts=bundleSweaterPolicy.hudAttempts;bundleReport.sweaterHudReturns=bundleSweaterPolicy.hudReturns;
    bundleReport.sweaterHudQueuedAt=bundleSweaterPolicy.hudQueuedAt;
    bundleReport.sweaterHudFirstServiceAt=bundleSweaterPolicy.hudFirstServiceAt;
    bundleReport.sweaterHudProofFailure=bundleSweaterPolicy.hudProofFailure;
    bundleReport.runWrites=bundleRun.writes;bundleReport.runRefusals=bundleRun.refused;
    bundleReport.footBurstStage=static_cast<unsigned>(bundleFootBurst.stage);bundleReport.footBurstMask=bundleFootBurst.mask;
    bundleReport.footBurstStarts=bundleFootBurst.starts;bundleReport.footBurstRestores=bundleFootBurst.restores;
    bundleReport.footBurstRefused=bundleFootBurst.refused;bundleReport.footBurstUntil=bundleFootBurst.until;bundleReport.footBurstReadyAt=bundleFootBurst.readyAt;
    bundleReport.convergenceFearWrites=bundleConvergenceFearWrites;bundleReport.convergenceFearRefused=bundleConvergenceFearRefused;
    bundleReport.cabinBoundsStage=static_cast<unsigned>(bundleCabins.cache.stage);
    bundleReport.cabinBoundsFailure=bundleCabins.cache.failure;bundleReport.cabinBoundsCount=bundleCabins.cache.count;
    bundleReport.cabinBoundsCaptured=bundleCabins.cache.captured;bundleReport.cabinBoundsCalls=bundleCabins.cache.calls;
    bundleReport.sweaterPickupStage=bundleSweaterPickup.stage;bundleReport.sweaterPickupInstalled=bundleSweaterPickup.installed;
    bundleReport.sweaterPickupCalls=bundleSweaterPickup.accepted;bundleReport.sweaterPickupFailure=bundleSweaterPickup.lastFailure;
    bundleReport.sweaterPickupRefused=bundleSweaterPickup.refused;
    bundleReport.sweaterPickupBasePassed=bundleSweaterPickup.basePassed;bundleReport.sweaterPickupBaseDenied=bundleSweaterPickup.baseDenied;
    bundleReport.sweaterPickupInteractor=bundleSweaterPickup.lastInteractor;
    bundleReport.sweaterArmPending=bundleSweaterPolicy.armPending?1u:0u;
    bundleReport.sweaterArmAttempts=bundleSweaterPolicy.armAttempts;bundleReport.sweaterArms=bundleSweaterPolicy.arms;
    bundleReport.sweaterArmDeferrals=bundleSweaterPolicy.armDeferrals;bundleReport.sweaterArmRejections=bundleSweaterPolicy.armRejections;
    bundleReport.sweaterArmFailure=static_cast<unsigned>(bundleSweaterPolicy.armFailure);
    bundleReport.rbPhase=static_cast<unsigned>(bundleRBEmote.policy.phase);bundleReport.rbShows=bundleRBEmote.policy.shows;
    bundleReport.rbHides=bundleRBEmote.policy.hides;bundleReport.rbRefused=bundleRBEmote.policy.refused;bundleReport.rbAbandoned=bundleRBEmote.policy.abandoned;
    bundleReport.rbCapturedEdges=bundleRBEmote.edges.edges;bundleReport.rbBlockedEdges=bundleRBEmote.edges.blocked;bundleReport.rbExpiredEdges=bundleRBEmote.edges.expired;
    const auto& rbInput=bundleRBEmote.InputStatus();
    bundleReport.rbProviderReady=rbInput.providerReady?1u:0u;bundleReport.rbPolls=rbInput.polls;
    bundleReport.rbForegroundPolls=rbInput.foregroundPolls;bundleReport.rbConnectedPolls=rbInput.connectedPolls;bundleReport.rbHeldPolls=rbInput.presses;
    bundleReport.soundBlipAttempts=bundleSoundBlips.report.attempts;bundleReport.soundBlipApplied=bundleSoundBlips.report.applied;
    bundleReport.soundBlipRestores=bundleSoundBlips.report.restores;bundleReport.soundBlipRestored=bundleSoundBlips.report.restored;
    bundleReport.soundBlipRefused=bundleSoundBlips.report.refused;bundleReport.soundBlipLost=bundleSoundBlips.report.lost;
    bundleReport.soundEmitterReady=bundleSoundHookEnabled?1u:0u;bundleReport.soundEmitterSuppressed=bundleSoundSuppressed;
#if CFG_H276_SOUND_EMITTER
    bundleReport.soundInstall=bundleSoundInstall;bundleReport.soundEntries=bundleSoundEntries;bundleReport.soundHostedCalls=bundleSoundHostedCalls;
    bundleReport.soundGate=bundleSoundGate;bundleReport.soundMh=bundleSoundMh;bundleReport.soundLastKiller=bundleSoundLastKiller;
#endif
#if CFG_H277_HELPER_SPAWN_SAFE
    bundleReport.vetoCount=BundleAccess::h277VetoCount_;bundleReport.vetoTolerated=BundleAccess::h277VetoTolerated_;bundleReport.vetoStage=BundleAccess::h277VetoStage_;
    bundleReport.vetoSerializing=BundleAccess::h277VetoSerializing_;bundleReport.vetoCollecting=BundleAccess::h277VetoCollecting_;
    bundleReport.vetoScoped=BundleAccess::h277VetoScoped_;bundleReport.vetoFunction=BundleAccess::h277VetoFunction_;
#endif
    bundleReport.trapPriorityHookReady=bundleTrapHookEnabled?1u:0u;bundleReport.trapPriorityEvents=bundleTrapEvents;
    bundleReport.trapPriorityAccepted=bundleTrapAccepted;bundleReport.trapPriorityRefused=bundleTrapRefused;
    bundleReport.trapPriorityActive=bundleTrapPriority.Active(now)?1u:0u;bundleReport.trapPriorityMaxReads=bundleTrapMaxReads;
    bundleReport.difficulty=bundleConfiguration.difficulty;bundleReport.difficultyHookReady=bundleDifficultyHookEnabled?1u:0u;
    bundleReport.difficultyScaled=bundleDifficultyScaled;bundleReport.difficultyMaxReads=bundleDifficultyMaxReads;
    bundleReport.doorDecisionPhase=bundleDoorRecovery.phase;
    bundleReport.spectatorCycleEvents=bundleSpectatorCycleEvents;
    bundleReport.spectatorNativeCycles=bundleSpectatorNativeCycles;
    bundleReport.spectatorNativeEntries=static_cast<unsigned>(InterlockedCompareExchange(&bundleSpectatorNativeEntries,0,0));
    bundleReport.spectatorCycleRejection=bundleSpectatorCycleRejection;
    bundleReport.spectatorCycleDirection=bundleSpectatorCycleDirection;bundleReport.spectatorCycleController=bundleSpectatorCycleController;
    bundleReport.spectatorStatusFunctionHint=bundleSpectatorStatusFunction;bundleReport.spectatorStatusWidgetHint=bundleSpectatorStatusWidgetHint;
    bundleReport.botNameAttempts=bundleBotNames.attempts;bundleReport.botNameConfirmed=bundleBotNames.confirmed;
    bundleReport.botNameSkipped=bundleBotNames.skipped;bundleReport.botNameFailure=bundleBotNames.failure;
    bundleReport.f3Polls=bundleF3Polls;bundleReport.f3Foreground=bundleF3Foreground;
    bundleReport.f3DownSamples=bundleF3DownSamples;bundleReport.f3ShortSamples=bundleF3ShortSamples;
    bundleReport.carPositionSamples=bundleCarPursuit.positionSamples;
    bundleReport.roadStage=static_cast<unsigned>(bundleRoads.stage);bundleReport.roadFailure=bundleRoads.failure;
    bundleReport.roadTraceState=bundleRoadTrace.state;bundleReport.roadTraceSegments=bundleRoadTrace.segments;
    bundleReport.roadTraceNearest=bundleRoadTrace.nearest;bundleReport.roadTraceLateral=bundleRoadTrace.lateral;
    bundleReport.roadPoints=bundleRoads.graph.count;bundleReport.roadSlices=bundleRoads.slices;bundleReport.roadLastReads=bundleRoads.lastReads;
    bundleReport.carVelocityFallbacks=bundleCarPursuit.velocityFallbacks;
    bundleReport.carStationary=bundleCarPursuit.stationary;bundleReport.carHeld=bundleCarPursuit.held;
    bundleReport.carBusy=bundleCarPursuit.busy;bundleReport.carMoveAttempts=bundleCarPursuit.moves;
    bundleReport.loadoutRequested=bundleLoadoutResource.requested?1u:0u;
    bundleReport.loadoutReady=bundleLoadoutResource.ready?1u:0u;
    bundleReport.loadoutFailed=bundleLoadoutResource.failed?1u:0u;
    if(a.knifeFailure!=UINT32_MAX)bundleReport.knifeFailure=a.knifeFailure;
    bundleReport.knifePresses=bundleKnife.presses;bundleReport.knifeReleases=bundleKnife.releases;
    bundleReport.knifeLaunches=bundleKnife.launches;bundleReport.knifeFinalizes=bundleKnife.finalizes;
    bundleReport.knifeHeldStops=bundleKnifeHeldStops;bundleReport.knifeHeldStopRefused=bundleKnifeHeldRefused;
    bundleReport.knifeStopMaxReads=bundleKnifeStopMaxReads;bundleReport.emoteMovementStops=bundleEmoteStops;bundleReport.emoteBrainResumes=bundleEmoteResumes;
    bundleReport.knifeNotifyEntries=bundleKnifeNotifyEntries;bundleReport.knifeNotifyCorrections=bundleKnifeNotifyCorrections;
    bundleReport.knifeNotifyRefused=bundleKnifeNotifyRefused;bundleReport.knifeNotifyStops=bundleKnifeNotifyStops;
    bundleReport.knifeNotifyAdmissionRefused=bundleKnifeNotifyAdmissionRefused;bundleReport.knifeNotifyMaxReads=bundleKnifeNotifyMaxReads;
    bundleReport.knifeAdapterStage=bundleKnifeReleaseDiagnostics.stage;bundleReport.knifeAdapterFailure=bundleKnifeReleaseDiagnostics.failure;
    bundleReport.knifeAdapterReads=bundleKnifeReleaseDiagnostics.reads;bundleReport.knifeAdapterCalls=bundleKnifeReleaseDiagnostics.calls;
    bundleReport.knifeAdapterEntryReads=bundleKnifeReleaseDiagnostics.entryReads;
    bundleReport.knifeRefusals=bundleKnife.refusals;bundleReport.knifeTimeouts=bundleKnife.timeouts;
    for(unsigned i=0;i<10;++i)bundleReport.knifeAbortStages[i]=bundleKnife.abortStages[i];
    bundleReport.knifeAbortOwnerObserve=bundleKnife.abortOwnerObserve;bundleReport.knifeFirstAbortStage=static_cast<unsigned>(bundleKnife.firstAbortStage);
    bundleReport.knifeFirstAbortFailure=bundleKnife.firstAbortFailure;bundleReport.knifeFirstAbortReads=bundleKnife.firstAbortReads;
    bundleReport.knifeFirstAbortCalls=bundleKnife.firstAbortCalls;bundleReport.knifeFirstAbortRecorded=bundleKnife.firstAbortRecorded?1u:0u;
    bundleReport.knifeFlightSamples=bundleKnife.flightSamples;bundleReport.knifeFlightsMoved=bundleKnife.flightsMoved;
    bundleReport.knifeFlightsStationary=bundleKnife.flightsStationary;bundleReport.knifeFlightUnavailable=bundleKnife.flightUnavailable;
    bundleReport.knifeFlightPosition={bundleKnife.lastFlight.x,bundleKnife.lastFlight.y,bundleKnife.lastFlight.z};
    bundleReport.knifeFlightSpeed=bundleKnife.lastFlight.speed;bundleReport.knifeFlightUpdated=bundleKnife.lastFlight.updated;
    bundleReport.knifeFlightActive=bundleKnife.lastFlight.active?1u:0u;
    bundleReport.travelAcquires=bundleTravelAcquires;bundleReport.travelMoves=bundleTravelMoves;
    bundleReport.travelResumes=bundleTravelResumes;bundleReport.travelLost=bundleTravelLost;
    bundleReport.convergenceAxeStage=bundleConvergenceAxes.stage;bundleReport.convergenceAxeScanned=bundleConvergenceAxes.scanned;
    bundleReport.convergenceAxeAttempts=bundleConvergenceAxes.attempts;bundleReport.convergenceAxeGrants=bundleConvergenceAxes.grants;
    bundleReport.convergenceAxeRefused=bundleConvergenceAxes.refused;
    BundleObjectiveReport();
    BundleEmoteReport();
    MemoryBarrier();InterlockedIncrement(reinterpret_cast<volatile LONG*>(&bundleReport.generation));
}

#include "HostedRoutineRuntime.hpp"
#if CFG_H216_HOSTED_SPECTATOR && defined(CFG_H1_JASON)
#include "HostedSpectatorObservation.hpp"
#endif

