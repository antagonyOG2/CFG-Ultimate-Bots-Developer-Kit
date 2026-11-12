#define main OriginalUrinationProductionMain
#include "UrinationNativeProductionTests.cpp"
#undef main
#ifndef CFG_WORLDROOT_PRODUCTION_MAIN
#define CFG_WORLDROOT_PRODUCTION_MAIN main
#endif
int CFG_WORLDROOT_PRODUCTION_MAIN(){const auto prior=OriginalUrinationProductionMain();unsigned failures=0;
 const auto check=[&](bool okay){if(!okay)++failures;};const auto calls=gcRegistryReport.calls;
 const auto previous=bundlePreviousWorld;const auto retirement=worldRootRetirement;
 bundlePreviousWorld={};worldRootRetirement={};check(FrontendPreviousWorldAllowsLoad());
 bundlePreviousWorld.recordAttempted=true;check(!FrontendPreviousWorldAllowsLoad());
 worldRootRetirement={};
 bundlePreviousWorld.world.object=0x100000;bundlePreviousWorld.world.index=1;bundlePreviousWorld.generation=1;
 check(!FrontendPreviousWorldAllowsLoad());worldRootRetirement.previous=bundlePreviousWorld;
 worldRootRetirement.stage=cfgretain::WorldRetirementStage::Armed;check(!FrontendPreviousWorldAllowsLoad());
 FrontendWorldRetirementAfter(nullptr,nullptr,false,true,0);
 check(worldRootRetirement.stage==cfgretain::WorldRetirementStage::Armed&&gcRegistryReport.calls==calls);
 worldRootRetirement.stage=cfgretain::WorldRetirementStage::Ready;check(FrontendPreviousWorldAllowsLoad());
 check(worldRootRetirement.beforeChecks==1&&frontendReport.worldRootBeforeLoadChecks==1); // diagnostic did not veto Ready
 WorldRootDiagnostic(true);check(frontendReport.worldRootAfterLoadChecks==1);
 const auto savedMatch=bundlePreviousWorld;cfgworlddiag::BeginRecord(bundlePreviousWorld);
 check(!FrontendPreviousWorldAllowsLoad());bundlePreviousWorld=savedMatch;
 // A new match at the exact same address/index still needs its own retirement.
 ++bundlePreviousWorld.generation;check(!FrontendPreviousWorldAllowsLoad());
 worldRootRetirement.previous=bundlePreviousWorld;worldRootRetirement.stage=cfgretain::WorldRetirementStage::Refused;
 worldRootRetirement.failureName="startupRootBank";worldRootRetirement.failureOwner=0x800000;WorldRootRefuse();
 check(!FrontendPreviousWorldAllowsLoad());
 check(!FrontendPreviousWorldAllowsLoad()&&frontendReport.worldRootRefusals==1&&worldRootRetirement.notice&&
  frontendReport.worldRootFailureOwner==0x800000&&!std::strcmp(frontendReport.worldRootFailureName.data(),"startupRootBank"));
 frontendReport={};SyncWorldRootReport();check(frontendReport.worldRootRefusals==1&&frontendReport.worldRootFailureOwner==0x800000);
 worldRootRetirement.stage=cfgretain::WorldRetirementStage::Armed;
 FrontendWorldRetirementAfter(nullptr,nullptr,true,true,0);FrontendWorldRetirementAfter(nullptr,nullptr,false,false,0);
 check(gcRegistryReport.calls==calls&&!bundleRootLease&&startupRootBank.Healthy()&&frontendRootBank.Healthy()&&bundleRootBank.Healthy());
 bundlePreviousWorld=previous;worldRootRetirement=retirement;
 std::printf("Actual B2 production load gate: %u failures; verified retirement allows load despite uncollected World; before/after diagnostics and named terminal refusal published; stale Ready invalidated on new record. No native work in unknown context.\n",failures);
 return prior||failures?1:0;
}
