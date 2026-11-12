#include "../Core/SweaterLifecyclePolicy.hpp"
#include <cassert>
#include "../Core/SweaterPublication.hpp"
#include "KillRuntime.hpp"
#include <cstdio>
#include <initializer_list>
int main(){
    assert(cfgsweater::AssignedOwnedSlot(0x10000,3,0,0x10000,3,12,true));
    assert(!cfgsweater::AssignedOwnedSlot(0x10000,3,1,0x10000,3,12,true));
    assert(!cfgsweater::AssignedOwnedSlot(0x10000,3,0,0x10000,3,12,false));
    using namespace cfgsweater;unsigned checks=0,failures=0;
    auto checkAt=[&](bool ok,unsigned line){++checks;if(!ok){++failures;std::printf("FAIL SweaterLifecycleTests.cpp:%u\n",line);}};
#define check(...) checkAt((__VA_ARGS__),__LINE__)
    Policy p;check(p.Due(0));check(!p.Due(1));check(p.Due(500));
    check(p.Decide(500,false,false,false)==Action::None&&p.stage==Stage::Dormant);
    check(p.Decide(1000,true,false,false)==Action::Discover&&p.stage==Stage::Discovering);
    check(p.Decide(1500,true,false,true)==Action::Publish);
    p.BeginPublish();check(p.publicationStarted);p.ConfirmPublish();check(p.stage==Stage::Owned&&p.clearObserved);
    check(p.Decide(2000,true,true,true)==Action::Observe);check(p.ObserveUsed(2000,1));
    check(p.stage==Stage::Waiting&&p.rearmAt==13000);check(p.KillWindow(2000));check(!p.KillWindow(1999));
    check(p.Decide(12999,true,false,true)==Action::None);check(p.Decide(13000,true,false,true)==Action::Rearm);
    p.BeginPublish();p.Refuse();check(p.Decide(13001,true,false,true)==Action::None&&p.stage==Stage::Refused);
    Policy native;check(native.Decide(10,false,true,true)==Action::Publish);native.ConfirmPublish();native.clearObserved=false;
    check(!native.ObserveUsed(11,1));
    check(!native.ObserveUsed(12,0));check(native.ObserveUsed(13,1));check(!native.ObserveUsed(14,1));
    check(native.KillWindow(10013));check(!native.KillWindow(10014));
    Policy missing;missing.Decide(10,true,false,false);check(missing.Decide(60009,true,false,false)==Action::Discover);
    check(missing.Decide(60010,true,false,false)==Action::None&&missing.stage==Stage::Refused);
    Policy noInput;check(noInput.Decide(100,true,false,false)==Action::Discover);
    check(noInput.Decide(200,true,false,true)==Action::Publish); // gender/input independent
    struct Writer{
        unsigned calls=0,failAt=99;bool fresh=true;std::uintptr_t owner=0,ability=0;
        std::uint8_t has=0,wearing=0,unlimited=0,used=0;
        bool Fresh(const Publication&){return fresh;}
        bool Owner(std::uintptr_t expected,std::uintptr_t value){if(++calls==failAt||owner!=expected)return false;owner=value;return true;}
        bool Ability(std::uintptr_t expected,std::uintptr_t value){if(++calls==failAt||ability!=expected)return false;ability=value;return true;}
        bool Has(std::uint8_t expected,std::uint8_t value){if(++calls==failAt||has!=expected)return false;has=value;return true;}
        bool Wearing(std::uint8_t expected,std::uint8_t value){if(++calls==failAt||wearing!=expected)return false;wearing=value;return true;}
        bool Unlimited(std::uint8_t expected,std::uint8_t value){if(++calls==failAt||unlimited!=expected)return false;unlimited=value;return true;}
        bool Used(std::uint8_t expected,std::uint8_t value){if(++calls==failAt||used!=expected)return false;used=value;return true;}
        bool Visual(){return ++calls!=failAt;}
    };
    Publication grant{0x10000,0x20000,0,0,0,0,0,0,false};Writer w;
    check(Publish(w,grant)&&w.calls==6&&w.owner==grant.pawn&&w.ability==grant.ability&&w.used==0&&w.unlimited==1);
    for(unsigned fail=1;fail<=6;++fail){Writer partial;partial.failAt=fail;check(!Publish(partial,grant)&&partial.calls==fail);}
    Writer wrongOwner;wrongOwner.owner=0x30000;check(!Publish(wrongOwner,grant)&&wrongOwner.calls==1&&wrongOwner.owner==0x30000);
    auto borrowed=grant;borrowed.expectedOwner=0x30000;Writer untouched;
    check(!Publish(untouched,borrowed)&&untouched.calls==0);borrowed=grant;borrowed.expectedAbility=0x30000;
    check(!Publish(untouched,borrowed)&&untouched.calls==0);auto unready=grant;unready.used=1;
    check(!Publish(untouched,unready)&&untouched.calls==0);
    auto recharge=grant;recharge.rearm=true;recharge.used=1;Writer used;used.used=1;
    check(Publish(used,recharge)&&used.calls==7&&used.used==0);
    Writer wrongUsed;check(!Publish(wrongUsed,recharge)&&wrongUsed.calls==6);Writer changed;changed.fresh=false;
    check(!Publish(changed,grant)&&changed.calls==0);
    // Read-budget/source/function unavailability before mutation must preserve
    // an earned sweater and its six-second ordinary recharge deadline.
    for(auto state:{Stage::Dormant,Stage::Discovering,Stage::Owned,Stage::Waiting}){
        Policy transient;transient.stage=state;transient.rearmAt=12000;
        for(unsigned i=0;i<4;++i){transient.Unavailable();check(transient.stage==state&&!transient.publicationStarted);}
        check(transient.readDeferrals==4&&transient.rearmAt==12000);
    }
    Policy retry;retry.ConfirmPublish();check(retry.ObserveUsed(1000,1));
    retry.Unavailable();check(retry.stage==Stage::Waiting&&retry.lastUse==1000);
    check(retry.Decide(11999,false,false,true)==Action::None);
    check(retry.Decide(12000,false,false,true)==Action::Rearm);
    retry.Unavailable();check(retry.Decide(12500,false,false,true)==Action::Rearm);
    retry.BeginPublish();retry.ConfirmPublish();check(retry.stage==Stage::Owned&&retry.clearObserved);
    // Fresh ownership refusal occurs before publication entry; no writes and a
    // later fully validated retry. Every partial write/native visual failure is
    // terminal instead, including failure of the very first write attempt.
    struct GuardedWriter:Writer{
        Policy* policy=nullptr;
        bool Owner(std::uintptr_t from,std::uintptr_t to){policy->BeginPublish();return Writer::Owner(from,to);}
    };
    Policy freshness;freshness.ConfirmPublish();GuardedWriter notFresh;notFresh.policy=&freshness;notFresh.fresh=false;
    check(!Publish(notFresh,grant));freshness.Unavailable();
    check(freshness.stage==Stage::Owned&&freshness.readDeferrals==1&&notFresh.calls==0);
    notFresh.fresh=true;check(Publish(notFresh,grant));freshness.ConfirmPublish();check(freshness.stage==Stage::Owned);
    for(unsigned at=1;at<=7;++at){Policy uncertain;uncertain.ConfirmPublish();GuardedWriter partial;
        partial.policy=&uncertain;partial.used=1;partial.failAt=at;
        check(!Publish(partial,recharge));uncertain.Unavailable();
        check(uncertain.stage==Stage::Refused&&!uncertain.Due(50000)&&partial.calls==at);
    }
    // Kill admission uses the actual kill policy; a fresh bounded host can
    // recover after observation exhausted its budget without extending the hit
    // window, accepting ownership alone, or reopening an accepted transaction.
    struct Kill:cfgkill::Runtime{Kill(){failure=cfgkill::Failure::Scope;}};
    struct ArmHost{
        cfgkill::Identity identity{1,2,3};bool scope=true,living=true,maskRead=true,off=true;
        unsigned budget=3,calls=0;std::uintptr_t expectedWearer=4;
        bool Read(){++calls;if(!budget)return false;--budget;return true;}
        bool Scope(cfgkill::Identity& out){if(!Read()||!scope)return false;out=identity;return true;}
        bool KnownLiving(std::uintptr_t pawn){return Read()&&living&&pawn==expectedWearer;}
        bool MaskOff(bool& value){if(!Read()||!maskRead)return false;value=off;return true;}
    };
    for(unsigned exhausted=0;exhausted<3;++exhausted){Policy pending;pending.ConfirmPublish();Kill kill;ArmHost host;
        check(!pending.ServiceArm(host,kill,4,999)&&host.calls==0);
        check(pending.ObserveUsed(1000,1));check(!pending.ServiceArm(host,kill,4,1000)&&host.calls==0);host.budget=exhausted;
        check(pending.ServiceArm(host,kill,4,1500)&&pending.armPending&&pending.armDeferrals==1);
        check(pending.armFailure==static_cast<ArmFailure>(exhausted+1)&&kill.policy.stage==cfgkill::Stage::Idle);
        const auto beforeRetry=host.calls;check(!pending.ServiceArm(host,kill,4,1600)&&host.calls==beforeRetry);
        host.budget=3;check(pending.ServiceArm(host,kill,4,2000));
        check(!pending.armPending&&pending.arms==1&&pending.armAttempts==2&&kill.failure==cfgkill::Failure::None);
        check(kill.policy.usedAt==1000&&kill.policy.until==11000&&kill.policy.stage==cfgkill::Stage::AwaitHit);
        const auto calls=host.calls;check(!pending.ServiceArm(host,kill,4,2500)&&host.calls==calls);
    }
    for(unsigned failure=0;failure<3;++failure){Policy pending;pending.ConfirmPublish();pending.ObserveUsed(1000,1);
        Kill kill;ArmHost host;host.scope=failure!=0;host.living=failure!=1;host.maskRead=failure!=2;
        check(pending.ServiceArm(host,kill,4,1500)&&pending.armPending);
        check(pending.armFailure==static_cast<ArmFailure>(failure+1)&&kill.policy.stage==cfgkill::Stage::Idle);
    }
    {Policy pending;pending.ConfirmPublish();pending.ObserveUsed(1000,1);Kill kill;ArmHost masked;masked.off=false;
        check(pending.ServiceArm(masked,kill,4,1500)&&!pending.armPending&&pending.armFailure==ArmFailure::Masked);
        check(pending.armRejections==1&&kill.policy.stage==cfgkill::Stage::Idle&&pending.stage==Stage::Waiting);
        masked.off=true;masked.budget=3;check(!pending.ServiceArm(masked,kill,4,2000));
        check(pending.Decide(12000,false,false,true)==Action::Rearm);
    }
    {Policy pending;pending.ConfirmPublish();pending.ObserveUsed(1000,1);Kill kill;ArmHost host;
        check(pending.ServiceArm(host,kill,4,6000)&&pending.armFailure==ArmFailure::Expired&&host.calls==0);
        check(!pending.armPending&&pending.armRejections==1&&pending.stage==Stage::Waiting);
    }
    {Policy pending;pending.ConfirmPublish();pending.ObserveUsed(1000,1);Kill kill;ArmHost host;
        kill.policy.identity=host.identity;kill.policy.stage=cfgkill::Stage::Uncertain;
        check(pending.ServiceArm(host,kill,4,1500)&&pending.armFailure==ArmFailure::Policy&&!pending.armPending);
        check(kill.policy.stage==cfgkill::Stage::Uncertain&&pending.armRejections==1);
    }
    {Policy overflow;overflow.ConfirmPublish();check(!overflow.ObserveUsed(UINT64_MAX-10,1));
        check(overflow.stage==Stage::Owned&&!overflow.armPending);
    }
    {Policy repeated;repeated.ConfirmPublish();Kill kill;ArmHost host;
        for(unsigned cycle=0;cycle<3;++cycle){const auto use=1000ull+cycle*40000ull;
            check(repeated.ObserveUsed(use,1)&&!repeated.ObserveUsed(use+1,1));
            host.budget=3;check(repeated.ServiceArm(host,kill,4,use+500));
            check(kill.policy.usedAt==use&&kill.policy.until==use+10000&&repeated.arms==cycle+1);
            repeated.Unavailable();check(repeated.Decide(use+10000,false,false,true)==Action::None);
            check(repeated.Decide(use+11000,false,false,true)==Action::Rearm);
            repeated.BeginPublish();repeated.ConfirmPublish();
        }
    }
    {Policy hud;hud.QueueHud(1000,false);hud.next=1500;
        check(ChooseContinuation(hud,1001,true,false,true)==Continuation::Class);
        check(ChooseContinuation(hud,1500,true,true,true)==Continuation::Hud);
        check(ChooseContinuation(hud,1500,false,true,true)==Continuation::Hud);
        // The actual production chooser finishes owed HUD before optional
        // fresh/class requests even when helper publication stays active.
        unsigned nativeHudCalls=0,classTurns=0;
        for(std::uint64_t now=1000;now<2000;now+=25){
            const auto next=ChooseContinuation(hud,now,true,false,true);
            if(next==Continuation::Hud){check(hud.Due(now));
                check(hud.Decide(now,false,true,true)==Action::PublishHud);
                ++nativeHudCalls;hud.ConfirmPublish();}
            else if(next==Continuation::Class)++classTurns;
        }
        check(nativeHudCalls==1&&classTurns&&hud.stage==Stage::Owned);
        check(hud.hudQueuedAt==1000&&hud.hudFirstServiceAt==1500&&!hud.hudProofFailure);
        check(hud.ObserveUsed(2000,1)&&hud.lastUse==2000); // no cosmetic timeout disables use tracking
        check(ChooseContinuation(hud,2000,true,true,true)==Continuation::Fresh);
    }
    {Policy starved;starved.QueueHud(1000,false);
        check(starved.Decide(6000,false,true,true)==Action::None);
        check(starved.hudFirstServiceAt==6000&&starved.hudQueuedAt==1000&&
            !starved.readDeferrals&&!starved.hudProofFailure&&starved.lastFailure==30);
    }
    {Policy hud;hud.BeginPublish();hud.QueueHud(1000,false);
        check(hud.stage==Stage::HudPending&&!hud.publicationStarted&&!hud.pendingRearm);
        check(hud.Decide(1001,false,true,true)==Action::PublishHud);
        hud.Unavailable();check(hud.stage==Stage::HudPending&&hud.readDeferrals==1);
        hud.BeginPublish();hud.ConfirmPublish();check(hud.stage==Stage::Owned&&hud.clearObserved);
        hud.QueueHud(2000,true);check(hud.pendingRearm&&hud.Decide(6999,false,true,true)==Action::PublishHud);
        check(hud.Decide(7000,false,true,true)==Action::None&&hud.stage==Stage::Refused&&hud.lastFailure==30);
    }
    {Policy hud;hud.QueueHud(1000,true);hud.BeginPublish();hud.Unavailable();check(hud.stage==Stage::Refused);}
    check(DetachedEarnedAbility(true,true,0,0));
    check(!DetachedEarnedAbility(false,true,0,0));check(!DetachedEarnedAbility(true,false,0,0));
    check(!DetachedEarnedAbility(true,true,0x10000,0));check(!DetachedEarnedAbility(true,true,0,0x10000));
    // Same production reserve gate, including all six expected-value writes
    // and their readbacks, plus the final visual native call. Fresh gets500
    // reads (greater than ordinary CFG context+source proofs), no new reader.
    struct BudgetWriter:Writer{unsigned reads=1024,nativeCalls=11;bool Read(unsigned n){if(n>1536-reads)return false;reads+=n;return true;}
        bool Fresh(const Publication&){return PublicationBudget(reads,1536,nativeCalls,12)&&Read(500);}
        bool Owner(std::uintptr_t a,std::uintptr_t b){return Read(2)&&Writer::Owner(a,b);}
        bool Ability(std::uintptr_t a,std::uintptr_t b){return Read(2)&&Writer::Ability(a,b);}
        bool Has(std::uint8_t a,std::uint8_t b){return Read(2)&&Writer::Has(a,b);}
        bool Wearing(std::uint8_t a,std::uint8_t b){return Read(2)&&Writer::Wearing(a,b);}
        bool Unlimited(std::uint8_t a,std::uint8_t b){return Read(2)&&Writer::Unlimited(a,b);}
        bool Used(std::uint8_t a,std::uint8_t b){return Read(2)&&Writer::Used(a,b);}
        bool Visual(){if(nativeCalls>=12)return false;++nativeCalls;return Writer::Visual();}
    };
    {BudgetWriter budget;budget.used=1;check(Publish(budget,recharge)&&budget.reads==1536&&budget.nativeCalls==12&&budget.calls==7);}
    {BudgetWriter budget;budget.used=1;budget.reads=1025;check(!Publish(budget,recharge)&&!budget.calls&&budget.reads==1025);}
    {BudgetWriter budget;budget.used=1;budget.nativeCalls=12;check(!Publish(budget,recharge)&&!budget.calls);}
    check(!PublicationBudget(0,511,0,12));
    {Policy replacement;replacement.ConfirmPublish();check(replacement.ObserveUsed(2000,1));
        const auto use=replacement.lastUse,deadline=replacement.rearmAt;replacement.armPending=false;
        check(replacement.ReplaceOwned(3000,0)&&replacement.stage==Stage::HudPending&&replacement.pendingRearm);
        check(replacement.lastUse==use&&replacement.rearmAt==deadline&&!replacement.armPending);
        replacement.ConfirmPublish();check(replacement.ObserveUsed(15000,1));
        const auto nextDeadline=replacement.rearmAt;check(replacement.ReplaceOwned(16000,1));
        check(replacement.stage==Stage::Waiting&&replacement.rearmAt==nextDeadline);
        replacement.BeginPublish();check(!replacement.ReplaceOwned(17000,0));
        replacement.Refuse();check(!replacement.ReplaceOwned(18000,0));}
    check(AcceptOwnedDuringDiscovery(Stage::Dormant));check(AcceptOwnedDuringDiscovery(Stage::Discovering));
    check(!AcceptOwnedDuringDiscovery(Stage::Waiting)&&!AcceptOwnedDuringDiscovery(Stage::Refused));
    {Policy actual;check(actual.Decide(1000,true,false,false)==Action::Discover);
        check(AcceptOwnedDuringDiscovery(actual.stage));check(actual.Decide(1500,false,true,true)==Action::Publish);
        actual.ConfirmPublish();check(actual.ObserveUsed(2000,1));check(actual.Decide(actual.rearmAt,false,false,true)==Action::Rearm);}
    {Policy expired;check(expired.Decide(1000,true,false,false)==Action::Discover);
        check(expired.Decide(61000,true,false,false)==Action::None&&expired.searchExpired&&expired.CanPoll());
        check(expired.AdoptNativeOwned()&&!expired.searchExpired);
        check(expired.Decide(62000,false,true,true)==Action::Publish); // real pickup wins over expired mask search
        expired.BeginPublish();expired.Refuse();check(!expired.CanPoll()&&!expired.AdoptNativeOwned());}
    {Policy repair;repair.ConfirmPublish();check(repair.QueueRetiredOwned(100)&&repair.repairingOwned&&repair.stage==Stage::Waiting);
        check(!repair.lastUse&&!repair.armPending&&repair.ownedRepairs==1&&repair.deadline==60100);
        repair.ConfirmPublish();check(!repair.repairingOwned&&repair.QueueRetiredOwned(200));
        repair.ConfirmPublish();check(repair.QueueRetiredOwned(300));repair.ConfirmPublish();check(!repair.QueueRetiredOwned(400));
        Policy earned;earned.ConfirmPublish();check(earned.ObserveUsed(10,1));earned.ConfirmPublish();const auto use=earned.lastUse;
        check(earned.QueueRetiredOwned(20)&&earned.lastUse==use);}
    std::printf("Sweater lifecycle checks=%u failures=%u\n",checks,failures);return failures?1:0;
}
