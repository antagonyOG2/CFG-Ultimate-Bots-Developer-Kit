#define CFG_SPAWN_ADAPTER
#define CFG_JASON_APPROACH
#define CFG_PRIMARY_JASON
#define CFG_GAMEPLAY_BUNDLE
#define CFG_FRONTEND_BUNDLE
#define CFG_RETAINED_PRIMARY_JASON
#define CFG_RETAINED_GAMEPLAY_BUNDLE
#define CFG35_MENU_RETENTION_TEST
#include "OfflineEventObserver.cpp"
#include <cstdio>
int main(){unsigned failures=0;
    const auto check=[&](bool okay){if(!okay)++failures;};
    plan.module=0x100000;plan.threadId=GetCurrentThreadId();observerOwnerThread=plan.threadId;
    cfgmenu::hud::Binding binding{};binding.context.module=plan.module;binding.context.thread=plan.threadId;
    binding.context.hud=0x20000;binding.event=0x30000;
    // Real provider rejects wrong provenance BEFORE engine reads, registration
    // or dispatch. This executable has no engine pages at these addresses.
    for(unsigned fault=0;fault<6;++fault){
        FrontendMenuRootProvider provider(binding,fault==0?0x40000:0x20000,
            fault==1?0x50000:0x30000,plan.module+cfgmenu::transportdisk::ReturnPc+(fault==2?1u:0u),
            fault==3,fault!=4);
        if(fault==5)continue; // matching origin still requires real context/metadata
        check(!provider.Admitted());
        cfgmenu::rows::NativeRetentionScope scope(plan.module,provider.Hooks(),provider.Admitted());
        check(!scope.Entered()&&!cfgmenu::rows::NativeRetentionScope::Current(plan.module));
    }
    check(!frontendRootBank.Owners()&&!frontendRootBank.Poisoned()&&!gcRegistryReport.calls);
    // Post-match objective restore window: the real provider's metadata-only
    // window must not run work or open a GC registry window without admission.
    for(unsigned fault=0;fault<5;++fault){
        FrontendMenuRootProvider provider(binding,fault==0?0x40000:0x20000,
            fault==1?0x50000:0x30000,plan.module+cfgmenu::transportdisk::ReturnPc+(fault==2?1u:0u),
            fault==3,fault!=4);
        bool ran=false;Reader phase;phase.cap=cfgbundle::ReadCap;
        check(!provider.MetadataWindow<cfgobjective::Budget<Reader>>(phase,[&](Reader&)noexcept{ran=true;return true;})&&!ran);
        check(!provider.MetadataContext(phase)&&!phase.reads); // unadmitted: refused before any read
        check(!cfgprimary::LookupSafe(*std::make_unique<Reader>(),plan.module));
    }
    check(!gcRegistryReport.calls&&!frontendRootBank.Owners());
    // The retirement opportunity is inert until the epoch binds the menu HUD,
    // then refuses non-HUD, nested, uncompleted and foreign-thread callbacks.
    bundleObjectiveRetiring=true;bundleObjectiveThread=plan.threadId;
    FrontendObjectiveRetirementAfter(reinterpret_cast<void*>(0x20000),reinterpret_cast<void*>(0x30000),false,true,
        plan.module+cfgmenu::transportdisk::ReturnPc);
    check(!frontendRetireStepAt&&!gcRegistryReport.calls); // unbound: zero work
    FrontendObjectiveRetirementBind(binding);
    FrontendObjectiveRetirementAfter(reinterpret_cast<void*>(0x40000),reinterpret_cast<void*>(0x30000),false,true,
        plan.module+cfgmenu::transportdisk::ReturnPc);
    check(!frontendRetireStepAt&&!gcRegistryReport.calls); // other actor: pointer compare only
    FrontendObjectiveRetirementAfter(reinterpret_cast<void*>(0x20000),reinterpret_cast<void*>(0x30000),false,true,
        plan.module+cfgmenu::transportdisk::ReturnPc);
    FrontendObjectiveRetirementAfter(reinterpret_cast<void*>(0x20000),reinterpret_cast<void*>(0x30000),true,true,0);
    FrontendObjectiveRetirementAfter(reinterpret_cast<void*>(0x20000),reinterpret_cast<void*>(0x30000),false,false,0);
    bundleObjectiveThread=plan.threadId+1;
    FrontendObjectiveRetirementAfter(reinterpret_cast<void*>(0x20000),reinterpret_cast<void*>(0x30000),false,true,0);
    bundleObjectiveRetiring=false;bundleObjectiveThread=0;
    // Nothing owed: the bound HUD callback clears the binding, never retires.
    check(!frontendRetireEvent&&!frontendRetireStepAt&&!gcRegistryReport.calls&&!bundleObjectives.Status().restored);
    check(!(bundleReport.generation&1u)); // no publication left open
    FrontendMenuRootProvider noContext(binding,0x20000,0x30000,
        plan.module+cfgmenu::transportdisk::ReturnPc,false,true);
    check(!noContext.Admitted()&&!frontendRootBank.Owners()&&!gcRegistryReport.calls);
    // A staged menu candidate cannot perform native birth/restart merely
    // because exact image/thread metadata was supplied.
    cfgmenu::promotion::WindowsCalls calls(plan.module,plan.threadId,true);
    cfgmenu::promotion::Context context{};context.module=plan.module;context.thread=plan.threadId;context.exactImageVerified=true;
    check(!calls.Ready(context));
    // The next UI stage allows original Accept/map/settings, never native
    // match Start, including the graph alias and unclaimed Jason route.
    std::array<unsigned char,cfgmenu::startdisk::FrameLocals+8> startFrame{};
    std::int32_t startIndex=cfgmenu::startdisk::StartEntry;
    const auto startLocals=reinterpret_cast<std::uintptr_t>(&startIndex);
    std::memcpy(startFrame.data()+cfgmenu::startdisk::FrameLocals,&startLocals,sizeof(startLocals));
    const auto previousState=frontendReport.state;
    for(const bool claimed:{false,true}){
        frontendCounselorClaim=claimed;
        FrontendStart(nullptr,startFrame.data(),nullptr);
        FrontendGraph(nullptr,startFrame.data(),nullptr);
        check(frontendReport.state==previousState&&!frontendLaunch&&!frontendRootBank.Owners()&&!gcRegistryReport.calls);
    }
    frontendCounselorClaim=false;
    // PID6772: the native picker Construct is not a completed HUD callback.
    // Its normal owner getter MUST NOT silently borrow that capability. This
    // pins the observed refusal while the dedicated compatibility path is
    // proved separately; no real engine data/calls are needed for this guard.
    const auto currentModule=reinterpret_cast<std::uintptr_t>(GetModuleHandleW(nullptr));
    cfgmenu::rows::NativeDispatch outsideHud(currentModule,GetCurrentThreadId(),true);
    check(!outsideHud.BeginPhase()&&!outsideHud.Calls()&&!outsideHud.Faulted());
    frontendSource.module=currentModule;
    frontendEngine=std::make_unique<cfgmenu::rows::NativeDispatch>(currentModule,GetCurrentThreadId(),true);
    FrontendPickerHost pickerHost;cfgmenu::pickerprepare::Receiver receiver;
    check(!pickerHost.AdmitConstruct({}, {}, {},receiver)&&!receiver.widget&&!frontendEngine->Calls());
    cfgmenu::pickerprepare::Prepared prepared;cfgmenu::active::Context borrowed;
    borrowed.module=currentModule;borrowed.thread=GetCurrentThreadId();
    const auto registrations=gcRegistryReport.calls;
    FrontendMenuRootProvider foreignConstruct(borrowed,prepared,{},0x20000,0x30000,
        currentModule+cfgmenu::constructdisk::Return+1);
    check(!foreignConstruct.Admitted()&&gcRegistryReport.calls==registrations&&!frontendRootBank.Poisoned());
    cfgmenu::startbinding::Binding invalidStart;invalidStart.context.module=currentModule;invalidStart.context.thread=plan.threadId;
    FrontendStartRootProvider wrongStart(invalidStart,{},currentModule+cfgmenu::constructdisk::Return+1,0x20000,0x30000);
    check(!wrongStart.Admitted()&&gcRegistryReport.calls==registrations&&!frontendRootBank.Poisoned());
    // Exercise the actual resident epoch archive past its old64-match bound.
    // These owned dispatch objects are inert; no native/game call is made.
    auto previousEpochArchive=std::move(frontendOldEpochs);
    std::array<cfgmenu::rows::NativeDispatch*,80> retained{};
    for(unsigned i=0;i<retained.size();++i){
        ReserveFrontendEpochArchive();
        auto storage=std::make_unique<FrontendEpochStorage>();
        storage->engine=std::make_unique<cfgmenu::rows::NativeDispatch>(currentModule,GetCurrentThreadId(),false);
        retained[i]=storage->engine.get();frontendOldEpochs.push_back(std::move(storage));
        for(unsigned j=0;j<=i;++j)check(frontendOldEpochs[j]->engine.get()==retained[j]);
    }
    check(frontendOldEpochs.size()==80);
    frontendOldEpochs=std::move(previousEpochArchive);
    std::printf("Actual retained frontend provenance/no-scope/no-startup refusal: %u failures;80 resident epochs retain addresses; own process only\n",failures);
    return failures?1:0;
}
