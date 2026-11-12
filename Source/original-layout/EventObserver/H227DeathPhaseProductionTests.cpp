#define CFG_H1_JASON 1
#define main H227PriorProductionMain
#include "UrinationNativeProductionTests.cpp"
#undef main
int main(){
    unsigned checks=0,failures=0;const auto check=[&](bool v){++checks;if(!v)++failures;};
    plan.module=reinterpret_cast<std::uintptr_t>(GetModuleHandleW(nullptr));plan.threadId=GetCurrentThreadId();
    plan.world=0x100000;plan.mode=0x200000;
    cfgprimary::CurrentHostedBinding().active=true;cfgprimary::CurrentHostedBinding().plan=plan;
    hostedAdoptionActive=true;hostedAdoptionReceipt.valid=true;hostedAdoptionReceipt.possessReturned=true;
    hostedAdoptionReceipt.world=plan.world;hostedAdoptionReceipt.mode=plan.mode;
    hostedAdoptionReceipt.pawn={0x300000,1,10};hostedAdoptionReceipt.controller={0x400000,2,20};
    hostedAdoptionReceipt.controllerState=0x500000;hostedAdoptionReceipt.displacedState=0x600000;
    hostedAdoptionReceipt.botsAfter=1;hostedAdoptionReceipt.rosterAfter=1;
    hostedNativeDeathMotion=true;check(HostedAdoptionOwnsDriver());
    // Fake addresses cannot pass native authority. The actual BundlePhase must
    // still reach its death bookkeeping before suppressing new motion.
    bundleKill.policy.interactionAttempted=true;bundleDeathEnd.next=0;
    BundleAccess death;BundlePhase(death,cfgbundle::Lane::Hunt,1000);
    check(bundleDeathEnd.next==1250&&death.Calls()==0);
    // Existing owned helper cleanup remains reachable in the death-only slice.
    bundleReport.bots=1;bundleConvergenceTravel[0].policy.owned=true;
    const auto lost=bundleTravelLost;BundleAccess cleanup;BundlePhase(cleanup,cfgbundle::Lane::Vehicle,1300);
    check(!bundleConvergenceTravel[0].policy.owned&&bundleTravelLost==lost+1&&cleanup.Calls()==0);
    bundleReport.bots=0;bundleKill.policy.interactionAttempted=false;
    bundleCompletionReady=true;bundleEnding={};bundleEnding.stage=cfgending::Stage::Ready;
    BundleAccess completion;BundlePhase(completion,cfgbundle::Lane::Settings,1600);
    check(!bundleCompletionReady&&bundleReport.completionFailure==1&&completion.Calls()==0);
    RetireHostedRoutine();check(!hostedNativeDeathMotion&&!hostedNativeDeathMotionCheckAt);
    std::printf("Actual hosted death BundlePhase lifecycle: %u checks, %u failures; terminal observations/helper cleanup/completion/retirement, no engine calls.\n",checks,failures);
    return failures?1:0;
}
