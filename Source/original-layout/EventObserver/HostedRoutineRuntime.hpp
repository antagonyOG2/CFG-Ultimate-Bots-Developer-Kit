#ifndef CFG_H216_HOSTED_SPECTATOR
#define CFG_H216_HOSTED_SPECTATOR 0
#endif
#if CFG_H216_HOSTED_SPECTATOR && defined(CFG_H1_JASON)
#include "HostedSpectatorHookCalls.hpp"
bool PrepareHostedSpectatorHooks() noexcept;
void RetireHostedSpectatorHooks() noexcept;
#endif
#ifdef CFG_H1_JASON
// Runs only inside the existing returned host callback/root lease, after complete
// adoption publication. No offline spawn result/configuration/authority is forged.
__declspec(noinline) bool InitializeHostedRoutine(std::uint64_t now)noexcept{
    hostedInitClause=0;
#define CFG_H246_CLAUSE(n,expr) do{if(!(expr)){if(CFG_H246_FULL_LOBBY)hostedInitClause=(n);return false;}}while(0)
    CFG_H246_CLAUSE(1,HostedAdoptionOwnsDriver()&&bundleRootLease);
    ResetBundleWorldState(now);hostedRoutineStarted=true;
    struct Rollback {bool committed=false;~Rollback(){if(!committed)hostedRoutineStarted=false;}} rollback;
    BundleAccess access(8192);
    if(!access.HostedContext(false)){hostedRoutineStarted=false;if(CFG_H246_FULL_LOBBY)hostedInitClause=2;return false;}
    if(!access.Jason()){hostedRoutineStarted=false;if(CFG_H246_FULL_LOBBY)hostedInitClause=3;return false;}
    if(!access.NativeBindings()){hostedRoutineStarted=false;if(CFG_H246_FULL_LOBBY)hostedInitClause=4;return false;}
    if(!access.Manager()){hostedRoutineStarted=false;if(CFG_H246_FULL_LOBBY)hostedInitClause=5;return false;}
    Reader mapInputs;mapInputs.cap=512;std::uintptr_t tree=0;
    CFG_H246_CLAUSE(6,mapInputs.Read(plan.mode+cfghosted::birth35::CounselorBehaviorTree,tree));
    spawnAssets.primary.counselorTree=tree;
    CFG_H246_CLAUSE(7,!tree||cfgspawn::ObjectSlot(mapInputs,plan.objectArray,tree,bundleTreeSlot));
    bundleReport.jason=bundleJason.pawn.object;bundleReport.target=plan.pawn;bundleHumanTarget=plan.objects[3];
    bundleKillOriginal=reinterpret_cast<cfgkill::EligibilityOriginal>(plan.module+cfgkill::disk::Eligibility);
    bundleReport.initialJasonPawnState=hostedAdoptionReceipt.controllerState;
    bundleReport.initialJasonControllerState=hostedAdoptionReceipt.controllerState;
    bundleReport.initialRegisteredKillerState=hostedAdoptionReceipt.controllerState;
    bundleReport.initialKillerStateReadMask=7;
    // Native Hunt already owns this exact killer tuple; no offline metadata setters.
    bundleEnding.nativeOwned=true;
    Reader map;map.cap=256;cfgobserver::Plan fresh;std::uintptr_t package=0,packageType=0,type=0,outer=1;
    cfgnames::Text name;cfgnames::Observation<Reader> names(map,plan.module);
    bundleTrapFamily=cfgtrap::MapFamily::Unknown;bundleTrapMapWorld=0;bundleTrapMapPackage={};
#if CFG_H293_HOSTED_TRAP_RESET
    // H2.93: a finished offline match never runs the H2.75 leftover-epoch reset, so its startup-trap progress and discovered
    // trap sources (offline-world actors) survived into this hosted world: live, the hosted traps lane never entered, sources
    // still "prepared=12" with the offline morph coordinates. Clear them exactly as the epoch reset does; Start follows below.
    bundleTraps={};bundleTrapSources={};bundleTrapInputs={};
#endif
    if(cfgprimary::RetainedWorld(map,plan,fresh)&&map.Read(plan.world+0x20,package)&&package&&
       map.Read(plan.module+cfgmenu::worldmapdisk::NativePackageCache,packageType)&&packageType&&
       map.Read(package+cfgprobe::ObjectClass,type)&&type==packageType&&map.Read(package+0x20,outer)&&!outer&&
       names.Object(package,name)&&cfgobserver::Same(map,plan.world+0x20,package)&&
       cfgjason::SameSlot(map,fresh.objectArray,plan.objects[0])){
        bundleTrapFamily=cfgtrap::startup::MapFamilyForArrival(name.View(),plan.world,plan.world);
        bundleTrapMapWorld=plan.world;bundleTrapMapPackage=name;
    }
    CFG_H246_CLAUSE(8,bundleSchedule.Start(now,0,0));
    bundleSchedule.helpersActive=false;bundleReport.requestedBots=0;
    // Hosted births use Hunt RestartPlayer, not the offline helper pipeline.
    // Its default Project stage incorrectly blocked roof/radio/knife work forever.
    bundleHelpers={};CFG_H246_CLAUSE(9,bundleHelpers.AdoptNative(0,0));
#undef CFG_H246_CLAUSE
    bundleReport.state=static_cast<unsigned>(cfgbundle::State::Active);
    bundleReport.initializationStage=static_cast<unsigned>(cfgbundle::InitializationStep::Ready);
    bundleTrapInputs={plan.module,plan.world,plan.mode,spawnAssets.gameState,bundleJason.pawn.object,bundleJason.killerController.object,
        bundleJason.pawn,bundleJason.killerController,bundleTrapFamily,bundleTrapMapWorld==plan.world,
                cfgtrap::startup::NoCarFallbackMap(bundleTrapFamily,bundleTrapMapWorld==plan.world,bundleTrapMapPackage.count>0)};
    bundleTraps.Start(now,cfgtrap::startup::EffectiveFamily(bundleTrapFamily,bundleTrapInputs.fallbackMap),bundleTrapInputs.verifiedMap);
    if(CFG_J214_TRAP_PRIORITY)(void)PrepareBundleTrapHook(plan.module);
    if(bundleTraps.Busy())++hostedDriverTrace.startupQueued;
    bundleReport.trapMapFamily=static_cast<unsigned>(bundleTrapFamily);
    for(unsigned i=0;i<name.count&&i+1<bundleReport.trapMapPackage.size();++i)bundleReport.trapMapPackage[i]=static_cast<char>(name.data[i]);
    hostedRoutineRosterAt=hostedBrainAt=0;hostedBots={};hostedLanes={};hostedBotCursor=hostedRosterSamples=hostedRosterRefused=hostedRosterReason=0;
    hostedH1Report.hidingRosterProof=0;
#if CFG_H216_HOSTED_SPECTATOR && defined(CFG_H1_JASON)
    (void)PrepareHostedSpectatorHooks(); // Optional, default-off; failure never blocks hosted AI.
#endif
    // Spectator status is delivered by UserWidget ProcessEvent, not the actor
    // dispatcher. The shared J2.19 widget observer must be installed in Hunt
    // as well as offline startup for its existing label consumer to run.
    bundleReport.spectatorDispatcherReady=PrepareBundleWidgetDispatcher(plan.module)?1u:0u;
    if(bundleReport.spectatorDispatcherReady){
        (void)access.HostedSpectatorLabelHint();
        hostedSpectatorHintRetryAt=now+1000;
    }
    rollback.committed=true;return true;
}
void RetireHostedRoutine()noexcept{
    // Native match travel retires weak state only. Active claims are scoped and
    // released by the existing owner lease; never call old World objects here.
    hostedRoutineStarted=false;hostedRoutineRosterAt=hostedBrainAt=hostedSpectatorHintRetryAt=0;
    hostedNativeDeathMotion=false;hostedNativeDeathMotionCheckAt=0;
#if CFG_H216_HOSTED_SPECTATOR && defined(CFG_H1_JASON)
    RetireHostedSpectatorHooks();
#endif
}
#endif
