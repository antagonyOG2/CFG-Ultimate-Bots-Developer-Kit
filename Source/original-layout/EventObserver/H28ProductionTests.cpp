#define H28_PRODUCTION_EXTRA 1
#include "H27ProductionTests.cpp"
unsigned H28ProductionExtra(){unsigned checks=0,failures=0;const auto check=[&](bool okay){++checks;if(!okay){++failures;std::printf("H28 failed check %u\\n",checks);}};
 const auto before=hostedH1;const auto saved=plan;const auto binding=cfgprimary::CurrentHostedBinding();
 const auto restore=hostedClassRestore;const auto active=hostedAdoptionActive;
 const auto native=gcRegistryReport.calls;const auto thread=observerOwnerThread;
 observerOwnerThread=GetCurrentThreadId();plan.threadId=observerOwnerThread;
 hostedClassRestore={};
 hostedH1={};hostedH1.planSwapped=true;hostedH1.savedPlan=plan;hostedH1.adoptionPending=true;
 hostedH1.adoptionPlan=plan;hostedH1.adoptionPlan.pawn=0xABC000;hostedH1.adoptionBinding.active=true;hostedH1.adoptionBinding.plan=hostedH1.adoptionPlan;
 const auto old=plan.pawn;check(!HostedH1EnterPlan(0xABC000)&&plan.pawn==old); // not installed under old scope
 HostedH1LeavePlan();check(plan.pawn==0xABC000&&hostedH1.kept&&hostedH1.adopted&&cfgprimary::CurrentHostedBinding().Matches(plan));
 check(!HostedH1EnterPlan(old)&&HostedH1EnterPlan(0xABC000)); // only the pending activation callback is admitted
 hostedH1.adoptionPending=false;
 check(!HostedH1EnterPlan(0xABC000)); // stable adoption does not open a broad callback window
 Reader r;r.cap=4096;check(!cfgprimary::LookupSafe(r,plan.module)); // routing never supplies native authority
 BundleAccess access;hostedClassRestore={};hostedClassRestore.pending=true;hostedClassRestore.due=true;hostedClassRestore.world=plan.world;hostedClassRestore.mode=plan.mode;
 check(!access.HostedRestoreClassAtEnd()&&hostedClassRestore.pending&&gcRegistryReport.calls==native);
 ++hostedClassRestore.world;check(!access.HostedRestoreClassAtEnd()&&gcRegistryReport.calls==native);
 // Own-value comparison tolerates native weak-cache refresh, never a new class.
 cfgending::Packet original{},expected{},current{};original[16]=1;expected[16]=2;current=expected;current[0]=123;
 check(cfgending::SamePath(current,expected)&&!cfgending::SamePath(current,original));current[16]=3;check(!cfgending::SamePath(current,expected));
 hostedClassRestore=restore;hostedH1=before;plan=saved;cfgprimary::CurrentHostedBinding()=binding;hostedAdoptionActive=active;observerOwnerThread=thread;
 std::printf("H28 actual host-plan retirement/rebind and class-restore refusal: %u checks, %u failures; no authority transfer or native calls.\n",checks,failures);return failures;
}
