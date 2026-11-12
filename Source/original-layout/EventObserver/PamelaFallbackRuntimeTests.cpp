#define CFG_PAMELA_HOST_ONLY
#include "PamelaKillTests.cpp"
#undef CFG_PAMELA_HOST_ONLY
#include <array>
struct FallbackHost:PamelaHost {
    Runtime* runtime=nullptr;std::uint64_t now=0;
    bool human=false,commit=false,fault=false;
    unsigned dispatches=0,preparations=0;std::array<std::uintptr_t,8> recipients{};
    bool KnownLiving(std::uintptr_t pawn){return pawn==4||pawn==5;}
    bool CurrentWeapon(std::uintptr_t pawn,std::uintptr_t& weapon,Weapon& type){weapon=6;type=Weapon::Other;return KnownLiving(pawn);}
    bool PamelaCandidate(unsigned i,std::uintptr_t& pawn,std::uintptr_t& weapon,Weapon& type){
        pawn=i?5:4;
        if(!i&&!human)return false;
        if(i&&(!runtime->pamelaStock.Current(runtime->policy,now)||runtime->pamelaStock.pawn!=pawn))return false;
        return CurrentWeapon(pawn,weapon,type);
    }
    bool PamelaFinisher(std::uintptr_t& pawn,std::uintptr_t& weapon,Weapon& type){
        return SelectPamelaFinisher(*this,1,runtime->pamelaFinisherCursor,pawn,weapon,type);
    }
    Preparation PrepareInteraction(std::uintptr_t actor,std::uintptr_t pawn){
        ++preparations;assert(actor==10&&KnownLiving(pawn));return Preparation::Ready;
    }
    template<class Mark>bool FinalInteraction(const Context& c,std::uintptr_t pawn,Mark mark){
        assert(c.actor==10&&dispatches<recipients.size());
        if(!mark())return false;
        recipients[dispatches++]=pawn;
        if(fault)return false; // ambiguous/unreturned call, never retried
        if(commit)context.active=true;
        return true; // returned native invocation is NOT evidence that a kill started
    }
    Step Tick(Runtime& r,std::uint64_t at){runtime=&r;now=at;return r.Tick(*this,at);}
    bool Stock(std::uint64_t at,std::uintptr_t ctx=10){
        return runtime->pamelaStock.Remember(runtime->policy,ctx,12,5,6,55,555,at);
    }
};
static void ArmFallback(FallbackHost& h,Runtime& r){
    h.runtime=&r;assert(r.policy.PamelaUsed(h.identity,4,100,true,100,100));
    assert(r.policy.PamelaAccept(h.identity,101));
    assert(h.Tick(r,102)==Step::StanceDispatched&&!r.policy.finisher&&!h.dispatches);
    h.context={10,11,12,false};
}
int main(){
    {FallbackHost h;Runtime r;ArmFallback(h,r);
     assert(h.Tick(r,103)==Step::Waiting&&!h.dispatches&&!r.policy.finisher); // no stock proof, no AI
     assert(!h.Stock(103,99)); // foreign context never admitted
     assert(h.Stock(103));
     assert(h.Tick(r,104)==Step::InteractionDispatched&&h.recipients[0]==5&&!r.policy.interactionAttempted);
     h.human=true;
     assert(h.Tick(r,105)==Step::Waiting&&r.policy.finisher==4&&!r.policy.interactionAttempted&&h.dispatches==1);
     assert(h.Tick(r,1103)==Step::Waiting&&h.dispatches==1); // exact1s backoff
     h.commit=true;
     assert(h.Tick(r,1104)==Step::InteractionDispatched&&h.dispatches==2&&h.recipients[1]==4&&!r.policy.interactionAttempted);
     assert(h.Tick(r,1105)==Step::NativeActive&&r.policy.interactionAttempted);
     assert(h.Tick(r,1106)==Step::NativeActive&&h.dispatches==2);}
    {FallbackHost h;Runtime r;ArmFallback(h,r);assert(h.Tick(r,103)==Step::Waiting);
     assert(h.Stock(103));assert(h.Tick(r,20104)==Step::Waiting&&!h.dispatches); // stock proof older than20s
     h.human=true;h.now=20105;r.pamelaStock.weapon=99;
     assert(h.Tick(r,20105)==Step::InteractionDispatched&&h.recipients[0]==4);}
    {FallbackHost h;Runtime r;ArmFallback(h,r);h.human=true;
     assert(h.Tick(r,103)==Step::InteractionDispatched&&!r.policy.interactionAttempted);
     const auto end=r.policy.until;assert(end==60101); // refusals never extend the window
     assert(h.Tick(r,end)==Step::Idle&&h.dispatches==1&&!r.policy.interactionAttempted&&r.policy.RecoveryPending(end));
     cfgsweater::Policy sweater;sweater.stage=cfgsweater::Stage::Waiting;sweater.rearmAt=end+1;
     assert(sweater.Decide(end+1,true,true,true)==cfgsweater::Action::Rearm);}
    {FallbackHost h;Runtime r;ArmFallback(h,r);h.human=true;h.fault=true;
     assert(h.Tick(r,103)==Step::Unavailable&&r.policy.stage==Stage::Uncertain&&!r.policy.interactionAttempted);
     assert(!r.policy.RecoveryPending(r.policy.until)); // ambiguous call neither consumed nor eligible for EndStun
     assert(h.Tick(r,2000)==Step::Idle&&h.dispatches==1);}
    {Policy p;assert(p.PamelaUsed({1,2,3},4,100,true,100,100)&&p.PamelaAccept({1,2,3},101));
     assert(p.ObserveContext(10,102,false));PamelaStockApproval stock;
     assert(stock.Remember(p,10,12,5,6,55,555,103));assert(stock.Current(p,104));
     assert(!stock.Remember(p,10,12,5,6,55,555,105)&&stock.at==103);
     auto other=p;++other.usedAt;assert(!stock.Current(other,104));
     other=p;++other.identity.serial;assert(!stock.Current(other,104));
     other=p;other.context=99;assert(!stock.Current(other,104));
     assert(!stock.Current(p,102)&&stock.Current(p,20103)&&!stock.Current(p,20104));}
    std::puts("B1 #129 actual Runtime PASS: stance before selection; no unapproved AI; exact-context20s stock proof; later human wins; refusal stays open/1s fresh dispatch; native active consumes; expiry recharges; ambiguous call never retries.");
    return 0;
}
