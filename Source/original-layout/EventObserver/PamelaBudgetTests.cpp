#pragma warning(push)
#pragma warning(disable:4716) // legacy test main has main's implicit return; renamed and never called
#define main ExistingKillBudgetMain
#include "KillBudgetTests.cpp"
#undef main
#pragma warning(pop)
// Same declared shallow-class cost model as the legacy budget fixture. This
// drives the real Pamela state machine; it is not a live ancestry measurement.
namespace {
struct PamelaBudgetHost:Host {
    bool contextHook=false;unsigned dispatchCount=0;
    unsigned TypedHeld()const{return 31+4*cost.weapon;}
    bool PamelaFinisher(std::uintptr_t& pawn,std::uintptr_t& weapon,cfgkill::Weapon& kind){
        pawn=4;weapon=6;kind=cfgkill::Weapon::Other;
        return Read(2*cost.Known()+12+1+TypedHeld())&&Call(2);
    }
    cfgkill::Preparation PreparePamelaContext(const cfgkill::Context&){
        if(contextHook)return Read(1)?cfgkill::Preparation::Ready:cfgkill::Preparation::Unavailable;
        if(!Read(4))return cfgkill::Preparation::Unavailable;
        contextHook=true;return cfgkill::Preparation::Pending;
    }
    cfgkill::Preparation PrepareInteraction(std::uintptr_t actor,std::uintptr_t pawn){
        using P=cfgkill::Preparation;assert(actor==10&&pawn==4);
        switch(prep){
        case 0:++prep;return P::Pending;
        case 1:if(!Read(21+cost.Context()+8+15)||!Call())return P::Unavailable;++prep;return P::Pending;
        case 2:if(!Read(cost.Scope()+cost.Death()+cost.Known()+TypedHeld()+4))return P::Unavailable;++prep;return P::Pending;
        default:return Read(15)?P::Ready:P::Unavailable;
        }
    }
    template<class Mark>bool FinalInteraction(const cfgkill::Context&,std::uintptr_t,Mark mark){
        if(!Read(15+TypedHeld()+12+1+4+1+1+cost.Predicate())||!Call(2)||
           cfgbundle::ReadCap-reads<768||!mark()||!Call())return false;
        if(!Read(2*(cost.Predicate()+TypedHeld()+18)))return false;
        ++dispatchCount;active=true;return true;
    }
};
}
int main(){
    PamelaBudgetHost h;cfgkill::Runtime r;h.runtime=&r;
    assert(r.policy.PamelaUsed({1,2,3},4,100,true,100,100)&&r.policy.PamelaAccept({1,2,3},101));
    bool dispatched=false;
    for(std::uint64_t now=102;now<120&&!dispatched;++now){
        h.NewPhase();const auto step=r.Tick(h,now);
        assert(step!=cfgkill::Step::Unavailable&&step!=cfgkill::Step::Uncertain);
        dispatched=step==cfgkill::Step::InteractionDispatched;
    }
    assert(dispatched&&h.dispatchCount==1&&!r.policy.interactionAttempted);
    h.NewPhase();assert(r.Tick(h,121)==cfgkill::Step::NativeActive&&r.policy.interactionAttempted);
    h.NewPhase(false);assert(h.maxReads<cfgbundle::ReadCap&&h.maxCalls<=cfgbundle::CallCap);
    std::printf("B1 Pamela shared-budget fixture PASS: peak%u/1536 reads, %u/12 native calls; two predicate callbacks on SAME reader; declared ancestry only.\n",h.maxReads,h.maxCalls);
    return 0;
}
