#include "KillRuntime.hpp"
#include "KillEligibility.hpp"
#include <cassert>
#include <cstdio>
#include <limits>
using namespace cfgkill;
struct Host {
    Identity identity{1,2,3};bool scope=true,living=true,mask=true,stunned=false,bridge=true,enter=true,interaction=true,event=true;
    unsigned bridges=0,stances=0,interactions=0;
    std::uintptr_t held=6;Weapon kind=Weapon::Axe;float squared=100;
    DamagePacket packet{12,0,7,5,4};Context context{};
    bool Scope(Identity& out){out=identity;return scope;}
    bool KnownLiving(std::uintptr_t p){return living&&p==4;}
    bool DamageCounselor(std::uintptr_t p,std::uintptr_t& out){out=4;return p==5;}
    bool MaskOff(bool& out){out=mask;return true;}
    bool InStun(bool& out){out=stunned;return true;}
    bool CurrentWeapon(std::uintptr_t p,std::uintptr_t& out,Weapon& type){out=held;type=kind;return p==4;}
    bool DamageTypeMatches(std::uintptr_t weapon,std::uintptr_t type){return weapon==6&&type==7;}
    bool DistanceSquared(std::uintptr_t,float& out){out=squared;return true;}
    bool DamageEvent(std::uintptr_t f){return event&&f==8;}
    bool ReadDamage(std::uintptr_t p,DamagePacket& out){out=packet;return p==9;}
    bool contextValid=true;
    bool DeathContext(Context& out){out=context;return contextValid;}
    bool BridgeStun(){++bridges;return bridge;}
    bool EnterStance(){++stances;return enter;}
    bool FinalInteraction(Context c,std::uintptr_t p){++interactions;return interaction&&c.actor==10&&p==4;}
};
struct PreparingHost:Host {
    Preparation result=Preparation::Pending;unsigned preparations=0,scopeReads=0;
    Preparation PrepareStance(){++preparations;return result;}
    bool Scope(Identity& out){++scopeReads;return Host::Scope(out);}
};
struct TransactionHost:PreparingHost {
    Runtime* runtime=nullptr;unsigned interactionPhases=0,dispatchChecks=0,interactionChecks=0;
    bool stanceFresh=true,interactionFresh=true,prepareInteractionOk=true;
    Preparation PrepareInteraction(std::uintptr_t actor,std::uintptr_t pawn){
        assert(actor==10&&pawn==4&&runtime&&!runtime->policy.interactionAttempted);
        if(!prepareInteractionOk)return Preparation::Unavailable;
        if(interactionPhases<3){++interactionPhases;return Preparation::Pending;}
        return Preparation::Ready;
    }
    template<class Mark> bool DispatchStance(Mark mark){
        ++dispatchChecks;assert(runtime&&!runtime->policy.stanceAttempted);
        if(!stanceFresh)return false; // resource/frame/owner checks precede write
        if(!stunned&&!BridgeStun())return false;
        assert(!runtime->policy.stanceAttempted);
        if(!mark())return false;
        assert(runtime->policy.stanceAttempted);return EnterStance();
    }
    template<class Mark> bool FinalInteraction(const Context& c,std::uintptr_t p,Mark mark){
        ++interactionChecks;assert(runtime&&!runtime->policy.interactionAttempted);
        if(!interactionFresh)return false;
        if(!mark())return false;
        assert(runtime->policy.interactionAttempted);return Host::FinalInteraction(c,p);
    }
};
template<class H>static Runtime Armed(H& h){Runtime r;assert(r.SweaterUsed(h,4,100));return r;}
template<class H>static bool HitNow(Runtime& r,H& h,std::uint64_t now=101){return r.ObserveDamage(h,2,8,9,now,true);}
struct TableReader {
    std::uintptr_t module=0x140000000;EligibilityTable data{};
    TableReader(){data[0]=module+disk::EligibilityVtablePrefix;for(unsigned i=0;i<disk::EligibilityVtableEntries;++i)data[i+1]=module+disk::EligibilityVtableRvas[i];}
    bool Read(std::uintptr_t at,EligibilityTable& out){out=data;return at==module+disk::EligibilityVtable-sizeof(std::uintptr_t);}
};
#if defined(_WIN32)
static volatile char* testHunter=nullptr;
static int __fastcall Accepted(void*,void*,const EligibilityVector*,const EligibilityVector*){assert(*testHunter==1);return 3;}
static int __fastcall Refused(void*,void*,const EligibilityVector*,const EligibilityVector*){assert(*testHunter==1);return 0;}
static int __fastcall Fault(void*,void*,const EligibilityVector*,const EligibilityVector*){RaiseException(0xE0424242,0,0,nullptr);return 0;}
static bool FaultRestores(volatile char* byte){
    bool caught=false;__try {CallEligibilityWithHunter(Fault,nullptr,nullptr,nullptr,nullptr,byte);}
    __except(EXCEPTION_EXECUTE_HANDLER){caught=true;}return caught;
}
#endif
int main(){
    {struct FinishHost:Host{unsigned armed=0;bool ok=true;bool RequiresFinishingStrike()const{return true;}
        bool ArmFinishingStrike(const Context& c,std::uintptr_t pawn){++armed;assert(c.actor==10&&pawn==4);return ok;}}h;
     auto r=Armed(h);assert(HitNow(r,h));h.context={10,11,12,false};
     assert(r.Tick(h,102)==Step::ContextReady&&h.armed==1&&!h.interactions);
     assert(r.policy.until==6102);assert(r.Tick(h,200)==Step::ContextReady&&h.armed==1&&r.policy.until==6102);
     assert(r.Tick(h,6102)==Step::Idle&&r.policy.RecoveryPending(6102)&&!h.interactions);}
    {struct FinishHost:Host{unsigned armed=0;bool RequiresFinishingStrike()const{return true;}
        bool ArmFinishingStrike(const Context&,std::uintptr_t){++armed;return false;}}h;
     auto r=Armed(h);assert(HitNow(r,h));h.context={10,11,12,false};
     assert(r.Tick(h,102)==Step::Unavailable&&r.policy.stage==Stage::Uncertain&&h.armed==1);
     assert(r.Tick(h,103)==Step::Idle&&h.armed==1&&!h.interactions);}
    {Policy p;assert(p.SweaterUsed({1,2,3},4,100,true));p.stage=Stage::AwaitContext;
     assert(p.ObserveContext(10,200,false)&&p.until==6200);
     assert(p.ObserveContext(10,300,false)&&p.until==6200); // inactive polling never extends it
     assert(p.ObserveContext(10,400,true)&&p.until==60400&&p.nativeActiveObserved);
     assert(p.ObserveContext(10,500,true)&&p.until==60400);p.Expire(6200);
     assert(p.Exclusive(6200)&&!p.RecoveryPending(6200));}
    {struct FinishHost:Host{bool RequiresFinishingStrike()const{return true;}}h;
     auto r=Armed(h);assert(HitNow(r,h));h.context={10,11,12,false};
     assert(r.Tick(h,102)==Step::ContextReady&&!h.interactions&&!r.policy.interactionAttempted);
     assert(r.Tick(h,103)==Step::ContextReady&&!h.interactions); // standing nearby cannot finish
     assert(!HitNow(r,h,101)); // same first strike cannot be reused
     assert(HitNow(r,h,104)&&r.policy.finishingStrike&&r.accepted==2);
     assert(!HitNow(r,h,105));
     assert(r.Tick(h,106)==Step::InteractionDispatched&&h.interactions==1);
     assert(r.Tick(h,107)==Step::ContextReady&&h.interactions==1);}
    {struct FinishHost:Host{bool RequiresFinishingStrike()const{return true;}}h;
     auto r=Armed(h);assert(HitNow(r,h));h.context={10,11,12,false};assert(r.Tick(h,102)==Step::ContextReady);
     h.context.actor=20;assert(!HitNow(r,h,103)&&!h.interactions);
     h.context={10,11,12,true};assert(!HitNow(r,h,104)&&r.Tick(h,105)==Step::NativeActive&&!h.interactions);}
    {TransactionHost h;Runtime r;h.runtime=&r;h.result=Preparation::Ready;
        assert(r.SweaterUsed(h,4,100)&&r.ObserveDamage(h,2,8,9,101,true));h.context={10,11,12,false};
        assert(r.Tick(h,102)==Step::Waiting);const auto scopes=h.scopeReads;
        assert(h.interactionPhases==1&&!r.policy.interactionAttempted);
        assert(r.Tick(h,103)==Step::Waiting&&h.interactionPhases==2&&h.scopeReads==scopes);
        assert(r.Tick(h,104)==Step::Waiting&&h.interactionPhases==3&&h.scopeReads==scopes);
        assert(!h.bridges&&!h.stances&&!h.interactions&&!h.interactionChecks);
        assert(r.Tick(h,105)==Step::InteractionDispatched&&h.interactions==1&&h.interactionChecks==1);
        assert(r.Tick(h,106)==Step::ContextReady&&h.interactions==1);}
    // Only a dispatched ambiguity (or failed preparation) is terminal.
    for(unsigned test=0;test<2;++test){TransactionHost h;Runtime r;h.runtime=&r;h.result=Preparation::Ready;
        assert(r.SweaterUsed(h,4,100)&&r.ObserveDamage(h,2,8,9,101,true));h.context={10,11,12,false};
        h.interactionPhases=3;
        if(test==0)h.prepareInteractionOk=false;else h.interaction=false;
        assert(r.Tick(h,102)==Step::Unavailable&&!r.policy.Exclusive(103));
        assert(r.policy.interactionAttempted==(test==1));assert(h.interactions==(test==1?1u:0u));
        const auto checks=h.interactionChecks;assert(r.Tick(h,103)==Step::Idle&&h.interactionChecks==checks);}
    {TransactionHost h;Runtime r;h.runtime=&r;h.result=Preparation::Ready;h.interactionPhases=3;
        assert(r.SweaterUsed(h,4,100)&&r.ObserveDamage(h,2,8,9,101,true));h.context={10,11,12,false};h.interactionFresh=false;
        assert(r.Tick(h,102)==Step::Waiting&&r.policy.Exclusive(103)&&!r.policy.interactionAttempted);
        assert(r.interactionDeferrals==1&&!h.interactions);
        h.interactionFresh=true;assert(r.Tick(h,103)==Step::InteractionDispatched&&h.interactions==1);
        assert(r.Tick(h,104)==Step::ContextReady&&h.interactions==1&&r.failure==Failure::None);}
    {TransactionHost h;Runtime r;h.runtime=&r;h.result=Preparation::Ready;h.interactionPhases=3;
        assert(r.SweaterUsed(h,4,100)&&r.ObserveDamage(h,2,8,9,101,true));h.context={10,11,12,false};h.interactionFresh=false;
        assert(r.Tick(h,102)==Step::Waiting);assert(r.Tick(h,60102)==Step::Idle&&r.policy.stage==Stage::Expired);
        assert(!h.interactions&&!r.policy.interactionAttempted);}
    {TransactionHost h;Runtime r;h.runtime=&r;h.result=Preparation::Ready;h.stanceFresh=false;
        assert(r.SweaterUsed(h,4,100)&&r.ObserveDamage(h,2,8,9,101,true));
        assert(r.Tick(h,102)==Step::Uncertain&&!r.policy.stanceAttempted&&!h.bridges&&!h.stances);
        assert(r.Tick(h,103)==Step::Idle&&h.dispatchChecks==1);}
    {TransactionHost h;Runtime r;h.runtime=&r;h.result=Preparation::Ready;
        assert(r.SweaterUsed(h,4,100)&&r.ObserveDamage(h,2,8,9,101,true));
        assert(r.Tick(h,102)==Step::StanceDispatched&&r.policy.stanceAttempted&&h.bridges==1&&h.stances==1);
        assert(r.Tick(h,103)==Step::Waiting&&h.stances==1);}
    {PreparingHost h;Runtime r;assert(r.SweaterUsed(h,4,100));assert(r.ObserveDamage(h,2,8,9,101,true));
        const auto before=h.scopeReads;assert(r.Tick(h,102)==Step::Waiting);
        assert(h.preparations==1&&h.scopeReads==before&&!r.policy.stanceAttempted&&!h.bridges&&!h.stances);
        h.result=Preparation::Ready;assert(r.Tick(h,103)==Step::StanceDispatched);assert(h.preparations==2);
        assert(r.Tick(h,104)==Step::Waiting);assert(h.preparations==2);}
    {PreparingHost h;Runtime r;assert(r.SweaterUsed(h,4,100));assert(r.ObserveDamage(h,2,8,9,101,true));
        h.result=Preparation::Unavailable;assert(r.Tick(h,102)==Step::Unavailable);
        assert(r.failure==Failure::StancePreparation&&!r.policy.Exclusive(103)&&!r.policy.stanceAttempted&&!h.bridges&&!h.stances);}
    {Host h;auto r=Armed(h);assert(HitNow(r,h));h.contextValid=false;
        assert(r.Tick(h,102)==Step::Unavailable&&r.failure==Failure::Context);
        assert(!h.bridges&&!h.stances&&!h.interactions&&r.Tick(h,103)==Step::Idle);
        h.contextValid=true;assert(!r.SweaterUsed(h,4,12000)&&r.failure==Failure::Context);
        r.policy.Clear();assert(r.SweaterUsed(h,4,12000)&&r.failure==Failure::None);}
    {PreparingHost h;Runtime r;assert(r.SweaterUsed(h,4,100));assert(r.ObserveDamage(h,2,8,9,101,true));
        assert(r.Tick(h,6101)==Step::Idle);assert(h.preparations==0&&!h.bridges&&!h.stances);}
    {Host h;Runtime r;assert(!HitNow(r,h));assert(r.Tick(h,101)==Step::Idle);assert(h.stances==0);}
    {Host h;auto r=Armed(h);assert(!r.SweaterUsed(h,4,100));assert(r.policy.until==10100);assert(r.Tick(h,101)==Step::Idle);assert(h.bridges==0&&h.stances==0);
        assert(r.SweaterUsed(h,4,12000));assert(r.policy.until==22000);}
    {Host h;auto r=Armed(h);assert(!r.ObserveDamage(h,2,8,9,101,false));assert(!r.ObserveDamage(h,20,8,9,101,true));assert(!r.ObserveDamage(h,2,80,9,101,true));assert(!r.ObserveDamage(h,2,8,90,101,true));assert(!HitNow(r,h,99));assert(!HitNow(r,h,30100));}
    for(unsigned invalid=0;invalid<12;++invalid){Host h;auto r=Armed(h);
        switch(invalid){
        case 0:h.packet.damage=0;break;case 1:h.packet.damage=-1;break;
        case 2:h.packet.damage=std::numeric_limits<float>::quiet_NaN();break;
        case 3:h.packet.instigatedBy=50;break;case 4:h.packet.damageCauser=60;break;
        case 5:h.kind=Weapon::Other;break;case 6:h.living=false;break;case 7:h.mask=false;break;
        case 8:h.squared=40001;break;case 9:h.squared=std::numeric_limits<float>::infinity();break;
        case 10:++h.identity.serial;break;default:h.packet.damageType=99;break;}
        assert(!HitNow(r,h));assert(h.bridges==0&&h.stances==0);
    }
    {Host h;auto r=Armed(h);assert(HitNow(r,h));assert(!HitNow(r,h));assert(r.policy.Exclusive(102));
        assert(r.Tick(h,102)==Step::StanceDispatched);assert(h.bridges==1&&h.stances==1);
        assert(r.Tick(h,103)==Step::Waiting);assert(h.stances==1);h.context={10,11,12,false};
        assert(r.Tick(h,104)==Step::InteractionDispatched);assert(r.Tick(h,105)==Step::ContextReady);assert(h.interactions==1);
        h.context.active=true;assert(r.Tick(h,106)==Step::NativeActive);
        h.context.actor=99;assert(r.Tick(h,107)==Step::Uncertain);assert(!r.policy.Exclusive(108));
    }
    {Host h;h.kind=Weapon::Machete;auto r=Armed(h);assert(HitNow(r,h));h.stunned=true;
        assert(r.Tick(h,102)==Step::StanceDispatched);assert(h.bridges==0&&h.stances==1);}
    {Host h;auto r=Armed(h);assert(HitNow(r,h));h.context={10,11,12,true};
        assert(r.Tick(h,102)==Step::NativeActive);assert(h.bridges==0&&h.stances==0&&h.interactions==0);}
    {Host h;auto r=Armed(h);assert(HitNow(r,h));h.enter=false;
        assert(r.Tick(h,102)==Step::Uncertain);assert(r.Tick(h,103)==Step::Idle);assert(h.stances==1);assert(!r.SweaterUsed(h,4,50000));}
    {Host h;auto r=Armed(h);assert(HitNow(r,h));h.bridge=false;
        assert(r.Tick(h,102)==Step::Uncertain);assert(h.stances==0);}
    {Host h;auto r=Armed(h);assert(HitNow(r,h));h.held=60;
        assert(r.Tick(h,102)==Step::Unavailable);assert(!r.policy.Exclusive(103));assert(h.stances==0);assert(r.Tick(h,6101)==Step::Idle);}
    {TransactionHost h;Runtime r;h.runtime=&r;h.result=Preparation::Ready;h.interactionPhases=3;
        assert(r.SweaterUsed(h,4,100)&&r.ObserveDamage(h,2,8,9,101,true));h.context={10,11,12,false};h.held=60;
        assert(r.Tick(h,102)==Step::Unavailable&&!r.policy.Exclusive(103)&&!r.policy.interactionAttempted);
        assert(r.Tick(h,103)==Step::Idle&&!h.interactionChecks&&!h.interactions);}
    {Host h;auto r=Armed(h);assert(HitNow(r,h));h.scope=false;
        assert(r.Tick(h,102)==Step::Uncertain);assert(h.bridges==0&&h.stances==0);}
    {Host h;auto r=Armed(h);assert(r.Tick(h,30100)==Step::Idle);assert(r.policy.stage==Stage::Expired);assert(r.SweaterUsed(h,4,30101));}
    {Policy p;assert(!p.SweaterUsed({1,2,0},4,1,true));assert(!p.SweaterUsed({1,2,3},4,UINT64_MAX,true));}
    {TableReader reader;EligibilityTable out{};assert(BuildEligibilityTable(reader,reader.module,reader.module+disk::EligibilityVtable,123,out));
        for(unsigned i=0;i<out.size();++i)assert(out[i]==(i==162?123:reader.data[i]));
        const auto saved=out;reader.data[50]^=1;assert(!BuildEligibilityTable(reader,reader.module,reader.module+disk::EligibilityVtable,123,out));assert(out==saved);
        assert(!BuildEligibilityTable(reader,reader.module,reader.module+disk::EligibilityVtable+8,123,out));
    }
    {Host h;auto r=Armed(h);assert(HitNow(r,h));h.context={10,11,12,false};r.Tick(h,102);
        EligibilityTicket t{h.identity,10,12,4,true,true,true,true,true};
        assert(MayRelax(r.policy,t,12,4,103));assert(!MayRelax(r.policy,t,11,4,103));assert(!MayRelax(r.policy,t,12,40,103));
        t.offline=false;assert(!MayRelax(r.policy,t,12,4,103));t.offline=true;t.nativeCode=false;assert(!MayRelax(r.policy,t,12,4,103));
    }
#if defined(_WIN32)
    {volatile char byte=0;testHunter=&byte;assert(CallEligibilityWithHunter(Accepted,nullptr,nullptr,nullptr,nullptr,&byte)==3);assert(byte==0);
        assert(CallEligibilityWithHunter(Refused,nullptr,nullptr,nullptr,nullptr,&byte)==0);assert(byte==0);
        assert(FaultRestores(&byte)&&byte==0);byte=1;assert(CallEligibilityWithHunter(Accepted,nullptr,nullptr,nullptr,nullptr,&byte)==3);assert(byte==1);
    }
#endif
    {Policy p;p.identity={1,2,3};p.stage=Stage::Expired;p.until=100;p.stanceAttempted=true;
        assert(!p.RecoveryPending(99)&&p.RecoveryPending(100)&&p.RecoveryPending(5100)&&!p.RecoveryPending(5101));
        p.interactionAttempted=true;assert(!p.RecoveryPending(100));p.interactionAttempted=false;
        p.nativeActiveObserved=true;assert(!p.RecoveryPending(100));p.nativeActiveObserved=false;
        assert(p.RequestRecovery(100)&&!p.RequestRecovery(101)&&!p.RecoveryPending(101));}
    {Policy p;p.identity={1,2,3};p.context=10;p.stage=Stage::Uncertain;p.until=100;
     assert(!p.stanceAttempted&&p.RecoveryPending(100)&&p.RequestRecovery(100));
     assert(!p.ConfirmRecovery(101,true,true));p.recoveryReturned=true;
     assert(!p.ConfirmRecovery(101,false,true)&&!p.ConfirmRecovery(101,true,false));
     assert(p.ConfirmRecovery(101,true,true)&&p.stage==Stage::Idle&&p.SweaterUsed({1,2,3},4,102,true));}
    std::puts("CFG Pamela native kill evidence/state machine: PASS");
    return 0;
}
