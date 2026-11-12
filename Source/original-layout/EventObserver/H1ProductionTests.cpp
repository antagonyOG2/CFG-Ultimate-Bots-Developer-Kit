// H1 production-source compile + refusal tests: the ENTIRE observer (all flags) compiled with CFG_H1_OBSERVE and
// CFG_H1_JASON, exercising the real hosted service/glue against an OFFLINE-like (non-hosted) process. No hook, no
// native call, no game. Proves: the hosted code is inert unless a proven armed hosted world exists.
#define H1_PRODUCTION_EXTRA 1
#include "J215PresentationProductionTests.cpp"
#ifndef CFG_H1_OBSERVE
#error H1 production tests require CFG_H1_OBSERVE
#endif
#ifndef CFG_H1_JASON
#error H1 production tests require CFG_H1_JASON
#endif
#if CFG_H20_FILL
unsigned h20FillForwarded=0;
void __fastcall H20FillForward(void*){++h20FillForwarded;}
#endif
#ifdef H29_PRODUCTION_EXTRA
unsigned H29ProductionExtra();
#endif
#ifdef H28_PRODUCTION_EXTRA
unsigned H28ProductionExtra();
#endif
#ifdef H27_PRODUCTION_EXTRA
unsigned H27ProductionExtra();
#endif
unsigned H1ProductionExtra(){
    unsigned failures=0,checks=0;
    const auto checkAt=[&](bool ok,int line){++checks;if(!ok){++failures;std::printf("H1 production check failed at line %d\n",line);}};
#define check(expression) checkAt((expression),__LINE__)
    // With role forcing compiled out, a stock remote winner must remain eligible
    // for the existing hosted takeover; only the enabled role input rejects it.
    check(!cfghostedrole::RejectNonHostWinner(false,true,false));
    check(cfghostedrole::RejectNonHostWinner(true,true,false));
    check(!cfghostedrole::RejectNonHostWinner(true,true,true));
    // 1. Sampler against a non-hosted process: publishes a consistent, even-generation, non-hosted report.
    const auto savedPlan=plan;const auto savedThread=observerOwnerThread;
    observerOwnerThread=GetCurrentThreadId();plan.module=reinterpret_cast<std::uintptr_t>(GetModuleHandleW(nullptr));plan.threadId=observerOwnerThread;
    hostedH1.nextAt=0;ServiceHostedH1(false,true);
    check(hostedH1Report.magic==cfgh1::Magic&&hostedH1Report.version==cfgh1::Version&&hostedH1Report.bytes==sizeof(cfgh1::Report));
    check(!(hostedH1Report.generation&1u)&&hostedH1Report.samples==1&&hostedH1Report.stage<=static_cast<std::uint32_t>(cfgh1::Stage::NoNetDriver));
    check(hostedH1Report.buildFlags==(cfgh1::FlagObserve|cfgh1::FlagJason|(CFG_H20_FILL?cfgh1::FlagFill:0))&&hostedH1Report.armed==0&&hostedH1Report.netDriverSamples==0);
    check(hostedH1Report.framePasses==0&&hostedH1Report.takeoverAttempts==0&&hostedH1Report.takeoverEffects==0&&!hostedH1Report.adoptionPublished);
    // cadence: an immediate second call is rate-limited; nested/unreturned/foreign-thread calls do nothing
    const auto samples=hostedH1Report.samples;ServiceHostedH1(false,true);check(hostedH1Report.samples==samples);
    hostedH1.nextAt=0;ServiceHostedH1(true,true);ServiceHostedH1(false,false);check(hostedH1Report.samples==samples);
    {const auto thread=observerOwnerThread;observerOwnerThread=thread+1;hostedH1.nextAt=0;ServiceHostedH1(false,true);observerOwnerThread=thread;
     check(hostedH1Report.samples==samples);}
#if CFG_H22_HOTKEYS
    {HostedH22Arm();hostedH1.nextAt=0;ServiceHostedH1(false,true);
     check(hostedH1.manualArm&&hostedH1.armed&&hostedH1Report.armed==1&&!hostedH1.takeoverDue);
     hostedH1.options.world=0x123400;hostedH1.nextAt=0;ServiceHostedH1(false,true);
     check(hostedH1.manualArm&&hostedH1.armed&&!hostedH1.options.world&&!hostedH1.takeoverDue);
     HostedH22Disarm();hostedH1.nextAt=0;ServiceHostedH1(false,true);
     check(!hostedH1.armed&&hostedH1.disarmed&&!hostedH1.manualArm&&!hostedH1.takeoverDue);hostedH1.disarmed=false;}
#endif
    // 2. The operator arm file gates every native effect.
    {wchar_t path[MAX_PATH]{};HMODULE self=nullptr;
     check(GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
         reinterpret_cast<LPCWSTR>(&hostedH1Report),&self)&&GetModuleFileNameW(self,path,MAX_PATH));
     auto slash=std::wcsrchr(path,L'\\');check(slash!=nullptr);
     if(slash){wcscpy_s(slash+1,static_cast<rsize_t>(MAX_PATH-(slash+1-path)),L"H1_ARM_JASON.txt");
        DeleteFileW(path);check(!HostedH1ArmFilePresent());
        HANDLE file=CreateFileW(path,GENERIC_WRITE,0,nullptr,CREATE_NEW,FILE_ATTRIBUTE_NORMAL,nullptr);
        check(file!=INVALID_HANDLE_VALUE);if(file!=INVALID_HANDLE_VALUE)CloseHandle(file);
        check(HostedH1ArmFilePresent());check(DeleteFileW(path)&&!HostedH1ArmFilePresent());}}
    // 3. A takeover is never entered unless due+armed+stable on the exact host pawn; the plan is never swapped.
    {const auto before=plan;
     check(!HostedH1EnterPlan(0x1234)&&plan.world==before.world&&plan.pawn==before.pawn&&!hostedH1.planSwapped);
     hostedH1.takeoverDue=true;check(!HostedH1EnterPlan(0x1234)&&!hostedH1.planSwapped&&!hostedH1.takeoverTried);   // not armed
     hostedH1.takeoverDue=false;
     {HostedPlanScope scope(0x1234);check(!scope.active);}
     check(!cfgprimary::CurrentHostedBinding().active&&!hostedAdoptionActive&&!HostedAdoptionOwnsDriver());}
    // 3b. Once adoption is published, the normal host pawn and owned Jason
    // callback re-enter the existing H1 returned-callback lease. This is the
    // scheduling edge used by the one-shot killer presentation and F1 input.
    {const auto saved=hostedH1;const auto binding=cfgprimary::CurrentHostedBinding();
     const bool active=hostedAdoptionActive;const auto owner=observerOwnerThread;
     observerOwnerThread=GetCurrentThreadId();hostedH1.adopted=hostedH1.kept=true;
     hostedH1.adoptedWorld=plan.world;hostedAdoptionActive=true;
     auto& current=cfgprimary::CurrentHostedBinding();current={};current.active=true;current.plan=plan;
     const auto savedJason=bundleJason.pawn.object;bundleJason.pawn.object=plan.pawn+0x1000;
     check(!HostedH1EnterPlan(plan.pawn));
     check(!HostedH1EnterPlan(bundleJason.pawn.object)); // adopted status alone must not admit every later callback
     bundleJason.pawn.object=savedJason;
     hostedH1=saved;cfgprimary::CurrentHostedBinding()=binding;hostedAdoptionActive=active;observerOwnerThread=owner;}
    // 4. Adoption cannot publish without a matching hosted binding and a complete receipt; nothing is left published.
    {cfghosted::adoption::Receipt empty{};hostedH1.adoptionRefusal=0;const bool activeBefore=bundleSchedule.active;const auto pawnBefore=bundleJason.pawn.object;
     check(!HostedPublishAdoption(empty)&&hostedH1.adoptionRefusal==1&&!hostedAdoptionActive&&bundleSchedule.active==activeBefore&&bundleJason.pawn.object==pawnBefore);}
    // 5. The native tick resolver never admits an adopted owner while no adoption is active; offline owners carry no receipt.
    {cfgjasontick::Owner owner{};owner.adoption.valid=true;check(!ResolveNativeJasonTickOwner(0x5000,owner)&&!owner.adoption.valid);}
    // A timed-out hosted native-only context keeps the donor lane reserved
    // until KillRecovery obtains its fresh inactive/non-dying proof. The same
    // policy state remains unreserved without the hosted-only opt-in.
    {const auto old=bundleKill.policy;const auto oldDeathEnd=bundleDeathEnd.confirmedAt;
     bundleKill.policy={};bundleKill.policy.pamela=true;bundleKill.policy.pamelaNativeContextOnly=true;
     bundleKill.policy.hostedNativeContextRecovery=true;bundleKill.policy.nativeActiveObserved=true;
     bundleKill.policy.hostedNativeContextUsedAt=100;bundleKill.policy.usedAt=100;
     bundleKill.policy.interactionAttempted=true;bundleKill.policy.context=0x1234;
     bundleKill.policy.stage=cfgkill::Stage::Expired;bundleKill.policy.until=100;
     bundleDeathEnd.confirmedAt=0;check(bundleKill.policy.RecoveryPending(101)&&
        DonorNativeDeathExclusiveState(true,bundleKill.policy,101,0));
     check(!bundleKill.policy.HostedNativeContextRecoveryProof(101,bundleKill.policy.identity,true,false,0,false,0,100));
     check(!DonorNativeDeathExclusiveState(false,bundleKill.policy,101,0));
     bundleKill.policy.hostedNativeContextRecovery=false;
     check(!bundleKill.policy.RecoveryPending(101)&&!DonorNativeDeathExclusiveState(true,bundleKill.policy,101,0));
     bundleKill.policy=old;bundleDeathEnd.confirmedAt=oldDeathEnd;}
    // 6. Retire is idempotent and restores nothing it did not swap.
    {const auto before=plan;HostedH1Retire();HostedH1Retire();check(plan.world==before.world&&!hostedH1.adopted&&!hostedH1.planSwapped);}
#if CFG_H20_FILL
    {const auto original=hostedFillOriginal;hostedFillOriginal=&H20FillForward;
     const auto calls=gcRegistryReport.calls;HostedFillStartup(reinterpret_cast<void*>(0x1234));
     check(h20FillForwarded==1&&hostedFillDepth==0&&gcRegistryReport.calls==calls&&hostedFillReport.applied==0);
     hostedFillOriginal=original;}
#endif
    check(HostedDifficultyDamage(0x1234,20.f,0x2345,0x3456)==20.f&&!hostedAdoptionActive);
#if CFG_H22_HOTKEYS
    {const auto saved=hostedH1;const auto before=plan;const auto native=gcRegistryReport.calls;
     hostedH1.f3Pending=1;hostedH1.f3ActorHint=0x5678;hostedH1.f3World=0x9876;hostedH1.f3Next=0;
     check(!HostedH1EnterPlan(0x1234)&&!hostedH1.f3Rechecks&&plan.world==before.world);
     hostedH1.f3ActorHint=0x1234;
     for(unsigned i=0;i<4;++i){hostedH1.f3Next=0;check(!HostedH1EnterPlan(0x1234));}
     check(!hostedH1.f3Pending&&hostedLobbyReport.f3Reason==14&&!hostedH1.f3Window&&gcRegistryReport.calls==native);
     hostedH1.f3Pending=1;hostedH1.f3Window=true;
     check(HostedH1Window(nullptr,false)&&hostedLobbyReport.f3Reason==1&&!hostedH1.f3Pending&&gcRegistryReport.calls==native);
     hostedH1.f3Window=false;hostedH1.manualArm=true;hostedH1.armed=true;hostedH1.takeoverDue=true;hostedH1.f3Pending=2;
     HostedH22Disarm();check(hostedH1.disarmed&&!hostedH1.armed&&!hostedH1.manualArm&&!hostedH1.takeoverDue&&!hostedH1.f3Pending&&gcRegistryReport.calls==native);
     hostedH1.takeoverTried=true;hostedH1.takeoverRetrySafe=false;HostedH22Arm();check(hostedH1.armed&&hostedH1.takeoverTried);
     hostedH1.takeoverRetrySafe=true;HostedH22Arm();check(hostedH1.armed&&!hostedH1.takeoverTried&&!hostedH1.takeoverRetrySafe&&gcRegistryReport.calls==native);
     hostedH1=saved;}
#endif
#undef check
    plan=savedPlan;observerOwnerThread=savedThread;
    std::printf("H1 production-source hosted glue: %u checks, %u failures; BuildFlags=0x%X sampler/arm-file/lease/adoption/retire refuse outside a proven armed hosted world. Own process only.\n",
        checks,failures,hostedH1Report.buildFlags);
#ifdef H27_PRODUCTION_EXTRA
    failures+=H27ProductionExtra();
#endif
#ifdef H28_PRODUCTION_EXTRA
    failures+=H28ProductionExtra();
#endif
#ifdef H29_PRODUCTION_EXTRA
    failures+=H29ProductionExtra();
#endif
#ifdef H212_PRODUCTION_EXTRA
    failures+=H212ProductionExtra();
#endif
    return failures;
}
