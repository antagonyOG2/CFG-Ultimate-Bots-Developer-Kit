#define H27_PRODUCTION_EXTRA 1
#include "H1ProductionTests.cpp"
unsigned H27ProductionExtra(){unsigned checks=0,failures=0;
 const auto check=[&](bool ok){++checks;if(!ok){++failures;std::printf("H27 failure %u\n",checks);}};
 const auto savedPlan=plan;const auto savedBinding=cfgprimary::CurrentHostedBinding();
 const auto savedReceipt=hostedAdoptionReceipt;const auto savedActive=hostedAdoptionActive;
 const auto savedEndFrozen=hostedEndFrozen;
 const auto savedJason=bundleJason;const auto savedSchedule=bundleSchedule;const auto savedStarted=bundleStarted;
 const auto savedService=bundleNativeJasonDonorService;const auto savedTrace=hostedDriverTrace;
 plan.module=0x140000000;plan.world=0x400000;plan.mode=0x500000;plan.pawn=0x600000;plan.controller=0x700000;
 auto& binding=cfgprimary::CurrentHostedBinding();binding.active=true;binding.plan=plan;
 cfghosted::adoption::Receipt receipt;receipt.valid=true;receipt.possessReturned=true;
 receipt.world=plan.world;receipt.mode=plan.mode;receipt.controllerState=0x800000;receipt.displacedState=0x900000;
 receipt.pawn.object=0xA00000;receipt.pawn.serial=11;receipt.pawn.index=11;receipt.controller.object=0xB00000;receipt.controller.serial=12;receipt.controller.index=12;
 receipt.botsBefore=0;receipt.botsAfter=1;receipt.rosterBefore=2;receipt.rosterAfter=3;check(receipt.Complete());
 hostedAdoptionReceipt=receipt;hostedAdoptionActive=true;hostedEndFrozen=false;bundleStarted=false;bundleSchedule={};
 bundleJason.pawn=receipt.pawn;bundleJason.killerController=receipt.controller;
 bundleNativeJasonDonorService=&ServiceDonorNativeHunting;
 cfgjasontick::Owner owner;check(HostedAdoptionOwnsDriver());
 hostedEndFrozen=true;check(!HostedAdoptionOwnsDriver()&&!ResolveNativeJasonTickOwner(receipt.controller.object,owner));hostedEndFrozen=false;
 check(ResolveNativeJasonTickOwner(receipt.controller.object,owner)&&owner.adoption.Complete()&&!bundleSchedule.active&&!bundleStarted);
 // Routing is not permission: actual native owner observation still refuses these synthetic unregistered objects.
 Reader read;read.cap=4096;check(!cfgjasontick::ObserveOwner(read,owner,GetCurrentThreadId()));
 ++plan.world;check(!HostedAdoptionOwnsDriver()&&!ResolveNativeJasonTickOwner(receipt.controller.object,owner));plan=savedPlan;
 plan=binding.plan;binding.Retire();check(!HostedAdoptionOwnsDriver());binding.active=true;binding.plan=plan;
 DonorNativeJasonRuntime state;bool owed=true;
 check(!state.QueueHostedStartup(owed,true)&&owed);state.bound=true;state.birth.adoption=receipt;
 check(!state.QueueHostedStartup(owed,false)&&owed&&!state.startupFinishPending);
 check(state.QueueHostedStartup(owed,true)&&!owed&&state.startupFinishPending&&!state.startup.Active);
 state.startupFinishPending=false;check(!state.QueueHostedStartup(owed,true)&&!state.startupFinishPending);
 hostedDriverTrace.publications=1;const auto retire=hostedDriverTrace.retirements;
 HostedUnpublishAdoption();check(!hostedAdoptionActive&&!HostedAdoptionOwnsDriver()&&hostedDriverTrace.publications==1&&hostedDriverTrace.retirements==retire+1);
 HostedUnpublishAdoption();check(hostedDriverTrace.retirements==retire+1);
 plan=savedPlan;cfgprimary::CurrentHostedBinding()=savedBinding;hostedAdoptionReceipt=savedReceipt;hostedAdoptionActive=savedActive;hostedEndFrozen=savedEndFrozen;
 bundleJason=savedJason;bundleSchedule=savedSchedule;bundleStarted=savedStarted;bundleNativeJasonDonorService=savedService;hostedDriverTrace=savedTrace;
 std::printf("H27 actual hosted routing/startup/retirement: %u checks, %u failures; no native authority fabricated.\n",checks,failures);
 return failures;
}
