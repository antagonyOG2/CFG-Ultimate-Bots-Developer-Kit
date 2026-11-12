#define CFG_SPAWN_ADAPTER
#define CFG_JASON_APPROACH
#define CFG_PRIMARY_JASON
#define CFG_GAMEPLAY_BUNDLE
#define CFG_FRONTEND_BUNDLE
#define CFG_RETAINED_PRIMARY_JASON
#define CFG_RETAINED_GAMEPLAY_BUNDLE
#define CFG35_MENU_RETENTION_TEST
#define CFG35_NATIVE_STARTUP_ENTRY_TEST
#define CFG35_NATIVE_STARTUP_ROOT_TEST
#include "OfflineEventObserver.cpp"
#include <cstdio>
unsigned stockIntroForwards=0,stockMatchForwards=0;
void __fastcall StockIntroModel(void*){++stockIntroForwards;}
void __fastcall StockMatchModel(void*){++stockMatchForwards;}
int main(){unsigned failures=0;const auto check=[&](bool ok){if(!ok)++failures;};
    plan.module=reinterpret_cast<std::uintptr_t>(GetModuleHandleW(nullptr));
    plan.threadId=GetCurrentThreadId();observerOwnerThread=plan.threadId;
    nativeOfflineOriginalIntro=&StockIntroModel;nativeOfflineOriginalMatch=&StockMatchModel;
    // Wrong origin/no real engine: observation fails without metadata reads,
    // but the original stock callbacks ALWAYS forward. No mod startup enters.
    NativeOfflineIntro(reinterpret_cast<void*>(0x20000));NativeOfflineMatch(reinterpret_cast<void*>(0x20000));
    check(stockIntroForwards==1&&stockMatchForwards==1);
    for(const auto& entry:frontendRetentionReport.startup)
        check(entry.candidates==1&&!entry.originPassed&&!entry.ownerPassed&&entry.forwards==1&&entry.stage==1&&!entry.reads);
    check(nativeOfflineStartup.Status().stage==cfgmenu::offlinestartup::Stage::Empty&&
        !frontendGameplayReady&&!frontendReport.spawnQueued&&!gcRegistryReport.calls&&!frontendRootBank.Owners());
    // The counselor/mod Start path remains consumed, never forwarded by the
    // stock-Jason-only observation exception (including the graph branch).
    frontendCounselorClaim=true;
    std::array<unsigned char,cfgmenu::startdisk::FrameLocals+8> frame{};
    std::int32_t entry=cfgmenu::startdisk::StartEntry;const auto locals=reinterpret_cast<std::uintptr_t>(&entry);
    std::memcpy(frame.data()+cfgmenu::startdisk::FrameLocals,&locals,sizeof(locals));
    FrontendStart(nullptr,frame.data(),nullptr);FrontendGraph(nullptr,frame.data(),nullptr);
    check(nativeOfflineStartup.Status().stage==cfgmenu::offlinestartup::Stage::Empty&&!frontendLaunch&&!gcRegistryReport.calls);
    cfgmenu::promotion::WindowsCalls calls(plan.module,plan.threadId,true);cfgmenu::promotion::Context context{};
    context.module=plan.module;context.thread=plan.threadId;context.exactImageVerified=true;
    check(!calls.Ready(context)&&!cfgmenu::startupretain::Scope::Occupied());
    check(!startupRootBank.Owners()&&!startupRootBank.Poisoned()&&!nativeStartupRootSession);
    std::printf("Actual startup observer stock forwarding/mod refusal: %u failures; own process only.\n",failures);return failures?1:0;
}
