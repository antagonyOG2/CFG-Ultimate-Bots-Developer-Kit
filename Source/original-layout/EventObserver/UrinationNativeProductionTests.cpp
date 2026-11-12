#define CFG35_PRIMARY_GAMEPLAY_TEST
#define CFG35_KNIFE_BOUNDARY_OBSERVE
#define CFG35_KNIFE_RELEASE_ADAPTER
#define CFG35_URINATION_NATIVE_PORT
#define CFG_URINATION_PRODUCTION_ONLY
#include "RetainedCounselorStartupProductionTests.cpp"
int main(){unsigned failures=0;const auto check=[&](bool okay){if(!okay)++failures;};
    plan.module=reinterpret_cast<std::uintptr_t>(GetModuleHandleW(nullptr));plan.threadId=GetCurrentThreadId();
    observerOwnerThread=plan.threadId;bundleStarted=true;bundleSchedule.active=true;
    cfgurination::LoadIdentity fake{0x100000,0x200000,1,2,plan.threadId,0x18E030,1};
    const auto before=gcRegistryReport.calls;
    check(!urinationNativeHost.Configure());check(!urinationNativeHost.Install());
    check(!urinationNativeHost.Admitted()&&!urinationNativeHost.Discover());
    check(!urinationNativeHost.LoaderFunction(false)&&!urinationNativeHost.LoaderFunction(true));
    check(!urinationNativeHost.MakePath()&&!urinationNativeHost.RequestLoad());
    check(!urinationNativeHost.PublishLoaded()&&!urinationNativeHost.Publication().Attempted());
    check(!BundleUrinationService(100)&&urinationRuntimePhases.Stage()==cfgurination::RuntimeStage::Discovery);
    BundleUrinationSelection(nullptr,nullptr,nullptr,false);
    check(!urinationNativeHost.Playback(100)&&!urinationNativeHost.RetireFailure());
    check(!urinationNativeHost.CurrentResidentAssets()&&!urinationNativeHost.ResidentLoaded(0x500000));
    check(!urinationNativeHost.RestoreFailedSelection());
    BundleUrinationSelectionAfter(nullptr,nullptr,false,false);
    check(!urinationNativeHost.PinnedBuild(plan.module));check(!urinationNativeHost.Current(fake));
    check(!urinationNativeHost.MakePath(0x100000,0x200000)&&!urinationNativeHost.RequestLoad(0x100000,0x200000));
    check(!urinationNativeHost.Prepare({fake.world,fake.controller,0x300000,0x400000,0},100));
    check(!urinationNativeHost.Inputs(fake,{fake.world,fake.controller,0x300000,0x400000,0}));
    check(!urinationNativeHost.Loaded(fake,0x500000));check(!urinationNativeHost.BeginRequest());
    std::uintptr_t resolved=123;check(!urinationNativeHost.ResolveLoaded(resolved)&&!resolved);
    check(!urinationNativeHost.RetireRequestInputs()&&!urinationNativeHost.RetireRejected());
    urinationNativeHost.Constructed(0x600000,0x600000,0x700000,0x800000,plan.module+cfgurination::disk::ConstructorReturn);
    urinationNativeHost.Completing(0x600118,0x500000,plan.module+cfgurination::disk::DelegateReturn);
    urinationNativeHost.Destroying(0x600000);urinationNativeHost.RequestReturned(false);
    check(!urinationNativeHost.NativeWindowOpen()&&!urinationRetainedLoad.Attempted());
    check(!urinationAssetBank.Owners()&&urinationAssetBank.Healthy()&&gcRegistryReport.calls==before);
    check(!bundleRootLease&&bundleRootBank.Healthy());
    check(CfgQueueSpectatorToggle(0)==0&&gcRegistryReport.calls==before&&!bundleSpectatorInput.Take(GetTickCount64(),true));
    // The real ending setter gateway must also refuse this unowned context
    // before retention/native entry. These are current verified RVAs, never
    // callable simply because their disk signatures matched.
    {BundleAccess access;cfgending::Packet packet{};packet[16]=1;
     const auto count=bundleEnding.calls;
     check(!access.EndingNative(cfgending::NativeSetter::Class,&packet));
     check(!access.EndingNative(cfgending::NativeSetter::Active,&packet));
     check(!access.EndingNative(cfgending::NativeSetter::Weapon,&packet));
     check(!access.EndingNative(cfgending::NativeSetter::Skin,reinterpret_cast<void*>(0x500000)));
     check(bundleEnding.calls==count&&!bundleEnding.nativeConsumed&&!bundleEnding.nativeBusy&&
         gcRegistryReport.calls==before&&!bundleRootLease&&bundleRootBank.Healthy());}
    {BundleAccess access;cfgrbemote::Owner owner{};cfgrbemote::ConfirmationEvidence evidence{};std::uintptr_t output=0;
     check(!access.RBEligible(owner,evidence));check(!access.RBInvoke(owner,evidence,0,&output));
     check(gcRegistryReport.calls==before&&!bundleRootLease&&bundleRootBank.Healthy());}
    std::printf("Actual full-source urination owner/native hook admission: %u failures; unknown context refuses before registry/native work, no retained roots/hooks/request. Own process only.\n",failures);
    return failures?1:0;
}
