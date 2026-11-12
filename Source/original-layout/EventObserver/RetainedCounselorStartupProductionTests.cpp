#define CFG_RETAINED_PRIMARY_JASON
#define CFG_RETAINED_GAMEPLAY_BUNDLE
#define CFG35_MENU_RETENTION_TEST
#define CFG35_NATIVE_COUNSELOR_STARTUP_TEST
#include "OfflineFrontendBundle.cpp"
#include <cstdio>
unsigned originalIntros=0;
void __fastcall StockIntro(void*){++originalIntros;}
unsigned observedKnifeForwards=0,observedStopForwards=0;
void __fastcall StockKnife(void*){++observedKnifeForwards;}
void __fastcall StockKnifeStop(void*,const void*){++observedStopForwards;}
unsigned observedMulticasts=0,observedLaunches=0;
const cfgspawn::Vec3* forwardedVelocity=nullptr;const cfgspawn::Vec3* forwardedLocation=nullptr;
void __fastcall StockKnifeMulticast(void*,void*,const cfgspawn::Vec3* velocity,const cfgspawn::Vec3* location){
    ++observedMulticasts;forwardedVelocity=velocity;forwardedLocation=location;
}
void __fastcall StockKnifeLaunch(void*,const cfgspawn::Vec3*){++observedLaunches;}
#ifndef CFG_URINATION_PRODUCTION_ONLY
int main(){unsigned failures=0;const auto check=[&](bool okay){if(!okay)++failures;};
    plan.module=reinterpret_cast<std::uintptr_t>(GetModuleHandleW(nullptr));
    plan.threadId=GetCurrentThreadId();observerOwnerThread=plan.threadId;
    // Dedicated counselor Start without a prepared owned settings transaction
    // must still consume/refuse; no native forwarding, factories or publication.
    frontendCounselorClaim=true;
    std::array<unsigned char,cfgmenu::startdisk::FrameLocals+8> frame{};
    std::int32_t entry=cfgmenu::startdisk::StartEntry;
    const auto locals=reinterpret_cast<std::uintptr_t>(&entry);
    std::memcpy(frame.data()+cfgmenu::startdisk::FrameLocals,&locals,sizeof(locals));
    FrontendStart(nullptr,frame.data(),nullptr);FrontendGraph(nullptr,frame.data(),nullptr);
    check(!frontendLaunch&&!frontendGameplayReady&&!gcRegistryReport.calls);
    cfgmenu::freshgame::Source source{plan.module,0x100000,0x110000,0x120000,0x130000,0x140000,plan.threadId,true};
    cfgmenu::promotion::Packet counselor{},killer{};counselor.bytes[16]=1;killer.bytes[16]=2;
    check(nativeOfflineStartup.Arm(source,counselor,killer,0,3));
    nativeOfflineOriginalIntro=&StockIntro;
    // No real native caller/engine pages: the armed production coordinator
    // must refuse BEFORE birth or stock fallback; no synthetic root authority.
    NativeOfflineIntro(reinterpret_cast<void*>(0x200000));
    check(nativeOfflineStartup.Status().stage==cfgmenu::offlinestartup::Stage::Refused&&
        nativeOfflineStartup.Status().failure==601&&!originalIntros);
    FrontendArrivalAfter(nullptr,false,true);
    check(!frontendGameplayReady&&!frontendReport.spawnQueued&&!spawnRuntime.Pending());
    check(!startupRootBank.Owners()&&!startupRootBank.Poisoned()&&!nativeStartupRootSession&&!gcRegistryReport.calls);
#ifdef CFG35_PRIMARY_GAMEPLAY_TEST
    unsigned work=0;
    bool untouched=false;
    check(!WithBundleRootWindow([&](GcRegistryAccess&)noexcept{++work;return true;},nullptr,&untouched)&&untouched&&!work);
    check(!WithBundleRootWindow([&](GcRegistryAccess&)noexcept{++work;return true;})&&!work);
    check(!bundleRootLease&&!bundleRootBank.Owners()&&!bundleRootBank.Poisoned());
    check(DrainBundleDeferredRetirement()&&!bundleDeferredRetirement.Claims()&&!gcRegistryReport.calls);
    const auto emptyBefore=bundleRootDiagnostics.emptyRetirements,windowsBefore=bundleRootDiagnostics.attempts;
    {BundleCallbackRootScope empty(plan.module);check(empty.Entered());
     check(!WithBundleRootWindow([&](GcRegistryAccess&)noexcept{++work;return true;}));}
    check(!work&&!bundleRootLease&&bundleRootBank.Healthy()&&bundleRootDiagnostics.emptyRetirements==emptyBefore+1&&bundleRootDiagnostics.attempts==windowsBefore);
    BundleAccess access;
    check(!access.Initialize()&&bundleReport.initializationStage==static_cast<unsigned>(cfgbundle::InitializationStep::Context)&&!access.Calls());
    Reader emoteReader;emoteReader.cap=cfgbundle::ReadCap;
    cfgemoteobservation::PlaybackAccess<Reader,BundleAccess> emotePlayback(emoteReader,plan,access);
    check(!emotePlayback.Invoke(0x200000,0x300000,nullptr,true)&&!access.Calls()&&!gcRegistryReport.calls);
    bool counselorPressed=true;
    check(!BundleCounselorWindowInput(100,true,counselorPressed)&&!counselorPressed&&!access.Calls());
    bundleReport.bots=1;bundleBotNames={};
    check(access.BotNamePhase()&&bundleBotNames.failure==1&&!bundleBotNames.attempts&&!access.Calls()&&!gcRegistryReport.calls);
    bundleReport.bots=0;bundleBotNames={}; // no real world: verified setter never bypasses Context
#ifdef CFG35_KNIFE_BOUNDARY_OBSERVE
    bundleKnifeBoundaryObserveEnabled=true;bundleStarted=true;bundleSchedule.active=true;
    bundleJason.pawn.object=0x200000;bundleKnife.stage=cfgknife::Stage::Notify;
    bundleKnifeNotifyHook.original=reinterpret_cast<void*>(&StockKnife);bundleKnifeStopOriginal=&StockKnifeStop;
    BundleNativeThrowKnife(reinterpret_cast<void*>(bundleJason.pawn.object));
    BundleNativeKnifeStop(reinterpret_cast<void*>(0x300000),nullptr);
    check(observedKnifeForwards==1&&observedStopForwards==1&&bundleReport.knifeBoundaryCount==2&&!bundleReport.knifeBoundaries[0].flags);
    {BundleCallbackRootScope lexical(plan.module);check(lexical.Entered());bundleKnifeReleaseDepth=1;
     BundleNativeThrowKnife(reinterpret_cast<void*>(bundleJason.pawn.object));bundleKnifeReleaseDepth=0;}
    check(observedKnifeForwards==2&&bundleReport.knifeBoundaryCount==3&&bundleReport.knifeBoundaries[2].flags==5);
    const auto recorded=bundleReport.knifeBoundaryCount;BundleNativeThrowKnife(reinterpret_cast<void*>(0x900000));
    check(observedKnifeForwards==3&&bundleReport.knifeBoundaryCount==recorded);
    check(!bundleRootLease&&bundleRootBank.Healthy()&&!gcRegistryReport.calls&&!access.Calls()&&!bundleKnifeNotifyEnabled);
#ifdef CFG35_KNIFE_RELEASE_ADAPTER
    bundleKnifeReleaseAdapterEnabled=true;bundleKnife.stage=cfgknife::Stage::Windup;
    const auto forwards=observedKnifeForwards,stops=observedStopForwards;
    BundleNativeThrowKnife(reinterpret_cast<void*>(bundleJason.pawn.object));
    BundleNativeKnifeStop(reinterpret_cast<void*>(0x300000),nullptr);
    check(observedKnifeForwards==forwards+1&&observedStopForwards==stops+1&&!bundleKnifeReleaseFrame&&
        !bundleRootLease&&bundleRootBank.Healthy()&&!gcRegistryReport.calls&&!access.Calls()&&!bundleKnifeHeldStops);
    bundleKnifeMulticastHook.original=reinterpret_cast<void*>(&StockKnifeMulticast);
    bundleKnifeLaunchHook.original=reinterpret_cast<void*>(&StockKnifeLaunch);
    cfgspawn::Vec3 nativeVelocity{1,2,3},nativeLocation{4,5,6};
    cfgknife::NotifyWindow orphan;orphan.armed=orphan.geometryReady=true;orphan.driver={0x300000,1};
    orphan.jason={bundleJason.pawn.object,1};bundleKnifeNotifyWindow=&orphan;
    BundleNativeKnifeMulticast(reinterpret_cast<void*>(orphan.driver.object),reinterpret_cast<void*>(orphan.jason.object),&nativeVelocity,&nativeLocation);
    BundleNativeKnifeLaunch(reinterpret_cast<void*>(orphan.driver.object),&nativeVelocity);
    check(observedMulticasts==1&&observedLaunches==1&&forwardedVelocity==&nativeVelocity&&forwardedLocation==&nativeLocation&&
        !orphan.geometryUsed&&!orphan.multicast&&!orphan.launched&&!gcRegistryReport.calls);
    bundleKnifeNotifyWindow=nullptr;
    bundleKnifeReleaseAdapterEnabled=false;
#endif
    bundleStarted=false;bundleSchedule.active=false;bundleKnifeBoundaryObserveEnabled=false;
#endif
#endif
    std::printf("Actual counselor startup isolation/wrong-origin refusal: %u failures; own process only.\n",failures);
    return failures?1:0;
}
#endif
