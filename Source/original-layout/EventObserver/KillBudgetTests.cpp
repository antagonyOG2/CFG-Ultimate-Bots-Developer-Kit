#include "KillRuntime.hpp"
#include "../Core/GameplayBundlePolicy.hpp"
#include <cassert>
#include <cstdio>
// Production reader-call cost model, NOT a claim about observed live ancestry.
// Every ObjectSlot costs4; ObjectIs costs2+2*depth; Context costs36+2*stateDepth;
// LinkedKiller costs40+2*(pawn+controller+path depths). Native code-array comparison
// is one Read irrespective of byte length. This fixture chooses ordinary shallow
// class depths and the longer (tracked-bot) roster path, then drives the actual
// Runtime with the outer BundlePhase guards charged to the same1536/12 budget.
namespace {
struct Cost {
    unsigned state=5,jason=8,controller=6,path=3,counselor=8,counselorAI=6,weapon=6,actor=4,interact=5,kill=4,level=2;
    static unsigned Is(unsigned depth){return 2+2*depth;}
    unsigned Context()const{return 36+2*state;}
    unsigned Jason()const{return 40+2*(jason+controller+path);}
    unsigned Scope()const{return Context()+Jason();}
    unsigned Known()const{return 15+2*(counselor+counselorAI);}
    unsigned WeaponType()const{return 24+Is(weapon);}
    unsigned CurrentWeapon()const{return Known()+21+2+Is(counselor)+WeaponType()+5;}
    unsigned ActorWorld()const{return 14+Is(level);}
    unsigned Death()const{return Jason()+1+2+10+Is(actor)+ActorWorld()+2+10+Is(interact)+10+Is(kill)+7;}
    unsigned Predicate()const{return 17+Known()+7;}
};
struct Host {
    Cost cost{};cfgkill::Runtime* runtime=nullptr;
    unsigned reads=0,calls=0,maxReads=0,maxCalls=0,stances=0,interactions=0,writes=0,prep=0;
    bool stanceReady=false,weaponCold=true,aliveContext=false,active=false,requireFinish=false;
    bool RequiresFinishingStrike()const{return requireFinish;}
    bool Read(unsigned n){if(n>cfgbundle::ReadCap-reads)return false;reads+=n;return true;}
    bool Call(unsigned n=1){if(n>cfgbundle::CallCap-calls)return false;calls+=n;return true;}
    void NewPhase(bool outer=true){maxReads=(std::max)(maxReads,reads);maxCalls=(std::max)(maxCalls,calls);reads=calls=0;if(outer)assert(Read(cost.Scope()));}
    bool Scope(cfgkill::Identity& id){id={1,2,3};return Read(cost.Scope());}
    bool KnownLiving(std::uintptr_t p){return p==4&&Read(cost.Known());}
    bool MaskOff(bool& off){off=true;return Read(2);}
    bool DamageEvent(std::uintptr_t p){return p==8&&Read(1+7*8+12*2);} // own class +8class headers +2function headers
    bool ReadDamage(std::uintptr_t p,cfgkill::DamagePacket& out){out={12,0,7,5,4};return p==9&&Read(1);}
    bool DamageCounselor(std::uintptr_t p,std::uintptr_t& out){out=4;return p==5&&Read(cost.Known()-1+2);}
    bool CurrentWeapon(std::uintptr_t p,std::uintptr_t& out,cfgkill::Weapon& kind){
        out=6;kind=cfgkill::Weapon::Axe;
        if(p!=4||!Read(cost.CurrentWeapon())||!Call())return false;
        if(weaponCold){weaponCold=false;
            // Cold factory Context + second ancestry + worst128-byte names for
            // every weapon ancestor. Name decoder is bounded28reads per name.
            return Read(cost.Context()+Cost::Is(cost.weapon)+28*cost.weapon)&&Call();}
        return true;
    }
    bool DamageTypeMatches(std::uintptr_t p,std::uintptr_t type){return p==6&&type==7&&Read(cost.WeaponType()+12);}
    bool DistanceSquared(std::uintptr_t,float& out){out=100;return Read(12)&&Call(2);}
    bool DeathContext(cfgkill::Context& out){out=aliveContext?cfgkill::Context{10,11,12,active}:cfgkill::Context{};
        return Read(aliveContext?cost.Death():cost.Jason()+1);}
    cfgkill::Preparation PrepareStance(){
        if(stanceReady)return Read(16)?cfgkill::Preparation::Ready:cfgkill::Preparation::Unavailable;
        if(!Read(32+cost.Context())||!Call())return cfgkill::Preparation::Unavailable;
        stanceReady=true;return cfgkill::Preparation::Pending;
    }
    template<class Mark> bool DispatchStance(Mark mark){
        if(!Read(cost.Scope()+16+2+2)||reads>=cfgbundle::ReadCap||calls>=cfgbundle::CallCap)return false;
        ++writes;if(!Read(1)||!mark()||!Call())return false;++stances;aliveContext=true;return true;
    }
    cfgkill::Preparation PrepareInteraction(std::uintptr_t actor,std::uintptr_t pawn){
        using P=cfgkill::Preparation;if(actor!=10||pawn!=4)return P::Unavailable;
        switch(prep){
        case 0:++prep;return P::Pending;
        case 1:if(!Read(21+cost.Context()+8+15)||!Call())return P::Unavailable;++prep;return P::Pending;
        case 2:{cfgkill::Identity id{};cfgkill::Context death{};std::uintptr_t weapon=0;cfgkill::Weapon kind{};
            if(!Scope(id)||!DeathContext(death)||!KnownLiving(pawn)||!CurrentWeapon(pawn,weapon,kind)||!Read(1+3))return P::Unavailable;
            ++prep;return P::Pending;}
        default:return Read(15)?P::Ready:P::Unavailable;
        }
    }
    template<class Mark> bool FinalInteraction(const cfgkill::Context&,std::uintptr_t,Mark mark){
        float squared=0;
        if(!Read(15)||!DistanceSquared(4,squared)||!Read(1+4+1+1+cost.Predicate())||
            cfgbundle::ReadCap-reads<334||calls>=cfgbundle::CallCap||!mark()||!Call())return false;
        // Two native predicate callbacks, both on THIS reader; no reset/new host.
        if(!Read(2*cost.Predicate()))return false;++interactions;active=true;return true;
    }
};
}
int main(){
    Host h;cfgkill::Runtime runtime;h.runtime=&runtime;
    h.NewPhase();assert(runtime.SweaterUsed(h,4,100));
    h.NewPhase(false);assert(runtime.ObserveDamage(h,2,8,9,101,true));const auto damageReads=h.reads;
    h.NewPhase();assert(runtime.Tick(h,102)==cfgkill::Step::Waiting);assert(!h.writes&&!h.stances);
    h.NewPhase();assert(runtime.Tick(h,103)==cfgkill::Step::StanceDispatched);const auto stanceReads=h.reads;
    h.NewPhase();assert(runtime.Tick(h,104)==cfgkill::Step::Waiting);
    h.NewPhase();assert(runtime.Tick(h,105)==cfgkill::Step::Waiting);
    h.NewPhase();assert(runtime.Tick(h,106)==cfgkill::Step::Waiting);
    h.NewPhase();assert(runtime.Tick(h,107)==cfgkill::Step::InteractionDispatched);const auto interactionReads=h.reads;
    h.NewPhase();assert(runtime.Tick(h,108)==cfgkill::Step::NativeActive);h.NewPhase(false);
    assert(h.stances==1&&h.interactions==1&&h.writes==1&&h.maxReads<cfgbundle::ReadCap&&h.maxCalls<=cfgbundle::CallCap);
    assert(h.cost.Predicate()==67&&h.cost.Scope()==120); // declared shallow fixture, not live-game measurements
    std::printf("CFG kill production-cost fixture PASS: damage=%u stance=%u interaction=%u peak=%u/1536 calls=%u/12\n",
        damageReads,stanceReads,interactionReads,h.maxReads,h.maxCalls);
    {Host finish;cfgkill::Runtime twoHits;finish.runtime=&twoHits;finish.requireFinish=true;
     finish.NewPhase(false);assert(twoHits.SweaterUsed(finish,4,100));
     finish.NewPhase(false);assert(twoHits.ObserveDamage(finish,2,8,9,101,true));
     finish.NewPhase();assert(twoHits.Tick(finish,102)==cfgkill::Step::Waiting);
     finish.NewPhase();assert(twoHits.Tick(finish,103)==cfgkill::Step::StanceDispatched);
     finish.NewPhase();assert(twoHits.Tick(finish,104)==cfgkill::Step::ContextReady&&!finish.interactions);
     finish.NewPhase(false);assert(twoHits.ObserveDamage(finish,2,8,9,105,true));const auto finishingReads=finish.reads;
     for(std::uint64_t now=106;now<=110&&!finish.interactions;++now){finish.NewPhase();twoHits.Tick(finish,now);}
     finish.NewPhase(false);assert(finish.interactions==1&&finish.maxReads<cfgbundle::ReadCap&&finish.maxCalls<=cfgbundle::CallCap);
     std::printf("CFG second native melee edge cost=%u/1536; full two-strike path peak=%u calls=%u/12\n",finishingReads,finish.maxReads,finish.maxCalls);
    }
    // Max-depth supported metadata is still allowed to refuse under1536, never
    // silently enlarge budgets or bridge after the proof budget is exhausted.
    Host deep;cfgkill::Runtime blocked;deep.runtime=&blocked;
    deep.cost.state=deep.cost.jason=deep.cost.controller=deep.cost.path=32;
    deep.cost.counselor=deep.cost.counselorAI=deep.cost.weapon=32;deep.weaponCold=false;deep.stanceReady=true;
    blocked.policy.SweaterUsed({1,2,3},4,100,true);
    cfgkill::Hit hit{{1,2,3},4,5,6,cfgkill::Weapon::Axe,12,100,true,true,true,true};assert(blocked.policy.Accept(hit,101));
    deep.NewPhase();const auto result=blocked.Tick(deep,102);
    assert(result==cfgkill::Step::Uncertain||result==cfgkill::Step::Unavailable);
    assert(deep.reads<=1536&&!deep.writes&&!deep.stances&&!blocked.policy.stanceAttempted&&!blocked.policy.Exclusive(103));
}
