// H1 hosted-lobby runtime glue. Included inside the observer's anonymous namespace after the
// native Jason controller Tick runtime, before Observe(). Owner-thread only.
//
//   CFG_H1_OBSERVE : read-only sampler -> cfgh1::Report (exported CfgHostedH1Report). No native call, no write.
//   CFG_H1_JASON   : (separate flag, operator arm-file gated) donor takeover of the stock-selected human Jason (local OR remote),
//                    then hand-off to the existing native-tick Jason driver through the RetainedWorld dispatcher.
//   CFG_H1_FILL    : reserved; not built (static order BotFill-before-3DC930 not proven; see docs).
#ifdef CFG_H1_OBSERVE
#ifdef CFG_H1_JASON
#include "../Core/HostedRolePreference.hpp"
#include "../Core/HostedOfflineEpoch.hpp"
#endif
cfgh1::Report hostedH1Report;
#include "HostedStockBaseline.hpp"
struct HostedH1BuildFlags { HostedH1BuildFlags() noexcept { hostedH1Report.buildFlags=cfgh1::FlagObserve
#ifdef CFG_H1_JASON
    |cfgh1::FlagJason
#endif
#if defined(CFG_H1_FILL) || CFG_H20_FILL
    |cfgh1::FlagFill
#endif
    ; } } hostedH1BuildFlags;
class HostedH1Runtime {
public:
    std::uint64_t nextAt=0,armCheckAt=0;
    bool armed=false;
    unsigned retryCount=0,classStage=0,classWarmCalls=0;std::uint64_t retryAt=0;
    std::uintptr_t classReflection=0,classConstructor=0;cfghostedpacket::Diagnostic packetDiagnostic{};
    bool manualArm=false,f8Down=false;unsigned f8Presses=0;
    std::uintptr_t stockFillWorld=0;bool stockFillWanted=false;unsigned stockFillDeferred=0;
#if CFG_H22_HOTKEYS
    bool disarmed=false,f9Down=false,f3Window=false,f3BrainWindow=false,f3BrainServiced=false,f3Swapped=false,f3Serviced=false,f3Quarantined=false,takeoverRetrySafe=false;
    bool f3Automatic=false,autoFillStopped=false,fillHasFrame=false;unsigned autoFillQueued=0,autoFillAdded=0,autoFillRefused=0;unsigned autoFillRetries=0;std::uint64_t fillStableWorld=0,fillStableAt=0,autoFillNext=0;
    unsigned f3Pending=0,f3Rechecks=0;std::uint64_t keyNext=0,f3At=0,f3World=0,f3ActorHint=0,f3Next=0;
    unsigned f3BrainTicketIndex=8;
    cfgobserver::Plan f3SavedPlan{};cfgprimary::HostedBinding f3SavedBinding{};
#endif
    cfghostedoptions::Match options;
    cfgh1::Stability stable;
    cfghosted::takeover35::Frame frame{},freshFrame{};
    bool frameValid=false;
    std::uint64_t frameAt=0;
    // Lease request: the takeover may only run inside a verified returned ReceiveTick of the host's pawn.
    bool takeoverDue=false,takeoverTried=false,planSwapped=false,kept=false,adopted=false;
    std::uintptr_t takeoverWorld=0,adoptedWorld=0;
    bool adoptionPending=false;unsigned adoptionInitAttempts=0;
    cfgobserver::Plan adoptionPlan{};cfgprimary::HostedBinding adoptionBinding{};
    cfghosted::adoption::Receipt pendingReceipt{};
    cfgobserver::Plan savedPlan{};
    std::uintptr_t originalLocalState=0;
    bool presentationPrimaryAttempted=false,presentationPrimaryComplete=false,presentationExtraPending=false,presentationExtraAttempted=false,presentationExtraComplete=false;
    unsigned presentationNotAttemptedReason=0,presentationNotAttemptedWindows=0;
    unsigned f1InputEdges=0,f1RequestAccepted=0,f1RequestRefused=0;
    bool rolePreferenceApplied=false;
    unsigned rolePreferenceAttempts=0,rolePreferenceRefused=0,rolePreferenceRefusal=0,rolePreferenceRemoteCount=0;
    unsigned rolePreferenceInstallAttempts=0,rolePreferenceInstallRefused=0;
    std::uint8_t rolePreferenceHostBefore=0,rolePreferenceHostAfter=0,rolePreferenceWinnerLocal=0;
    unsigned h245PinMask=0,h245WritesApplied=0,h245HostWon=0,h245Refusal=0; // H2.45 host-as-Jason lottery counters
    unsigned h248BridgeCalls=0,h248BridgeLastGate=0;std::uint64_t h248BridgeInstallAt=0,h248BridgeFirstCallAt=0; // H2.48d bridge entry evidence
    unsigned h250ViewFailure=0; // H2.50c: LocalViewReady failure*10+resolve step
    bool extraRequestAccepted=false,extraRequestTimerStarted=false;std::uint64_t extraRequestMatchAt=0,extraRequestNextAttempt=0;
    unsigned presentationRefusal=0,presentationCopyRefusal=0,presentationCopyBundleNativeStage=0;std::uintptr_t presentationSkinBefore=0,presentationSkin=0;cfgending::Packet presentationWeaponBefore{},presentationWeapon{};
    std::uintptr_t primaryPresentationSkinBefore=0,primaryPresentationSkin=0,extraPresentationSkinBefore=0,extraPresentationSkin=0;
    cfgending::Packet primaryPresentationWeaponBefore{},primaryPresentationWeapon{},extraPresentationWeaponBefore{},extraPresentationWeapon{};
    cfgh1::OutroEvidence outro{};
    unsigned ownerChecks=0,ownerRefusals=0,attempts=0,armRefusal=0,adoptionRefusal=0;
    unsigned adoptionStage5Clause=0,adoptionStage5Reads=0,adoptionStage5Cap=0;
    std::uintptr_t adoptionStage5PawnClass=0,adoptionStage5KillerClass=0,adoptionStage5PathClass=0;
    cfgprobe::lifetime::SlotObservation adoptionStage5PawnSlot{},adoptionStage5ControllerSlot{},adoptionStage5PathSlot{};
    std::uint32_t outcome=0,stage=0,reason=0,rollbackVerified=0,effects=0;
    cfgspawn::ResourceProgress adoptionResourceProgress{};
    std::uintptr_t aiController=0;std::uint64_t takeoverTick=0;
    unsigned primaryPresentationAttempts=0;std::uint64_t primaryPresentationNext=0; // H2.43 post-adoption copy
} hostedH1;
#ifdef CFG_H1_JASON
void HostedF1CaptureRequest(bool accepted)noexcept{
    ++hostedH1.f1InputEdges;
    if(accepted)++hostedH1.f1RequestAccepted;else ++hostedH1.f1RequestRefused;
}
#define CFG_H1_ROLE_SELECTION 0
#if CFG_H1_ROLE_SELECTION || CFG_H245_HOST_JASON
#include "HostedRoleSelectionRuntime.hpp"
#endif
bool HostedJasonReservationArmed()noexcept{return hostedH1.armed;}
bool HostedJasonReservationAdopted()noexcept{return hostedH1.adopted;}
#endif
#ifdef CFG_H1_JASON
bool HostedH1ArmFilePresent()noexcept;
#endif
#include "HostedFillRuntime.hpp"
#include "HostedLobbyRuntime.hpp"
#ifdef CFG_H1_JASON
// The operator arms the native takeover by creating this file beside the primary DLL. Without it
// the build is observation-only: the takeover transaction never starts (no native effect).
inline bool HostedH1ArmFilePresent() noexcept {
    wchar_t path[MAX_PATH]{};HMODULE self=nullptr;
    if(!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
        reinterpret_cast<LPCWSTR>(&hostedH1Report),&self)||!self)return false;
    const auto length=GetModuleFileNameW(self,path,MAX_PATH);
    if(!length||length>=MAX_PATH-32)return false;
    auto slash=std::wcsrchr(path,L'\\');if(!slash)return false;
    if(wcscpy_s(slash+1,static_cast<rsize_t>(MAX_PATH-(slash+1-path)),L"H1_ARM_JASON.txt"))return false;
    return GetFileAttributesW(path)!=INVALID_FILE_ATTRIBUTES;
}
// ---- adoption: hand the verified-adopted Jason to the existing native-tick driver (mailbox #349) --------------------
// Fills ONLY what the native driver reads (gameState, killer/AI/path classes, resolved Jason template, manager and
// counselor class), proves the adopted pair with LinkedAdoptedKiller, then publishes bundleJason + the receipt.
// bundleStarted stays FALSE; receipt-owned HostedRoutineActive admits the shared AI lanes.
void HostedUnpublishAdoption()noexcept{
    if(hostedAdoptionActive)++hostedDriverTrace.retirements;
    RetireHostedRoutine();hostedDriverStartupOwed=false;hostedAdoptionActive=false;hostedAdoptionReceipt={};bundleJason={};bundleSchedule={};
}
__declspec(noinline) bool HostedPrepareAdoption(const cfghosted::adoption::Receipt& receipt,const cfgobserver::Plan& target,
    const cfgprimary::HostedBinding& binding)noexcept{
    auto fail=[&](unsigned stage){hostedTx.adoptStage=stage;hostedH1.adoptionRefusal=stage;return false;};
    hostedH1.adoptionStage5Clause=hostedH1.adoptionStage5Reads=hostedH1.adoptionStage5Cap=0;
    hostedH1.adoptionStage5PawnClass=hostedH1.adoptionStage5KillerClass=hostedH1.adoptionStage5PathClass=0;
    hostedH1.adoptionStage5PawnSlot=hostedH1.adoptionStage5ControllerSlot=hostedH1.adoptionStage5PathSlot={};
    Reader r;r.cap=6144;cfgobserver::Plan fresh{};std::uintptr_t counselor=0,pawnClass=0,path=0;
    if(!binding.Matches(target)||!cfgprimary::RetainedWorld(r,target,fresh,binding))return fail(1);
    if(fresh.objects[2].object!=fresh.controller||fresh.objects[3].object!=fresh.pawn||
       !cfgjason::SameSlot(r,fresh.objectArray,fresh.objects[2])||
       !cfgjason::SameSlot(r,fresh.objectArray,fresh.objects[3])||
       !cfgobserver::Same(r,fresh.controller+cfgprobe::ControllerPawn,fresh.pawn)||
       !cfgobserver::Same(r,fresh.pawn+cfgescape::disk::Controller,fresh.controller))return fail(1);
    spawnAssets.gameState=hostedTx.gameState;spawnAssets.killerBase=hostedTx.killerBase;
    if(!r.Read(target.module+cfgprobe::NativeClasses[cfgprobe::Counselor].reflectionCacheRva,counselor)||!counselor||
       !cfgspawn::CaptureHostedRuntimeResources(r,fresh.module,fresh.objectArray,spawnAssets.resources,&hostedH1.adoptionResourceProgress)||
       spawnAssets.resources.classMask!=255)return fail(2);
    for(unsigned i=0;i<4;++i){std::uintptr_t a=0,b=0;
        if(!r.Read(target.module+cfgjason::disk::Classes[i].reflection,a)||!r.Read(target.module+cfgjason::disk::Classes[i].constructor,b)||!a||a!=b||
           !cfgspawn::ObjectSlot(r,fresh.objectArray,a,spawnAssets.classSlots[i]))return fail(3);
        spawnAssets.classes[i]=a;
        // Reflected UFunction caches are published only if already resident (no lazy factory allocation here).
        std::uintptr_t function=0;spawnAssets.functions[i]=0;
        if(r.Read(target.module+cfgjason::disk::Functions[i].cache,function)&&function&&
           cfgspawn::ObjectSlot(r,fresh.objectArray,function,spawnAssets.functionSlots[i])){spawnAssets.functions[i]=function;}
    }
    if(!cfgjason::SameSlot(r,fresh.objectArray,receipt.pawn)||!receipt.pawn.HasAssignedSerial()||
       !cfgjason::SameSlot(r,fresh.objectArray,receipt.controller)||!receipt.controller.HasAssignedSerial()||
       !r.Read(receipt.pawn.object+cfgprobe::ObjectClass,pawnClass)||
       !cfgprimary::ObserveResolvedTemplate(r,fresh.objectArray,pawnClass,spawnAssets.killerBase,
           spawnAssets.resources.classes[cfgspawn::BaseAI],spawnAssets.killerTemplate))return fail(4);
    cfgjason::NewActorEvidence evidence{};evidence.pawn=receipt.pawn;evidence.killerController=receipt.controller;
    hostedH1.adoptionStage5PawnClass=spawnAssets.killerTemplate.pawnClass;
    hostedH1.adoptionStage5KillerClass=spawnAssets.resources.classes[cfgspawn::KillerAI];
    hostedH1.adoptionStage5PathClass=spawnAssets.classes[3];
    hostedH1.adoptionStage5PawnSlot=receipt.pawn;hostedH1.adoptionStage5ControllerSlot=receipt.controller;
    const unsigned stage5Begin=r.reads;
    if(!r.Read(receipt.controller.object+cfgjason::disk::PathFollowing,path)){hostedH1.adoptionStage5Clause=19;hostedH1.adoptionStage5Reads=r.reads-stage5Begin;hostedH1.adoptionStage5Cap=r.cap;return fail(5);}
    if(!cfgspawn::ObjectSlot(r,fresh.objectArray,path,evidence.path)){
        std::int32_t rawIndex=-1;if(cfgprobe::ObjectPointer(path))r.Read(path+cfgprobe::lifetime::ObjectIndexOffset,rawIndex);
        cfgclosedtrace::Mark("h225-adopt-path-index-refused",path,static_cast<std::uintptr_t>(static_cast<std::uint32_t>(rawIndex)));
        cfgclosedtrace::Mark("h225-adopt-saved-registry",target.objectArray.data,static_cast<std::uintptr_t>(target.objectArray.count));
        cfgclosedtrace::Mark("h225-adopt-fresh-registry",fresh.objectArray.data,static_cast<std::uintptr_t>(fresh.objectArray.count));
        hostedH1.adoptionStage5Clause=20;hostedH1.adoptionStage5Reads=r.reads-stage5Begin;hostedH1.adoptionStage5Cap=r.cap;return fail(5);
    }
    hostedH1.adoptionStage5Clause=hostedH1.adoptionStage5Reads=0;hostedH1.adoptionStage5Cap=r.cap;
    hostedH1.adoptionStage5PathSlot=evidence.path;
    const auto stage5=cfgjason::LinkedAdoptedKillerProof(r,fresh.objectArray,evidence,receipt,fresh,fresh.pawn,
        spawnAssets.killerTemplate.pawnClass,spawnAssets.killerTemplate.defaults,spawnAssets.killerBase,
        spawnAssets.resources.classes[cfgspawn::KillerAI],spawnAssets.classes[3],target.controller);
    hostedH1.adoptionStage5Clause=stage5.firstFailure;hostedH1.adoptionStage5Reads=(r.reads-stage5Begin);
    if(!stage5)return fail(5);
    if(!r.Read(target.module+cfghunter::disk::ManagerReflection,bundleManagerClass)||!bundleManagerClass||
       !cfgobserver::Same(r,target.module+cfghunter::disk::ManagerConstructor,bundleManagerClass)||
       !cfgspawn::ObjectSlot(r,fresh.objectArray,bundleManagerClass,bundleManagerClassSlot))return fail(6);
    bundleCounselorClass=counselor;
    if(!cfgspawn::ObjectSlot(r,fresh.objectArray,spawnAssets.gameState,spawnAssets.primary.stateSlot)||
       !cfgspawn::ObjectSlot(r,fresh.objectArray,target.mode,spawnAssets.primary.modeSlot))return fail(7);
    bundleDonorRoster={};bundleDonorPrimary={};bundleDonorAdditional={};bundleDonorHeldPrimary={};bundleDonorHeldAdditional={};
    bundleDonorConvergence={};bundleDonorPostGrab={};bundleDonorSurveyBackoff={};
    bundleJason=evidence;
    bundleConfiguration.difficulty=hostedH1.options.values.Difficulty();
    if(bundleConfiguration.difficulty!=1&&!PrepareBundleDifficultyHook(target.module))return fail(9);
    bundleNativeJasonDonorService=&ServiceDonorNativeHunting;
    if(!PrepareNativeJasonControllerTick(target.module))return fail(8);
    hostedTx.adoptStage=100;hostedTx.receipt=receipt;hostedTx.evidence=evidence;
    return true;
}
bool HostedActivateAdoption(const cfghosted::adoption::Receipt& receipt)noexcept{
    cfghostedend::ClearForActivation(hostedEndFrozen,hostedEndWatchAt);
    hostedAdoptionReceipt=receipt;hostedAdoptionActive=true;
    if(!InitializeHostedRoutine(GetTickCount64())){
        RetireHostedRoutine();hostedAdoptionActive=false;hostedAdoptionReceipt={};
        hostedH1.adoptionRefusal=10;return false;
    }
    ++hostedDriverTrace.publications;hostedDriverTrace.stage=1;
    hostedDriverTrace.world=receipt.world;hostedDriverTrace.controller=receipt.controller.object;hostedDriverStartupOwed=false;
    hostedH1.adoptionRefusal=0;return true;
}
bool HostedPublishAdoption(const cfghosted::adoption::Receipt& receipt)noexcept{
    return HostedPrepareAdoption(receipt,plan,cfgprimary::CurrentHostedBinding())&&HostedActivateAdoption(receipt);
}
bool HostedPresentCurrentKiller(bool extra)noexcept{
    auto& h=hostedH1;
    bool& attempted=extra?h.presentationExtraAttempted:h.presentationPrimaryAttempted;
    if(attempted)return false;
    attempted=true; // latch before any native setter dispatch
    if(!HostedRoutineActive()){h.presentationRefusal=1;return false;}
    std::uintptr_t targetState=extra?bundleAdditional.journal.killerState:0;
    if(!extra){Reader stateReader;stateReader.cap=64;if(!bundleJason.pawn.object||
        !stateReader.Read(bundleJason.pawn.object+cfgescape::disk::PlayerState,targetState))targetState=0;}
    const auto sourceController=h.freshFrame.host.local[2].object;
    const auto sourceState=h.originalLocalState;
    const auto selectedClass=extra?bundleAdditional.selected.pawnClass:spawnAssets.killerTemplate.pawnClass;
    if(!targetState||!sourceController||!sourceState||!selectedClass){h.presentationRefusal=2;return false;}
    BundleAccess access(1024);
    std::uintptr_t skin=0,skinBefore=0;cfgending::Packet weapon{},weaponBefore{};
    const bool copied=access.HostedCopyKillerPresentation(targetState,sourceState,sourceController,selectedClass,extra,false,skin,weapon,skinBefore,weaponBefore);
    h.presentationCopyRefusal=access.HostedPresentationCopyRefusal();
    h.presentationCopyBundleNativeStage=access.HostedPresentationCopyBundleNativeStage();
    h.presentationSkinBefore=skinBefore;h.presentationWeaponBefore=weaponBefore;
    if(extra){h.extraPresentationSkinBefore=skinBefore;h.extraPresentationWeaponBefore=weaponBefore;}
    else {h.primaryPresentationSkinBefore=skinBefore;h.primaryPresentationWeaponBefore=weaponBefore;}
    if(!copied){
        h.presentationRefusal=3;return false;
    }
    h.presentationSkin=skin;h.presentationWeapon=weapon;
    if(extra){
        h.extraPresentationSkinBefore=skinBefore;h.extraPresentationSkin=skin;
        h.extraPresentationWeaponBefore=weaponBefore;h.extraPresentationWeapon=weapon;
        h.presentationExtraPending=false;h.presentationExtraComplete=true;
    }
    else {
        h.primaryPresentationSkinBefore=skinBefore;h.primaryPresentationSkin=skin;
        h.primaryPresentationWeaponBefore=weaponBefore;h.primaryPresentationWeapon=weapon;
        h.presentationPrimaryComplete=true;
    }
    h.presentationRefusal=0;return true;
}
// Donor OfflineSetup.cpp:8828/10331: present the newly spawned killer PlayerState
// from the host's selected PlayerState before the takeover transaction possesses it.
bool HostedPresentBirthKiller(BundleAccess& access,const cfghostedtx::Birth& birth)noexcept{
#if CFG_H243_PRESENTATION
    // H2.43: cosmetic best effort, result ignored by PrepareAiWithBestEffortPresentation. The copy moved after the adoption
    // receipt (HostedH1ServicePrimaryPresentation); nothing retained or called here, so the receipt cannot be disturbed.
    (void)access;(void)birth;hostedH1.presentationRefusal=5;return false;
}
bool HostedPresentBirthKillerH241(BundleAccess& access,const cfghostedtx::Birth& birth)noexcept{
#endif
    auto& h=hostedH1;if(h.presentationPrimaryAttempted)return false;
    h.presentationPrimaryAttempted=true; // one-shot even if native entry becomes ambiguous
    const auto targetState=birth.state,sourceState=h.originalLocalState;
    const auto sourceController=h.freshFrame.host.local[2].object;
    std::uintptr_t selectedClass=0;
    if(!targetState||!sourceState||!sourceController){h.presentationRefusal=2;return false;}
    if(!access.HostedSelectedKillerClass(selectedClass)){h.presentationRefusal=4;return false;}
    std::uintptr_t skin=0,skinBefore=0;cfgending::Packet weapon{},weaponBefore{};
    const bool copied=access.HostedCopyKillerPresentation(targetState,sourceState,sourceController,
        selectedClass,false,true,skin,weapon,skinBefore,weaponBefore);
    h.presentationCopyRefusal=access.HostedPresentationCopyRefusal();
    h.presentationCopyBundleNativeStage=access.HostedPresentationCopyBundleNativeStage();
    h.presentationSkinBefore=skinBefore;h.presentationWeaponBefore=weaponBefore;
    h.primaryPresentationSkinBefore=skinBefore;h.primaryPresentationWeaponBefore=weaponBefore;
    if(!copied){h.presentationRefusal=3;return false;}
    h.presentationSkin=skin;h.presentationWeapon=weapon;
    h.primaryPresentationSkin=skin;h.primaryPresentationWeapon=weapon;
    h.presentationPrimaryComplete=true;h.presentationRefusal=0;return true;
}
// Do not swap plan while the old returned-callback lease is alive. Prove the
// restarted local counselor now, stage the exact new plan, and install it only
// after lookup/root scopes are destroyed. Its own next returned tick activates AI.
bool HostedStageLocalAdoption(const cfghosted::adoption::Receipt& receipt)noexcept{
    if(hostedTx.snap.human!=hostedTx.snap.localController)return HostedPublishAdoption(receipt);
    Reader r;r.cap=16384;cfghosted::listen35::Context host;cfgobserver::Plan candidate;cfgprimary::HostedBinding binding;
    const cfghosted::native::Admission admission{plan.module,plan.engine,plan.viewport,plan.world,plan.threadId,true,true};
    std::uintptr_t counselor=0;const auto& klass=cfgprobe::NativeClasses[cfgprobe::Counselor];
    if(!cfghosted::listen35::Capture(r,admission,GetCurrentThreadId(),host)||
       !cfgprimary::BuildHostedLocalPlan(r,host,candidate,binding)||candidate.controller!=plan.controller||candidate.pawn==hostedTx.snap.jason||
       !cfghosted::native::Class(r,plan.module,klass.reflectionCacheRva,klass.constructorCacheRva,counselor)||
       !cfgspawn::ObjectIs(r,candidate.pawn,counselor)||!HostedPrepareAdoption(receipt,candidate,binding))return false;
    auto& s=hostedH1;s.adoptionPlan=candidate;s.adoptionBinding=binding;s.pendingReceipt=receipt;
    s.adoptionPending=true;s.adoptionInitAttempts=0;return true;
}
// Retire (mailbox #349 item 6): stop driving, no further native call. Never touches the live objects.
__declspec(noinline) void HostedH1Retire()noexcept{
    auto& s=hostedH1;const bool hadHosted=s.adopted||s.adoptionPending||HostedAdoptionOwnsDriver();
    cfghostedend::ClearForRetirement(hostedEndFrozen,hostedEndWatchAt);
    HostedUnpublishAdoption();
    if(s.planSwapped){plan=s.savedPlan;s.planSwapped=false;}
    cfgprimary::CurrentHostedBinding().Retire();
    if(hostedClassRestore.pending){++hostedClassRestore.refused;hostedClassRestore.pending=false;}
#if CFG_H22_HOTKEYS
    hostedCounselorBrainQueue={};s.f3BrainTicketIndex=8;s.f3BrainWindow=s.f3BrainServiced=false;
#endif
    hostedViewRepair.pending=false;hostedPreborn={};hostedF1Start={};hostedF1StartTurns=0;hostedF1StartWait=false;
    s.adopted=false;s.kept=false;s.adoptedWorld=0;s.adoptionPending=false;s.pendingReceipt={};hostedTx.Reset();
    s.presentationPrimaryAttempted=s.presentationPrimaryComplete=s.presentationExtraPending=s.presentationExtraAttempted=s.presentationExtraComplete=false;
    s.presentationNotAttemptedReason=s.presentationNotAttemptedWindows=0;s.f1InputEdges=s.f1RequestAccepted=s.f1RequestRefused=0;
    s.rolePreferenceApplied=false;s.rolePreferenceAttempts=s.rolePreferenceRefused=s.rolePreferenceRefusal=s.rolePreferenceRemoteCount=0;
    s.rolePreferenceHostBefore=s.rolePreferenceHostAfter=s.rolePreferenceWinnerLocal=0;
    s.h245WritesApplied=s.h245HostWon=s.h245Refusal=0; // pin mask is a property of the image and persists
    s.extraRequestAccepted=false;s.extraRequestTimerStarted=false;s.extraRequestMatchAt=s.extraRequestNextAttempt=0;
    hostedJasonExtraCountConsumed=false;s.presentationRefusal=s.presentationCopyRefusal=s.presentationCopyBundleNativeStage=0;s.presentationSkinBefore=s.presentationSkin=0;
    s.presentationWeaponBefore={};s.presentationWeapon={};hostedExtraPresentationPending=false;s.primaryPresentationAttempts=0;s.primaryPresentationNext=0;
    s.primaryPresentationSkinBefore=s.primaryPresentationSkin=s.extraPresentationSkinBefore=s.extraPresentationSkin=0;
    s.primaryPresentationWeaponBefore={};s.primaryPresentationWeapon={};s.extraPresentationWeaponBefore={};s.extraPresentationWeapon={};
    // Existing weak-only epoch reset; resident detours/root registries remain.
    // Do not erase an outstanding owned restoration or an active callback lease.
    if(hadHosted&&!bundleRootLease&&BundleEpochRetirable())BundleResetEpoch();
}
bool RetireHostedBeforeOffline(std::uintptr_t world)noexcept{
    auto& s=hostedH1;
    const bool dirty=s.adopted||s.planSwapped||s.adoptionPending||s.manualArm||s.armed||hostedRoutineStarted||
        hostedAdoptionActive||cfgprimary::CurrentHostedBinding().active||hostedClassRestore.pending;
    if(!dirty)return true; // offline-first is byte-for-byte the existing setup route
    if(GetCurrentThreadId()!=observerOwnerThread||bundleRootLease||bundleRootWitness.lease||!BundleEpochRetirable())return false;
    Reader r;r.cap=32;std::uintptr_t engine=0,viewport=0,current=0,driver=1;
    if(!world||!r.Read(plan.module+cfgprobe::EngineGlobalCandidate,engine)||
       !r.Read(engine+cfgprobe::EngineGameViewport,viewport)||!r.Read(viewport+cfgprobe::ViewportWorld,current)||current!=world||
       !r.Read(world+cfgprobe::WorldNetDriver,driver)||driver||
       !cfgobserver::Same(r,viewport+cfgprobe::ViewportWorld,world)||
       !cfgobserver::Same(r,world+cfgprobe::WorldNetDriver,std::uintptr_t{}))return false;
    // Called only by the independently proved native offline frontend. Never
    // retire on the transient empty World observed during an online match.
    const auto incoming=plan;const auto retired=hostedDriverTrace.retirements;
    HostedH1Retire();plan=incoming;
    ++hostedOfflineRetirements;
    s={};hostedTx={};hostedClassRestore={};hostedViewRepair={};hostedPreborn={};
#if CFG_H22_HOTKEYS
    hostedCounselor={};hostedCounselorPacket={};
#endif
    hostedRoutineStarted=false;hostedRoutineRosterAt=0;
    cfgprimary::CurrentHostedBinding().Retire();
    if(hostedDriverTrace.retirements==retired)++hostedDriverTrace.retirements;
    cfgclosedtrace::Mark("hosted-offline-transaction-retired",world,hostedDriverTrace.retirements);
    return true;
}
float HostedDifficultyDamage(std::uintptr_t victim,float incoming,std::uintptr_t instigator,std::uintptr_t causer)noexcept{
    const auto difficulty=hostedH1.options.values.Difficulty();
    if(!HostedAdoptionOwnsDriver()||difficulty==1||!cfgdifficulty::EligibleAmount(incoming)||
       GetCurrentThreadId()!=plan.threadId||!victim||!causer||
       (instigator&&instigator!=hostedAdoptionReceipt.controller.object))return incoming;
    Reader r;r.cap=1536;cfgobserver::Plan fresh{};cfgnames::Text phase;
    cfgnames::Words word{};cfgnames::Observation<Reader> names(r,plan.module);
    const auto& receipt=hostedAdoptionReceipt;
    float killerHealth=0;std::uint8_t role=0;
    if(!cfgprimary::LookupVetoClearForObservation(r,plan.module)||!cfgprimary::RetainedWorld(r,plan,fresh)||
       !cfghosted::adoption::Current(r,fresh.objectArray,receipt,plan)||
       !r.Read(receipt.pawn.object+cfghunter::disk::Health,killerHealth)||!std::isfinite(killerHealth)||killerHealth<=0||
       !r.Read(receipt.pawn.object+cfgknife::disk::Role,role)||role!=3||
       !cfgjason::SameSlot(r,fresh.objectArray,receipt.pawn)||!cfgjason::SameSlot(r,fresh.objectArray,receipt.controller)||
       !cfgobserver::Same(r,receipt.controller.object+cfgprobe::ControllerPawn,receipt.pawn.object)||
       !cfgobserver::Same(r,receipt.pawn.object+cfgescape::disk::Controller,receipt.controller.object)||
       !r.Read(plan.mode+cfgescape::disk::MatchState,word)||!names.Decode(word,phase)||phase.View()!=u"InProgress")return incoming;
    cfgescape::State living{};cfgprobe::lifetime::SlotObservation v{},c{},level{};std::uintptr_t levelType=0,type=0;
    if(!cfgescape::Counselor(r,fresh.objectArray,plan.module,victim,bundleCounselorClass,living)||living!=cfgescape::State::Living||
       !cfgspawn::ObjectSlot(r,fresh.objectArray,victim,v)||!v.HasAssignedSerial()||
       !cfgspawn::ObjectSlot(r,fresh.objectArray,causer,c)||!c.HasAssignedSerial()||
       !cfgkill::ActorWorldCurrent(r,fresh.objectArray,plan.module,victim,plan.world,level,levelType)||
       !cfgkill::ActorWorldCurrent(r,fresh.objectArray,plan.module,causer,plan.world,level,levelType))return incoming;
    auto source=cfgdifficulty::Source::None;
    if(causer==receipt.pawn.object)source=cfgdifficulty::Source::Jason;
    else if(cfghosted::native::Class(r,plan.module,cfgconvergence::disk::SCWeaponReflection,cfgconvergence::disk::SCWeaponConstructor,type)&&
            cfgkill::OwnedHeldWeapon(r,fresh.objectArray,receipt.pawn.object,causer,type))source=cfgdifficulty::Source::Weapon;
    else if(cfghosted::native::Class(r,plan.module,cfgknife::disk::ThrowableReflection,cfgknife::disk::ThrowableConstructor,type)&&
            cfgspawn::ObjectIs(r,causer,type)&&cfgobserver::Same(r,causer+cfgknife::disk::ThrownBy,receipt.pawn.object))source=cfgdifficulty::Source::Projectile;
    if(source==cfgdifficulty::Source::None||!cfgjason::SameSlot(r,fresh.objectArray,v)||!cfgjason::SameSlot(r,fresh.objectArray,c)||
       !cfgescape::Same(r,plan.mode+cfgescape::disk::MatchState,word)||!cfgprimary::RetainedWorld(r,plan,fresh)||
       !cfghosted::adoption::Current(r,fresh.objectArray,receipt,plan))return incoming;
    const float scaled=cfgdifficulty::Scale(incoming,difficulty,source);
    (void)cfgh1::RecordHardDamage(hostedH1Report,difficulty,static_cast<std::uint32_t>(source),incoming,scaled);
    return scaled;
}
struct HostedOps {
    BundleAccess& a;
    bool HasFullyTraveled(std::uintptr_t ps){return a.HostedHasFullyTraveled(ps);}
    bool ResolveClasses(){return a.HostedResolveClasses();}
    bool SpawnAi(cfghostedtx::Birth& b){return a.HostedSpawnAi(b);}
    bool PathReady(const cfghostedtx::Birth& b){return a.HostedPathReady(b);}
    bool ReadBots(std::int32_t& v){return a.HostedReadBots(v);}
    bool AccountBot(std::int32_t before){return a.HostedAccountBot(before);}
    bool UnaccountBot(std::int32_t current){return a.HostedUnaccountBot(current);}
    bool Roster(std::uintptr_t state,std::int32_t& count,bool& contains){return a.HostedReadRoster(state,count,contains);}
    bool PrepareAiClass(const cfghostedtx::Birth& b){return a.HostedPrepareAi(b);}
    bool PresentAiBestEffort(const cfghostedtx::Birth& b){return HostedPresentBirthKiller(a,b);}
    bool PrepareAi(const cfghostedtx::Birth& b){return cfghostedtx::PrepareAiWithBestEffortPresentation(*this,b);}
    bool AiBirthReady(const cfghostedtx::Birth& b,std::uintptr_t jason){return a.HostedAiBirthReady(b,jason);}
    bool PrepareHuman(std::uintptr_t state){return a.HostedPrepareHuman(state);}
    bool RestoreHumanClass(std::uintptr_t state){return a.HostedRestoreHumanClass(state);}
    bool PossessAi(const cfghostedtx::Birth& b,std::uintptr_t jason,bool& entered){return a.HostedPossessAi(b,jason,entered);}
    bool PossessHuman(std::uintptr_t human,std::uintptr_t jason,bool& entered){return a.HostedPossessHuman(human,jason,entered);}
    bool Restart(std::uintptr_t mode,std::uintptr_t human,bool& entered){return a.HostedRestart(mode,human,entered);}
    bool ReadPost(const cfghostedtx::Snapshot& s,const cfghostedtx::Birth& b,cfghostedtx::Post& p){return a.HostedReadPost(s,b,p);}
    bool InWorld(std::uintptr_t actor){return a.HostedInWorld(actor);}
    bool Destroy(std::uintptr_t actor){return a.HostedDestroy(actor);}
    bool WriteKillerOwner(std::uintptr_t expected,std::uintptr_t desired){return a.HostedWriteKillerOwner(expected,desired);}
    bool Slot(std::uintptr_t object,cfgprobe::lifetime::SlotObservation& slot){return a.HostedSlot(object,slot);}
#if CFG_H247_HOST_TAKEOVER
    // H2.48: donor-sequenced local view/HUD/input refresh over the byte-pinned 3.5 natives (HostedLocalViewMembers.hpp,
    // RuntimeProbe/HostLocalView35.generated.hpp). Ready is read-only; Refresh is the transaction's LocalView stage.
    bool LocalViewReady(){
#if CFG_H248_LOCAL_VIEW
        const bool ready=a.HostedLocalViewReady();hostedH1.h250ViewFailure=a.HostedViewFailureCode();return ready;
#else
        return false;
#endif
    }
    bool LocalViewRefresh(const cfghostedtx::Snapshot& s,std::uintptr_t counselor){
        const bool ok=a.HostedLocalViewRefresh(s,counselor);
        if(!ok)hostedH1.h250ViewFailure=a.HostedViewRefreshCode(); // H2.52c: >=1000000 = refresh failure (see HostedLocalViewMembers.hpp)
#if CFG_H253_HOST_VIEW_REPAIR
        if(ok)a.HostedViewRepairArm(counselor);
#endif
        return ok;
    }
#endif
#if CFG_H255_HOST_AI_BIRTH
    bool PrepareAiPreborn(const cfghostedtx::Birth& b){return a.HostedPrepareAiPreborn(b);}
    bool ControllerPawn(std::uintptr_t controller,std::uintptr_t& pawn){return a.HostedControllerPawn(controller,pawn);}
    void BindJason(std::uintptr_t jason){hostedTx.snap.jason=jason;}
    bool AiPathAfterRestart(const cfghostedtx::Birth& b){return a.HostedAiPathAfterPossess(b);}
    bool PrebornKillerReady(const cfghostedtx::Birth& b,std::uintptr_t jason){return a.HostedPrebornKillerReady(b,jason);}
    bool PublishPrebornKiller(std::uintptr_t human,std::uintptr_t ai,std::uintptr_t jason){return a.HostedPublishPrebornKiller(human,ai,jason);}
    // No class-restore ticket: the host's ActiveCharacterClass is already its counselor.
    bool AdoptPreborn(const cfghosted::adoption::Receipt& receipt){return a.HostedNotifyAiTraveled(receipt)&&HostedStageLocalAdoption(receipt);}
#endif
    bool Adopt(const cfghosted::adoption::Receipt& receipt){
        if(!a.HostedNotifyAiTraveled(receipt)||!HostedStageLocalAdoption(receipt))return false;
        if(a.HostedQueueClassRestore())return true;
        hostedH1.adoptionPending=false;hostedH1.pendingReceipt={};HostedUnpublishAdoption();return false;
    }
};
static_assert(static_cast<unsigned>(cfghostedtx::Eligibility::Ready)==static_cast<unsigned>(cfghosted::takeover35::Eligibility::Ready)&&
    static_cast<unsigned>(cfghostedtx::Eligibility::HostSelected)==static_cast<unsigned>(cfghosted::takeover35::Eligibility::HostSelected),
    "takeover eligibility enums must stay aligned");
// Called at the top of the existing returned-callback lease prologue (ServiceQueuedGameplay). Swaps the process
// plan to the freshly proved hosted plan ONLY for an armed, stable, due takeover; the scope restores it otherwise.
bool HostedH1EnterPlan(std::uintptr_t actor)noexcept{
    auto& s=hostedH1;
#if CFG_H22_HOTKEYS
    if(s.disarmed)for(auto& ticket:hostedCounselorBrainQueue)ticket.pending=false;
#endif
    if(hostedClassRestore.pending&&hostedClassRestore.due&&s.adopted)
        return s.adoptedWorld==plan.world&&cfgprimary::CurrentHostedBinding().Matches(plan)&&
            (actor==plan.pawn||actor==bundleJason.pawn.object)&&GetCurrentThreadId()==observerOwnerThread;
    if(s.adoptionPending)return s.kept&&actor==s.adoptionPlan.pawn&&
        GetCurrentThreadId()==observerOwnerThread&&cfgprimary::CurrentHostedBinding().Matches(s.adoptionPlan);
#if CFG_H22_HOTKEYS
    for(unsigned i=0;i<hostedCounselorBrainQueue.size();++i){auto& ticket=hostedCounselorBrainQueue[i];
        if(!ticket.pending||actor!=ticket.localPawn||GetTickCount64()<ticket.due)continue;
        const auto charge=[&](){if(++ticket.retries>=3)ticket.pending=false;else ticket.due=GetTickCount64()+250ULL*ticket.retries;};
        if(s.disarmed||s.f3Quarantined){ticket.pending=false;continue;}
        Reader input;input.cap=8192;std::uintptr_t engine=0,viewport=0,world=0;
        if(!input.Read(plan.module+cfgprobe::EngineGlobalCandidate,engine)||!input.Read(engine+cfgprobe::EngineGameViewport,viewport)||
           !input.Read(viewport+cfgprobe::ViewportWorld,world)){charge();continue;}
        if(world!=ticket.world){ticket.pending=false;continue;}
        const cfghosted::native::Admission admission{plan.module,engine,viewport,world,plan.threadId,true,true};
        cfghosted::listen35::Context host;cfgobserver::Plan candidate;cfgprimary::HostedBinding binding;
        if(!cfghosted::listen35::Capture(input,admission,GetCurrentThreadId(),host)||
           !cfgprimary::BuildHostedLocalPlan(input,host,candidate,binding)){charge();continue;}
        const cfghostedfill::BrainRetryLocalIdentity localIdentity{ticket.world,ticket.localController,ticket.localPawn,
            ticket.localControllerSlot.index,ticket.localControllerSlot.serial,ticket.localPawnSlot.index,ticket.localPawnSlot.serial};
        if(!cfghostedfill::MatchBrainRetryLocal(localIdentity,candidate.world,actor,candidate.controller,
           candidate.objects[2].index,candidate.objects[2].serial,candidate.pawn,candidate.objects[3].index,candidate.objects[3].serial)){
            ticket.pending=false;continue;}
        s.f3SavedPlan=plan;s.f3SavedBinding=cfgprimary::CurrentHostedBinding();plan=candidate;
        cfgprimary::CurrentHostedBinding()=binding;s.f3Swapped=true;s.f3BrainWindow=true;s.f3BrainServiced=false;
        s.f3BrainTicketIndex=i;ticket.host=host;return true;
    }
    if(s.f3Pending&&actor==s.f3ActorHint&&GetTickCount64()>=s.f3Next&&!s.disarmed&&!s.f3Quarantined&&
       (!s.takeoverDue||!s.frameValid||s.takeoverTried||s.adopted)&&GetCurrentThreadId()==observerOwnerThread){
        HostedLobbyPublishScope publication;s.f3Next=GetTickCount64()+1000;
        Reader input;input.cap=8192;std::uintptr_t engine=0,viewport=0,world=0;
        if(input.Read(plan.module+cfgprobe::EngineGlobalCandidate,engine)&&input.Read(engine+cfgprobe::EngineGameViewport,viewport)&&input.Read(viewport+cfgprobe::ViewportWorld,world)){
            const cfghosted::native::Admission admission{plan.module,engine,viewport,world,plan.threadId,true,true};
            cfghosted::listen35::Context host;cfgobserver::Plan candidate;cfgprimary::HostedBinding binding;
            if(world==s.f3World&&cfghosted::listen35::Capture(input,admission,GetCurrentThreadId(),host)&&
               cfgprimary::BuildHostedLocalPlan(input,host,candidate,binding)&&actor==candidate.pawn){
                s.f3SavedPlan=plan;s.f3SavedBinding=cfgprimary::CurrentHostedBinding();plan=candidate;cfgprimary::CurrentHostedBinding()=binding;
                s.f3Swapped=true;s.f3Window=true;s.f3Serviced=false;hostedCounselor={};hostedCounselor.host=host;return true;
            }
        }
        if(++s.f3Rechecks<=3)s.f3Next=GetTickCount64()+(1000ULL<<(s.f3Rechecks-1));
        else{--s.f3Pending;s.f3Automatic=false;s.f3Rechecks=0;++hostedLobbyReport.f3Refused;hostedLobbyReport.f3Reason=14;hostedLobbyReport.f3Queued=s.f3Pending;}
    }
#endif
    if(!s.takeoverDue||s.takeoverTried||s.planSwapped||s.adopted||!s.armed||!s.frameValid||actor!=s.frame.localPawn.object||
       GetCurrentThreadId()!=observerOwnerThread)return false;
    s.takeoverDue=false;
    Reader r;r.cap=16384;std::uintptr_t engine=0,viewport=0,world=0;
    if(!r.Read(plan.module+cfgprobe::EngineGlobalCandidate,engine)||!r.Read(engine+cfgprobe::EngineGameViewport,viewport)||
       !r.Read(viewport+cfgprobe::ViewportWorld,world)){s.armRefusal=1;return false;}
    const cfghosted::native::Admission admission{plan.module,engine,viewport,world,plan.threadId,true,true};
    cfghosted::takeover35::Frame fresh{};
    if(!cfghosted::takeover35::Capture(r,admission,GetCurrentThreadId(),fresh)||
       (fresh.eligibility!=cfghosted::takeover35::Eligibility::Ready&&fresh.eligibility!=cfghosted::takeover35::Eligibility::HostSelected&&
        !(CFG_H255_HOST_AI_BIRTH&&fresh.eligibility==cfghosted::takeover35::Eligibility::HostPreborn&&hostedPreborn.saved))||fresh.killer.object!=s.frame.killer.object||
       fresh.human.object!=s.frame.human.object||fresh.humanState.object!=s.frame.humanState.object||
       fresh.localPawn.object!=s.frame.localPawn.object||fresh.count!=s.frame.count||fresh.bots!=s.frame.bots){s.armRefusal=2;s.stable.Reset();return false;}
#if CFG_H1_ROLE_SELECTION
    if(cfghostedrole::RejectNonHostWinner(CFG_H1_ROLE_SELECTION!=0,s.options.values.jasonCount!=0,
       fresh.human.object==fresh.host.local[2].object)){s.armRefusal=22;return false;}
#endif
    cfgobserver::Plan hostedPlan{};cfgprimary::HostedBinding binding{};
    const bool planBuilt=fresh.eligibility==cfghosted::takeover35::Eligibility::HostPreborn?
        cfgprimary::BuildHostedLocalPlan(r,fresh.host,hostedPlan,binding):cfgprimary::BuildHostedPlan(r,fresh,hostedPlan,binding);
    if(!planBuilt){s.armRefusal=3;return false;}
    s.originalLocalState=0;s.outro={};
    (void)r.Read(fresh.host.local[2].object+cfghosted::adoption::ControllerState,s.originalLocalState);
    s.freshFrame=fresh;s.savedPlan=plan;plan=hostedPlan;cfgprimary::CurrentHostedBinding()=binding;
    s.planSwapped=true;s.kept=false;s.takeoverTried=true;++s.attempts;s.takeoverTick=GetTickCount64();s.armRefusal=0;
    return true;
}
void HostedH1LeavePlan()noexcept{
    auto& s=hostedH1;
#if CFG_H22_HOTKEYS
    if(s.f3BrainWindow){
        if(!s.f3BrainServiced&&s.f3BrainTicketIndex<hostedCounselorBrainQueue.size()){
            auto& ticket=hostedCounselorBrainQueue[s.f3BrainTicketIndex];
            if(++ticket.retries>=3)ticket.pending=false;else ticket.due=GetTickCount64()+250ULL*ticket.retries;}
        if(s.f3Swapped){plan=s.f3SavedPlan;cfgprimary::CurrentHostedBinding()=s.f3SavedBinding;}
        s.f3Swapped=false;s.f3BrainWindow=false;s.f3BrainServiced=false;s.f3BrainTicketIndex=8;return;}
    if(s.f3Window){
        if(!s.f3Serviced&&s.f3Pending){HostedLobbyPublishScope publication;
            if(++s.f3Rechecks<=3)s.f3Next=GetTickCount64()+(1000ULL<<(s.f3Rechecks-1));
            else{--s.f3Pending;s.f3Automatic=false;s.f3Rechecks=0;++hostedLobbyReport.f3Refused;hostedLobbyReport.f3Reason=14;hostedLobbyReport.f3Queued=s.f3Pending;}}
        if(s.f3Swapped){plan=s.f3SavedPlan;cfgprimary::CurrentHostedBinding()=s.f3SavedBinding;}s.f3Window=false;s.f3Swapped=false;return;}
#endif
    if(s.adoptionPending&&!s.kept){
        // This destructor runs after BundleCallbackRootScope/lookup eligibility.
        // Never carry a lease/witness to the new local pawn.
        if(bundleRootLease)return;
        plan=s.adoptionPlan;cfgprimary::CurrentHostedBinding()=s.adoptionBinding;
        s.kept=true;s.adopted=true;s.adoptedWorld=plan.world;return;
    }
    if(!s.planSwapped||s.kept)return;               // adopted: the hosted plan stays installed for the driver
    plan=s.savedPlan;cfgprimary::CurrentHostedBinding().Retire();s.planSwapped=false;
}
// Runs INSIDE the verified returned-ReceiveTick lease (RetainedWorld/hosted plan current). One transaction, no replay.
__declspec(noinline) bool HostedH1Window(void* object,bool wasNested)noexcept{
    (void)object;auto& s=hostedH1;
    if(!s.presentationPrimaryAttempted&&wasNested){s.presentationNotAttemptedReason=1;++s.presentationNotAttemptedWindows;}
    if(hostedClassRestore.pending&&hostedClassRestore.due&&!wasNested){
        BundleAccess access;(void)access.HostedRestoreClassAtEnd();return true;
    }
    if(s.adoptionPending&&s.kept&&!wasNested){
        // Native handoff is complete; activation retries are read-only setup,
        // never another controller birth/possess/restart.
        ++s.adoptionInitAttempts;
        if(HostedActivateAdoption(s.pendingReceipt)){s.adoptionPending=false;s.pendingReceipt={};}
        else if(s.adoptionInitAttempts>=3){s.adoptionPending=false;s.pendingReceipt={};s.adoptionRefusal=11;}
        return true;
    }
#if CFG_H22_HOTKEYS
    if(s.f3BrainWindow&&!wasNested){
        s.f3BrainServiced=true;
        if(s.f3BrainTicketIndex>=hostedCounselorBrainQueue.size())return true;
        auto& ticket=hostedCounselorBrainQueue[s.f3BrainTicketIndex];
        if(!ticket.pending)return true;
        const auto savedTx=hostedTx;const auto savedBirth=hostedCounselor;
        hostedCounselor={};hostedCounselor.host=ticket.host;hostedCounselor.before=ticket.before;
        hostedCounselor.born=ticket.born;hostedCounselor.pawn=ticket.pawn;hostedCounselor.pawnSlot=ticket.pawnSlot;
        hostedTx.Reset();hostedTx.gameState=hostedCounselor.host.session.State();
        BundleAccess access;bool ready=false;const bool stable=access.HostedCounselorRetryTuple(ticket.world,ticket.localController,
            ticket.localPawn,hostedCounselor.pawn,ticket.born.controller,ticket.born.state,ticket.pawnSlot,ready);
        const auto trace=std::find_if(hostedBots.begin(),hostedBots.end(),[&](const auto& d){
            return d.pawn==hostedCounselor.pawn&&d.controller==hostedCounselor.born.controller.object;});
        auto decision=cfghostedfill::DecideBrainRetry(stable&&trace!=hostedBots.end(),ready,
            trace!=hostedBots.end()&&trace->startAttempts!=0,ticket.retries);
        const bool started=decision==cfghostedfill::BrainRetry::Dispatch&&access.HostedCounselorBirthBrain();
        ticket.born=hostedCounselor.born;ticket.pawnSlot=hostedCounselor.pawnSlot;
        if(decision==cfghostedfill::BrainRetry::Cancel||started||
           (trace!=hostedBots.end()&&trace->startAttempts)){ticket.pending=false;}
        else if(++ticket.retries>=3)ticket.pending=false;
        else ticket.due=GetTickCount64()+250ULL*ticket.retries;
        hostedCounselor=savedBirth;hostedTx=savedTx;
        bundleReport.reads+=access.Reads();bundleReport.calls+=access.Calls();return true;
    }
    if(s.f3Window&&!wasNested){
        HostedLobbyPublishScope publication;
        if(!s.f3Pending)return false;s.f3Serviced=true;s.f3Rechecks=0;--s.f3Pending;hostedLobbyReport.f3Queued=s.f3Pending;++hostedLobbyReport.f3Attempts;
        const auto saved=hostedTx;hostedTx.Reset();BundleAccess access;
        struct BirthHost{BundleAccess& a;
            bool Preflight(){return a.HostedCounselorPreflight();}bool Spawn(){return a.HostedCounselorSpawn();}
            bool Clean(){return !hostedCounselor.fault&&!hostedTx.birth.controller;}
            bool NewController(){return a.HostedCounselorNew();}bool SetClass(){return a.HostedCounselorClass();}
            bool Restart(){return a.HostedCounselorRestart();}bool Traveled(){return a.HostedCounselorTraveled();}
            bool Account(){return a.HostedCounselorAccount();}bool Postconditions(){return a.HostedCounselorPost();}
            bool Rollback(){hostedCounselor.failedStage=hostedCounselor.stage;return a.HostedCounselorRollback();}
        }host{access};unsigned reason=0;const auto outcome=cfghostedfill::AddOne(host,reason);
        hostedLobbyReport.f3Reason=reason;
        hostedLobbyReport.f3CapReached=reason==1&&hostedCounselor.stage==14&&
            !cfghostedfill::CounselorRoom(hostedCounselor.before.list.count,s.armed,s.adopted);
        hostedLobbyReport.f3CapPlayers=hostedLobbyReport.f3CapReached?hostedCounselor.before.list.count:0;
        if(reason&&!hostedLobbyReport.f3FirstReason){hostedLobbyReport.f3FirstReason=reason;hostedLobbyReport.f3FirstStage=hostedCounselor.failedStage?hostedCounselor.failedStage:hostedCounselor.stage;hostedLobbyReport.f3FirstCalls=access.Calls();}
        hostedLobbyReport.f3TravelReturned=hostedCounselor.travelReturned;hostedLobbyReport.f3TravelReady=hostedCounselor.travelReady;
        hostedLobbyReport.packetReason=hostedCounselor.packetDiagnostic.reason;hostedLobbyReport.packetKind=hostedCounselor.packetDiagnostic.kind;hostedLobbyReport.packetIndex=hostedCounselor.packetDiagnostic.index;hostedLobbyReport.packetAddress=hostedCounselor.packetDiagnostic.address;hostedLobbyReport.f3NativeStage=hostedCounselor.stage;hostedLobbyReport.f3NativeCalls=access.Calls();
        hostedLobbyReport.f3BeforePlayers=hostedCounselor.before.list.count;hostedLobbyReport.f3BeforeBots=hostedCounselor.before.bots;
        hostedLobbyReport.f3Controller=hostedTx.birth.controller;hostedLobbyReport.f3State=hostedTx.birth.state;hostedLobbyReport.f3Pawn=hostedCounselor.pawn;
        if(outcome==cfghostedfill::Result::Completed){
            ++hostedLobbyReport.f3Completed;const bool brainStarted=access.HostedCounselorBirthBrain();
            const auto trace=std::find_if(hostedBots.begin(),hostedBots.end(),[&](const auto& d){
                return d.pawn==hostedCounselor.pawn&&d.controller==hostedCounselor.born.controller.object;});
            if(!brainStarted&&trace!=hostedBots.end()&&!trace->startAttempts&&hostedCounselor.born.controller.HasAssignedSerial()&&
               hostedCounselor.born.state.object&&hostedCounselor.pawnSlot.object&&plan.pawn!=hostedCounselor.pawn&&
               plan.objects[2].HasAssignedSerial()&&plan.objects[3].HasAssignedSerial()&&
               access.HostedLocalCallbackCurrent(plan.objects[2],plan.objects[3])){
                auto free=std::find_if(hostedCounselorBrainQueue.begin(),hostedCounselorBrainQueue.end(),
                    [](const auto& ticket){return !ticket.pending;});
                if(free!=hostedCounselorBrainQueue.end()){
                    *free={};free->pending=true;free->due=GetTickCount64()+250;free->world=plan.world;
                    free->localController=plan.controller;free->localPawn=plan.pawn;
                    free->localControllerSlot=plan.objects[2];free->localPawnSlot=plan.objects[3];
                    free->host=hostedCounselor.host;free->before=hostedCounselor.before;free->born=hostedCounselor.born;
                    free->pawn=hostedCounselor.pawn;free->pawnSlot=hostedCounselor.pawnSlot;
                }
            }
        }
        else ++hostedLobbyReport.f3Refused;
        if(s.f3Automatic){
            if(outcome==cfghostedfill::Result::Completed){++s.autoFillAdded;s.autoFillRetries=0;}
            else{++s.autoFillRefused;const bool retry=outcome==cfghostedfill::Result::Refused&&reason==1&&!hostedCounselor.entered&&access.HostedRetryAllowed();
                s.autoFillStopped=!retry;s.autoFillRetries=(std::min)(s.autoFillRetries+1,4u);s.autoFillNext=GetTickCount64()+(1000ULL<<(s.autoFillRetries-1));}
            s.f3Automatic=false;}
        s.f3Quarantined=outcome==cfghostedfill::Result::Quarantined;if(s.f3Quarantined){s.f3Pending=0;hostedCounselorBrainQueue={};hostedLobbyReport.f3Reason=100+reason;}
        hostedTx=saved;bundleReport.reads+=access.Reads();bundleReport.calls+=access.Calls();return true;
    }
#endif
    if(wasNested||!s.planSwapped||s.kept)return false;
    const auto& f=s.freshFrame;cfghostedtx::Snapshot snap{};
    snap.eligibility=static_cast<cfghostedtx::Eligibility>(static_cast<unsigned>(f.eligibility));
    snap.world=plan.world;snap.mode=plan.mode;snap.gameState=f.host.session.State();snap.jason=f.killer.object;snap.human=f.human.object;
    snap.humanState=f.humanState.object;snap.localController=f.host.local[2].object;snap.localPawn=f.localPawn.object;
    snap.bots=f.bots;snap.players=static_cast<std::int32_t>(f.count);snap.counselorTreeLoaded=f.counselorTree!=0;
    hostedTx.Reset();hostedTx.snap=snap;hostedTx.gameState=snap.gameState;
    BundleAccess access;HostedOps ops{access};
    cfghostedtx::Takeover<HostedOps> takeover(ops);
    const auto result=takeover.Run(snap);
    if(!s.presentationPrimaryAttempted){
        s.presentationNotAttemptedReason=!HostedRoutineActive()||!hostedAdoptionActive?2:3;
        ++s.presentationNotAttemptedWindows;
    }
    s.outcome=static_cast<std::uint32_t>(result.outcome);s.stage=static_cast<std::uint32_t>(result.stage);
    s.reason=static_cast<std::uint32_t>(result.reason);s.rollbackVerified=result.rollbackVerified?1u:0u;
    s.effects=result.nativeEffects;s.aiController=result.receipt.controller.object;
    s.packetDiagnostic=hostedTx.packetDiagnostic;s.classStage=hostedTx.classStage;s.classWarmCalls+=hostedTx.classWarmCalls;s.classReflection=hostedTx.classReflection;s.classConstructor=hostedTx.classConstructor;
#if CFG_H22_HOTKEYS
    s.takeoverRetrySafe=cfghostedtx::RetryBeforeBirth(result,access.HostedRetryAllowed());
    if(s.takeoverRetrySafe){++s.retryCount;s.retryAt=GetTickCount64()+cfghostedtx::RetryBackoff(s.retryCount);}
#endif
    if(result.outcome==cfghostedtx::Outcome::Adopted&&hostedAdoptionActive){
        s.kept=true;s.adopted=true;s.adoptedWorld=plan.world;
    }
    bundleReport.reads+=access.Reads();bundleReport.calls+=access.Calls();
    return true;
}
void HostedH1ServiceAdditionalRequests(std::uint64_t now)noexcept{
    auto& s=hostedH1;
    if(!HostedRoutineActive()||!s.adopted||!s.kept||!hostedAdoptionActive||
       s.options.values.jasonCount<=1||hostedJasonExtraCountConsumed||now<s.extraRequestNextAttempt)return;
    s.extraRequestNextAttempt=now+1000;
    BundleAccess access(256);if(!access.AdditionalInProgress())return;
    if(!s.extraRequestTimerStarted){s.extraRequestTimerStarted=true;s.extraRequestMatchAt=now;return;}
    if(now<s.extraRequestMatchAt||now-s.extraRequestMatchAt<180000)return;
    if(bundleAdditional.policy.Request(true,true,true,now)){
        hostedJasonExtraCountConsumed=true;s.extraRequestAccepted=true;
    }else (void)bundleAdditional.policy.Request(false,true,true,now);
}
#endif
void HostedH1ServicePrimaryPresentation(std::uint64_t now)noexcept{
#if CFG_H243_PRESENTATION && defined(CFG_H1_JASON)
    auto& h=hostedH1;
    if(h.presentationPrimaryAttempted||h.presentationPrimaryComplete||!HostedRoutineActive()||!h.adopted||!h.kept||!hostedAdoptionActive||
       now<h.primaryPresentationNext||h.primaryPresentationAttempts>=cfghostedpresent::MaxPreNativeAttempts||
       (h.takeoverTick&&now<h.takeoverTick+2000))return;
    h.primaryPresentationNext=now+1000;const auto attempt=h.primaryPresentationAttempts++;
    // Donor 10331 copies from the displaced human's PlayerState; the host's own state is the fallback source.
    const bool human=attempt<3&&hostedTx.snap.human&&hostedTx.snap.humanState;
    const auto sourceController=human?hostedTx.snap.human:h.freshFrame.host.local[2].object;
    const auto sourceState=human?hostedTx.snap.humanState:h.originalLocalState;
    std::uintptr_t targetState=0;Reader stateReader;stateReader.cap=64;
    if(!bundleJason.pawn.object||!stateReader.Read(bundleJason.pawn.object+cfgescape::disk::PlayerState,targetState))targetState=0;
    const auto selectedClass=spawnAssets.killerTemplate.pawnClass;
    if(!targetState||!sourceController||!sourceState||!selectedClass){h.presentationRefusal=2;return;}
    BundleAccess check(512);const bool before=check.HostedAdoptionCurrent();
    BundleAccess access(1024);
    std::uintptr_t skin=0,skinBefore=0;cfgending::Packet weapon{},weaponBefore{};
    const bool copied=access.HostedCopyKillerPresentation(targetState,sourceState,sourceController,selectedClass,false,false,
        skin,weapon,skinBefore,weaponBefore);
    const unsigned refusal=access.HostedPresentationCopyRefusal();
    h.presentationCopyRefusal=refusal;h.presentationCopyBundleNativeStage=access.HostedPresentationCopyBundleNativeStage();
    h.presentationSkinBefore=skinBefore;h.presentationWeaponBefore=weaponBefore;
    h.primaryPresentationSkinBefore=skinBefore;h.primaryPresentationWeaponBefore=weaponBefore;
    BundleAccess after(512);
    // Evidence only: 100 + 10*current-before + current-after (1 = adoption::Current passed). Never gates anything.
    h.presentationNotAttemptedReason=100u+(before?10u:0u)+(after.HostedAdoptionCurrent()?1u:0u);
    if(!copied){
        h.presentationRefusal=3;
        if(!cfghostedpresent::RetrySafe(refusal))h.presentationPrimaryAttempted=true; // a setter may have been entered: never replay
        return;
    }
    h.presentationPrimaryAttempted=true;
    h.presentationSkin=skin;h.presentationWeapon=weapon;h.primaryPresentationSkin=skin;h.primaryPresentationWeapon=weapon;
    h.presentationPrimaryComplete=true;h.presentationRefusal=0;
#else
    (void)now;
#endif
}
#ifdef CFG_H1_JASON
void HostedH1ServiceAdditionalPresentation()noexcept{
    auto& s=hostedH1;
    if(!HostedRoutineActive()||!s.adopted||!s.kept||!hostedAdoptionActive||!hostedExtraPresentationPending)return;
    hostedExtraPresentationPending=false;s.presentationExtraPending=true;s.presentationExtraComplete=false;
    (void)HostedPresentCurrentKiller(true);
}
#endif
#include "HostedKeysRuntime.hpp"
#if CFG_H22_HOTKEYS
#include "HostedStatusRuntime.hpp"
#endif
inline void HostedH1Publish(const cfgh1::Report& next) noexcept {
    auto& live=hostedH1Report;
    const auto generation=live.generation;
    InterlockedExchange(reinterpret_cast<volatile LONG*>(&live.generation),static_cast<LONG>(generation|1u));
    MemoryBarrier();
    auto copy=next;copy.generation=generation|1u;copy.bytes=sizeof(cfgh1::Report);copy.magic=cfgh1::Magic;copy.version=cfgh1::Version;
    std::memcpy(&live,&copy,sizeof(live));
    MemoryBarrier();
    InterlockedExchange(reinterpret_cast<volatile LONG*>(&live.generation),static_cast<LONG>((generation|1u)+1u));
}
// Called from Observe()'s finally block, after the original dispatch returned, BEFORE the offline
// gameplay service. Owner thread, non-nested only. Cadence-limited; offline sessions pay 4 reads/sec.
inline void ServiceHostedH1(bool wasNested,bool returned,std::uintptr_t actor=0,std::uintptr_t function=0,std::uintptr_t pc=0) noexcept {
    if(wasNested||!returned||GetCurrentThreadId()!=observerOwnerThread||!plan.module||plan.threadId!=observerOwnerThread)return;
    const auto now=GetTickCount64();
    if(HostedAdoptionOwnsDriver()&&now>=hostedEndWatchAt){
        hostedEndWatchAt=now+1000;BundleAccess ending(512);
        const auto state=ending.ObserveHostedEndState();
        bundleReport.reads+=ending.Reads();bundleReport.maxReads=(std::max)(bundleReport.maxReads,ending.Reads());
        if(state==2){
            auto& s=hostedH1;
            auto& e=s.outro;
            e.originalPawn=s.freshFrame.localPawn.object;e.originalState=s.originalLocalState;
            e.planSwapped=s.planSwapped;e.restorePending=hostedClassRestore.pending;
            e.restoreRefused=hostedClassRestore.refused;e.rootOwners=bundleRootBank.Owners();
            e.leaseFlags=(bundleRootLease?1u:0u)|(bundleRootWitness.lease?2u:0u)|
                (bundleRootBank.Poisoned()?4u:0u)|(NativeJasonControllerTickInFlight()?8u:0u);
            Reader facts;facts.cap=512;cfgobserver::Plan current{};
            if(cfgprimary::RetainedWorld(facts,plan,current)){
                using namespace cfghosted::adoption;
                const auto& receipt=hostedAdoptionReceipt;
                if(cfgjason::SameSlot(facts,current.objectArray,plan.objects[2])){
                    e.Read(facts,plan.controller+ControllerPawn,e.localPawn,0);
                    e.Read(facts,plan.controller+ControllerState,e.localState,1);
                }
                if(cfgjason::SameSlot(facts,current.objectArray,receipt.pawn)){
                    e.Read(facts,receipt.pawn.object+PawnController,e.jasonController,2);
                    e.Read(facts,receipt.pawn.object+PawnState,e.jasonState,3);
                }
                if(cfgjason::SameSlot(facts,current.objectArray,receipt.controller))
                    e.Read(facts,receipt.controller.object+ControllerPawn,e.aiPawn,4);
                if(spawnAssets.gameState&&cfgobserver::Same(facts,plan.world+cfgprobe::WorldGameState,spawnAssets.gameState))
                    e.Read(facts,spawnAssets.gameState+cfgprimary::CurrentKillerState,e.killerState,5);
            }
            (void)cfghostedend::Freeze(hostedEndFrozen,hostedDriverTrace.endFreezeAt,
                hostedDriverTrace.endFreezeCount,now);
            auto end=hostedH1Report;end.outro=e;end.driver=hostedDriverTrace;end.matchInProgress=0;
            HostedH1Publish(end);
        }
    }
#if CFG_H21_LOBBY_DISCOVERY
    ServiceHostedLobby();
#if CFG_H21_LOBBY_ROWS
    ServiceHostedLobbyRows(actor,function,pc);
#endif
#endif
#if CFG_H22_HOTKEYS
    ServiceHostedKeys();
    ServiceHostedStatus();
#endif
    auto& s=hostedH1;
    if(now<s.nextAt)return;
#if CFG_H20_FILL
    (void)PrepareHostedFill(plan.module);
#endif
#if CFG_H245_HOST_JASON
    // H2.49: install the PreMatchIntro lottery hook as early as the fill hook (live
    // H2.48d: InstallAt was 7 s before the takeover, after PreMatchIntro had run, so
    // Calls=0). The armed/options gate is evaluated inside the detour at call time.
    if(!hostedRoleSelectionHookInstalled&&!hostedRolePinsFailed)(void)InstallHostedRoleSelectionBridge();
#endif
    auto next=hostedH1Report;
    next.buildFlags=cfgh1::FlagObserve
#ifdef CFG_H1_JASON
        |cfgh1::FlagJason
#endif
#if defined(CFG_H1_FILL) || CFG_H20_FILL
        |cfgh1::FlagFill
#endif
        ;
    Reader reader;reader.cap=16384;
    std::uintptr_t engine=0,viewport=0,world=0;
    ++next.samples;next.lastSampleTick=now;
    cfgh1::Stage stage=cfgh1::Stage::WorldUnreadable;
    cfghosted::takeover35::Frame frame{};bool valid=false;
    if(reader.Read(plan.module+cfgprobe::EngineGlobalCandidate,engine)&&cfgprobe::ObjectPointer(engine)&&
       reader.Read(engine+cfgprobe::EngineGameViewport,viewport)&&cfgprobe::ObjectPointer(viewport)&&
       reader.Read(viewport+cfgprobe::ViewportWorld,world)&&cfgprobe::ObjectPointer(world)){
        const cfghosted::native::Admission admission{plan.module,engine,viewport,world,plan.threadId,true,true};
        stage=cfgh1::Sample(reader,admission,GetCurrentThreadId(),next,frame,valid);
    }
    next.stage=static_cast<std::uint32_t>(stage);
#if CFG_H254_HOST_PREBORN&&CFG_H260_PREBORN_DEFER
    if(hostedPreborn.pending&&now>=hostedPreborn.at&&GetCurrentThreadId()==observerOwnerThread){
        hostedPreborn.pending=false; // one attempt only
        if(hostedPreborn.world==world)HostedPrebornCounselor(hostedPreborn.mode,hostedPreborn.state);
    }
#endif
#ifdef CFG_H1_JASON
    if(next.matchInProgress){
        if(!s.extraRequestTimerStarted){s.extraRequestTimerStarted=true;s.extraRequestMatchAt=now;}
    }else if(!hostedJasonExtraCountConsumed){s.extraRequestTimerStarted=false;s.extraRequestMatchAt=0;s.extraRequestNextAttempt=0;}
#endif
#ifdef CFG_H1_JASON
    if(valid)ObserveHostedStockBaseline(frame,plan.module);
    next.stockBaseline=hostedStockBaseline;next.stockBaselineWorld=hostedStockWorld;
#endif
#if CFG_H20_FILL
    next.fillStockDeferred=s.stockFillDeferred;next.fillStockWorld=s.stockFillWorld;
    next.fillReason=hostedFillReason;next.fillContextStage=hostedFillContextStage;
    next.fillReady=hostedFillReady?1u:0u;next.fillEntries=hostedFillReport.entries;next.fillApplied=hostedFillReport.applied;
    next.fillRefused=hostedFillReport.refused+hostedFillReport.restoreRefused;next.fillRestored=hostedFillReport.restored;
    next.fillHumans=hostedFillHumans;next.fillExpected=hostedFillExpected;
    next.fillBotsBefore=hostedFillBotsBefore;next.fillBotsAfter=hostedFillBotsAfter;
#endif
    const auto& stored=cfghostedoptions::Current();
    next.storedOptions=stored.values.Mask();next.storedDifficulty=stored.values.difficulty;next.optionsRevision=stored.revision;
    const bool hostMatch=stage==cfgh1::Stage::ContextPassed||stage==cfgh1::Stage::FrameRefused||stage==cfgh1::Stage::FramePassed;
    if(hostMatch)(void)s.options.Capture(world,true,stored);
    else if(s.options.world&&world!=s.options.world){s.options.Retire();}
#if CFG_H1_ROLE_SELECTION || CFG_H245_HOST_JASON
    // H2.48c: the native Jason lottery runs at match start, before the hosted context
    // passes, so a match-only install always arrived after the lottery (live: RoleHook=1,
    // Attempts=0). Install as soon as the AI Jason is armed (lobby included); the bridge
    // itself still refuses offline worlds (preflight refusal 12) and needs every pin.
    if((CFG_H245_HOST_JASON?true:hostMatch)&&s.armed&&s.options.values.jasonCount&&!hostedRoleSelectionHookInstalled){
        ++s.rolePreferenceInstallAttempts;
        if(!InstallHostedRoleSelectionBridge()){
            ++s.rolePreferenceInstallRefused;s.rolePreferenceRefusal=9;}
    }
#endif
    next.matchOptions=s.options.values.Mask();next.matchDifficulty=s.options.values.difficulty;next.matchOptionsRevision=s.options.revision;
    if(stage!=cfgh1::Stage::NoNetDriver&&stage!=cfgh1::Stage::WorldUnreadable){
        ++next.netDriverSamples;if(!next.firstHostedTick)next.firstHostedTick=now;}
    if(stage==cfgh1::Stage::ContextRefused)++next.contextRefusals;
    if(stage==cfgh1::Stage::ContextPassed||stage==cfgh1::Stage::FrameRefused||stage==cfgh1::Stage::FramePassed)++next.contextPasses;
    if(stage==cfgh1::Stage::FrameRefused)++next.frameRefusals;
    if(stage==cfgh1::Stage::FramePassed)++next.framePasses;
    s.frameValid=valid;if(valid){s.frame=frame;s.frameAt=now;}
#if CFG_H22_HOTKEYS
    if(s.disarmed)hostedCounselorBrainQueue={};
    else {
        bool pending=false,sameWorldPending=false;for(const auto& ticket:hostedCounselorBrainQueue)if(ticket.pending){
            pending=true;sameWorldPending|=ticket.world==world;}
        cfghosted::listen35::Context observedHost{};
        bool hostProven=valid&&frame.host.session.World()==world&&cfgprobe::ObjectPointer(world);
        if(pending&&world&& !hostProven){
            const cfghosted::native::Admission admission{plan.module,engine,viewport,world,plan.threadId,true,true};
            hostProven=cfghosted::listen35::Capture(reader,admission,GetCurrentThreadId(),observedHost)&&
                observedHost.session.World()==world;
        }else if(hostProven)observedHost=frame.host;
        if(hostProven){
            cfgobserver::Plan currentLocal{};cfgprimary::HostedBinding currentBinding{};bool localPlanProven=false;
            if(sameWorldPending)localPlanProven=cfgprimary::BuildHostedLocalPlan(reader,observedHost,currentLocal,currentBinding);
            for(auto& ticket:hostedCounselorBrainQueue)if(ticket.pending){
                const bool changedWorld=world!=ticket.world;
                if(cfghostedfill::RetireBrainTicket(changedWorld,false,false)){ticket.pending=false;continue;}
                if(!localPlanProven)continue; // failed takeover/local proof never expires a same-world ticket
                const cfghostedfill::BrainRetryLocalIdentity expected{ticket.world,ticket.localController,ticket.localPawn,
                    ticket.localControllerSlot.index,ticket.localControllerSlot.serial,ticket.localPawnSlot.index,ticket.localPawnSlot.serial};
                const bool localMatches=cfghostedfill::MatchBrainRetryLocal(expected,currentLocal.world,ticket.localPawn,
                    currentLocal.controller,currentLocal.objects[2].index,currentLocal.objects[2].serial,
                    currentLocal.pawn,currentLocal.objects[3].index,currentLocal.objects[3].serial);
                if(cfghostedfill::RetireBrainTicket(false,true,localMatches))ticket.pending=false;
            }
        }
    }
#endif
    // Cadence: slow while no hosted world; steady once a NetDriver exists.
    s.nextAt=now+(stage==cfgh1::Stage::NoNetDriver||stage==cfgh1::Stage::WorldUnreadable?1000:500);
#ifdef CFG_H1_JASON
    // A new hosted world (or the end of hosting) re-arms the one-shot and retires any prior adoption (#349 item 6).
    const bool hostedNow=stage==cfgh1::Stage::ContextPassed||stage==cfgh1::Stage::FrameRefused||stage==cfgh1::Stage::FramePassed;
#if CFG_H275_OFFLINE_THEN_HOSTED
    // H2.75: a previous offline match's bundle epoch (schedule/Jason/spawn state) survives a menu->host trip and makes
    // hosted clause 8 (bundleSchedule.Start) fail. Same weak-only reset the hosted->offline path uses, before takeover.
    {static std::uintptr_t h293RetiredFor=0; // the finished-match retire runs once per hosted world (the reset keeps plan.world)
     const unsigned leftover=cfghostedoffline::LeftoverEpochDecision(true,hostedNow,
        s.adopted||s.planSwapped||s.adoptionPending||hostedRoutineStarted||hostedAdoptionActive||HostedRoutineActive(),
        // H2.93: a FINISHED offline match (schedule no longer active) leaves the same epoch state behind (F1, traps, catalogue);
        // retire it too whenever its world is not this hosted world. Same retirable/lease/hosted-inactive guards as before.
        CFG_H293_LEFTOVER_ANY?(bundleStarted||(plan.world!=0&&h293RetiredFor!=world)):bundleStarted,
        CFG_H293_LEFTOVER_ANY?(bundleSchedule.active||h293RetiredFor!=world):bundleSchedule.active,
        plan.world,world,bundleRootLease||bundleRootWitness.lease,BundleEpochRetirable());
     if(leftover==cfghostedoffline::Retire){BundleResetEpoch();h293RetiredFor=world;cfgclosedtrace::Mark("h275-offline-epoch-retired",plan.world,world);}
     else if(leftover==cfghostedoffline::Blocked){static unsigned blocked=0;
        if(blocked<16){++blocked;cfgclosedtrace::Mark("h275-offline-epoch-blocked",plan.world,BundleEpochRetireMask());}}}
#endif
    if(s.adopted&&hostedClassRestore.pending&&world==s.adoptedWorld){
        // A valid Hunt world survives InProgress -> post-match; waiting for
        // ContextRefused misses the last safe restoration window entirely.
        Reader end;end.cap=512;cfgnames::Words phase{};cfgnames::Text text;cfgnames::Observation<Reader> names(end,plan.module);
        cfgobserver::Plan current{};
        if(cfgprimary::CurrentHostedBinding().Matches(plan)&&cfgprimary::RetainedWorld(end,plan,current)&&
           end.Read(plan.mode+cfgescape::disk::MatchState,phase)&&names.Decode(phase,text)&&
           cfghostedrestore::EndPhase(text.View())&&
           cfgescape::Same(end,plan.mode+cfgescape::disk::MatchState,phase))hostedClassRestore.due=true;
    }
    if(s.adopted&&(!hostedNow||world!=s.adoptedWorld||stage==cfgh1::Stage::ClientOrForeign)){
        // Terminal Hunt callbacks still own the old World. Restore only there,
        // before travel; never call an old player after a new World arrives.
        cfgobserver::Plan retained{};Reader ending;ending.cap=512;
        if(world==s.adoptedWorld&&cfgprimary::CurrentHostedBinding().Matches(plan)&&
           cfgprimary::RetainedWorld(ending,plan,retained)){
            // Native phase predicates stop gameplay. Keep same-world local UI
            // observations alive through the outro; travel retires the receipt.
            if(hostedClassRestore.pending)hostedClassRestore.due=true;
        }else HostedH1Retire();
    }
    if(!hostedNow&&!s.adopted&&!s.planSwapped){s.takeoverTried=false;s.takeoverDue=false;s.takeoverWorld=0;}
    if(hostedNow&&s.takeoverWorld&&s.takeoverWorld!=world&&!s.adopted){s.takeoverTried=false;s.takeoverDue=false;}
    if(hostedNow)s.takeoverWorld=world;
#if !CFG_H22_HOTKEYS
    if(stage==cfgh1::Stage::FramePassed){
        DWORD foreground=0;const auto window=GetForegroundWindow();if(window)GetWindowThreadProcessId(window,&foreground);
        const SHORT key=GetAsyncKeyState(VK_F8);const bool down=(key&0x8000)!=0;
        if(foreground==GetCurrentProcessId()&&((down&&!s.f8Down)||(key&1))){s.manualArm=true;++s.f8Presses;
#if CFG_H22_HOTKEYS
            s.disarmed=false;
#endif
        }
        s.f8Down=down;
    }else s.f8Down=false;
#endif
    if(now>=s.armCheckAt&&stage==cfgh1::Stage::FramePassed){
        s.armed=s.options.values.jason||s.manualArm||HostedH1ArmFilePresent();
#if CFG_H22_HOTKEYS
        if(s.disarmed)s.armed=false;
#endif
        s.armCheckAt=now+2000;
    }
    // Operator preference survives travel; native entry still requires a fresh stable READY frame.
#if CFG_H22_HOTKEYS
    if(!s.disarmed&&(s.manualArm||s.options.values.jason))s.armed=true;
    if(s.disarmed)s.armed=false;
#else
    if(stage!=cfgh1::Stage::FramePassed&&!s.adopted)s.armed=false;
#endif
    next.armed=s.armed?1u:0u;
    next.f8Presses=s.f8Presses;
    const bool stableReady=s.stable.Observe(next,now,3000,3);
    next.stableFrames=s.stable.frames;
    if(s.armed&&stableReady&&s.takeoverRetrySafe&&s.takeoverTried&&now>=s.retryAt){s.takeoverTried=false;s.takeoverRetrySafe=false;}
    if(s.armed&&stableReady&&!s.takeoverTried&&!s.takeoverDue&&!s.adopted)s.takeoverDue=true; // consumed once per hosted world
    next.packetReason=s.packetDiagnostic.reason;next.packetKind=s.packetDiagnostic.kind;next.packetIndex=s.packetDiagnostic.index;next.packetAddress=s.packetDiagnostic.address;next.classStage=s.classStage;next.classWarmCalls=s.classWarmCalls;next.classReflection=s.classReflection;next.classConstructor=s.classConstructor;next.retryCount=s.retryCount;next.retryAt=s.retryAt;
#if CFG_H22_HOTKEYS
    if(s.fillStableWorld!=world){hostedLobbyReport.f3CapReached=0;hostedLobbyReport.f3CapPlayers=0;s.fillStableWorld=world;s.fillStableAt=now;s.autoFillStopped=false;s.autoFillQueued=s.autoFillAdded=s.autoFillRefused=s.autoFillRetries=0;s.autoFillNext=0;s.fillHasFrame=false;}
    if(!hostMatch){s.fillStableAt=now;s.fillHasFrame=false;}
    if(stage==cfgh1::Stage::FramePassed)s.fillHasFrame=true;
    if(cfghostedfill::DeferredReady(hostMatch,s.fillHasFrame,(s.options.values.fill!=0||(s.stockFillWanted&&s.stockFillWorld==world)),s.disarmed,s.autoFillStopped,
        s.f3Pending,s.f3Quarantined,now,s.fillStableAt,s.autoFillNext)){
        Reader fill;fill.cap=16384;cfgobserver::Plan local;cfgprimary::HostedBinding binding;
        cfghosted::listen35::Context owner;cfghosted::native::Population population;
        const cfghosted::native::Admission admission{plan.module,engine,viewport,world,plan.threadId,true,true};
        if(cfghosted::listen35::Capture(fill,admission,GetCurrentThreadId(),owner)&&
            cfghosted::native::PopulationBefore<Reader,cfghosted::listen35::Bindings,true>(fill,owner.session,GetCurrentThreadId(),population)&&
#if CFG_H252_RESERVE_BY_OPTION
            // H2.52b: host-as-Jason arms only after the lottery + HUD proof, so the fill ran with armed=0 and
            // reached 8 (live: "LOBBY FULL - leave 1 slot", RosterFull). Reserve whenever the AI Jason option is on.
            cfghostedfill::CounselorRoom(population.list.count,s.armed||s.options.values.jasonCount!=0,s.adopted)&&population.bots<7&&
#else
            cfghostedfill::CounselorRoom(population.list.count,s.armed,s.adopted)&&population.bots<7&&
#endif
            cfgprimary::BuildHostedLocalPlan(fill,owner,local,binding)){
            s.f3Pending=1;s.f3Automatic=true;s.f3World=world;s.f3At=now;s.f3ActorHint=local.pawn;s.f3Next=now;
            ++s.autoFillQueued;hostedLobbyReport.f3Queued=s.f3Pending;
        }
    }
    next.fillDeferredQueued=s.autoFillQueued;next.fillDeferredAdded=s.autoFillAdded;next.fillDeferredRefused=s.autoFillRefused;
#endif
    next.takeoverAttempts=s.attempts;next.takeoverOutcome=s.outcome;next.takeoverStage=s.stage;next.takeoverReason=s.reason;
    next.takeoverRollbackVerified=s.rollbackVerified;next.takeoverEffects=s.effects;next.adoptionPublished=hostedAdoptionActive?1u:0u;
    next.adoptionRefusal=s.adoptionRefusal;next.adoptionStage5Reserved=hostedInitClause;next.aiController=s.aiController;next.takeoverTick=s.takeoverTick;next.armRefusal=s.armRefusal;
    next.adoptionStage5Clause=s.adoptionStage5Clause;next.adoptionStage5Reads=s.adoptionStage5Reads;next.adoptionStage5Cap=s.adoptionStage5Cap;
    next.adoptionStage5PawnClass=s.adoptionStage5PawnClass;next.adoptionStage5KillerClass=s.adoptionStage5KillerClass;
    next.adoptionStage5PathClass=s.adoptionStage5PathClass;next.adoptionStage5Path=s.adoptionStage5PathSlot.object;
    next.adoptionStage5PawnIndex=s.adoptionStage5PawnSlot.index;next.adoptionStage5PawnSerial=s.adoptionStage5PawnSlot.serial;
    next.adoptionStage5ControllerIndex=s.adoptionStage5ControllerSlot.index;next.adoptionStage5ControllerSerial=s.adoptionStage5ControllerSlot.serial;
    next.adoptionStage5PathIndex=s.adoptionStage5PathSlot.index;next.adoptionStage5PathSerial=s.adoptionStage5PathSlot.serial;
#endif
#ifdef CFG_H1_JASON
    next.outro=s.outro;
    next.classRestorePending=hostedClassRestore.pending;next.classRestoreAttempts=hostedClassRestore.attempts;
    next.classRestored=hostedClassRestore.restored;next.classRestoreRefused=hostedClassRestore.refused;next.classRestoreChanged=hostedClassRestore.changed;
    next.classRestoreLastRefusal=hostedClassRestore.lastRefusal;
    next.localAdoptionPending=s.adoptionPending;next.localAdoptionAttempts=s.adoptionInitAttempts;
    next.presentationPrimaryAttempted=s.presentationPrimaryAttempted;next.presentationPrimaryComplete=s.presentationPrimaryComplete;
    next.presentationExtraPending=s.presentationExtraPending;next.presentationExtraAttempted=s.presentationExtraAttempted;
    next.presentationExtraComplete=s.presentationExtraComplete;next.presentationRefusal=s.presentationRefusal;next.presentationCopyRefusal=s.presentationCopyRefusal;next.presentationCopyBundleNativeStage=s.presentationCopyBundleNativeStage;
    next.presentationNotAttemptedReason=s.presentationNotAttemptedReason;next.presentationNotAttemptedWindows=s.presentationNotAttemptedWindows;
    next.extraRequestIssued=hostedJasonExtraCountConsumed;next.extraRequestAccepted=s.extraRequestAccepted;next.storedJasonCount=s.options.values.jasonCount;
    next.f1InputEdges=s.f1InputEdges;next.f1RequestAccepted=s.f1RequestAccepted;next.f1RequestRefused=s.f1RequestRefused;
    next.additionalJasonStage=static_cast<unsigned>(bundleAdditional.policy.stage);next.additionalJasonRequests=bundleAdditional.policy.requests;
    next.additionalJasonBirths=bundleAdditional.policy.births;next.additionalJasonFailures=bundleAdditional.policy.failures;
    next.f1SelectionRefusal=bundleAdditional.selectionRefusal;next.f1SelectionIndex=bundleAdditional.selection;
    next.f1SelectionConversionFailures=bundleAdditional.selectionConversionFailures;
    next.f1SelectionTemplateFailures=bundleAdditional.selectionTemplateFailures;
    next.counselorPreferencePreflight=hostedTx.counselorPreferencePreflight;
    next.counselorPreferencePreflightReason=hostedTx.counselorPreferencePreflightReason;
    next.counselorPreferencePreflightKind=hostedTx.counselorPreferencePreflightKind;
    next.counselorPreferencePreflightIndex=hostedTx.counselorPreferencePreflightIndex;
    next.counselorPreferenceHandoff=hostedTx.counselorPreferenceHandoff;
    next.counselorPreferenceHandoffReason=hostedTx.counselorPreferenceHandoffReason;
    next.counselorPreferenceHandoffKind=hostedTx.counselorPreferenceHandoffKind;
    next.counselorPreferenceHandoffIndex=hostedTx.counselorPreferenceHandoffIndex;
#if CFG_H1_ROLE_SELECTION || CFG_H245_HOST_JASON
    next.rolePreferenceInstalled=hostedRoleSelectionHookInstalled?1u:0u;
#endif
    next.rolePreferenceAttempts=s.rolePreferenceAttempts;
    next.rolePreferenceApplied=s.rolePreferenceApplied?1u:0u;next.rolePreferenceRefused=s.rolePreferenceRefused;next.rolePreferenceRefusal=s.rolePreferenceRefusal;
    next.rolePreferenceRemoteCount=s.rolePreferenceRemoteCount;next.rolePreferenceHostBefore=s.rolePreferenceHostBefore;
    next.rolePreferenceHostAfter=s.rolePreferenceHostAfter;next.rolePreferenceWinnerLocal=s.rolePreferenceWinnerLocal;
    next.h245PinResult=s.h245PinMask;next.h245WritesApplied=s.h245WritesApplied;next.h245HostWon=s.h245HostWon;next.h245RefusalCode=s.h245Refusal;
    next.h248BridgeCalls=s.h248BridgeCalls;next.h248BridgeLastGate=s.h248BridgeLastGate;next.h248BridgeInstallAt=s.h248BridgeInstallAt;next.h248BridgeFirstCallAt=s.h248BridgeFirstCallAt;
    next.h250ViewFailure=hostedViewRepair.code?hostedViewRepair.code:s.h250ViewFailure;
    if(bundleCarRefuseLine)next.carManagerCap=bundleCarRefuseLine;
    next.presentationSkinBefore=s.presentationSkinBefore;next.presentationSkinAfter=s.presentationSkin;
    next.presentationWeaponBefore=s.presentationWeaponBefore;next.presentationWeaponAfter=s.presentationWeapon;
    next.primaryPresentationSkinBefore=s.primaryPresentationSkinBefore;next.primaryPresentationSkinAfter=s.primaryPresentationSkin;
    next.primaryPresentationWeaponBefore=s.primaryPresentationWeaponBefore;next.primaryPresentationWeaponAfter=s.primaryPresentationWeapon;
    next.extraPresentationSkinBefore=s.extraPresentationSkinBefore;next.extraPresentationSkinAfter=s.extraPresentationSkin;
    next.extraPresentationWeaponBefore=s.extraPresentationWeaponBefore;next.extraPresentationWeaponAfter=s.extraPresentationWeapon;
    next.lanes=hostedLanes;next.bots=hostedBots;next.rosterSamples=hostedRosterSamples;next.rosterRefused=hostedRosterRefused;
    next.rosterReason=hostedRosterReason;next.offlineRetirements=hostedOfflineRetirements;
    next.driver=hostedDriverTrace;next.driverOwnerChecks=hostedDriverTrace.ownerChecks;next.driverOwnerRefusals=hostedDriverTrace.ownerRefusals;
#endif
    HostedH1Publish(next);
}
#endif
