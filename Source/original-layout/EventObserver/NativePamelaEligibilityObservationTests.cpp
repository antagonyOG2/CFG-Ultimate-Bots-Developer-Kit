#include "KillRuntime.hpp"
#include "PamelaFinisherEligibility.hpp"
#include <cassert>
#include <cstdio>
using namespace cfgkill;
static volatile char hunter=0;static std::uintptr_t allowedClass=11;static unsigned forwarded=0;
static int NativePredicate(void*,void*,const EligibilityVector*,const EligibilityVector*){
    ++forwarded;return hunter==1&&allowedClass==22?7:0;
}
struct PredicateHost {
    struct {std::uintptr_t world=1,module=0;}plan;
    struct {struct {std::uintptr_t object=2;std::int32_t serial=3;}pawn;}bundleJason;
    Runtime bundleKill;bool observed=true,jasonCurrent=true,held=true,eligibilityCurrent=true;
    Context context{10,11,12,false};unsigned fabricatedScopes=0;
    bool Scope(Identity&){++fabricatedScopes;return false;} // no completed-tick lease exists in this native callback
    bool SummaryObservationScope(){return observed;}
    bool Jason(){return jasonCurrent;}
    bool DeathContext(Context& out){out=context;return true;}
    bool PamelaHeld(std::uintptr_t,std::uintptr_t& out){out=6;return held;}
    bool EligibilityCurrent(std::uintptr_t,std::uintptr_t,bool nativeInteractor){return nativeInteractor&&eligibilityCurrent;}
    bool PamelaEligibilityCall(const Policy& policy,std::uintptr_t component,std::uintptr_t pawn,
        EligibilityOriginal original,const EligibilityVector* location,const EligibilityVector* direction,int& result,
        const EligibilityTicket* ticket){
        if(!ticket||!PamelaNativeOverrideAllowed(policy,*ticket,component,pawn,GetTickCount64()))return false;
        result=CallEligibilityWithHeldWeapon(original,reinterpret_cast<void*>(component),reinterpret_cast<void*>(pawn),
            location,direction,&hunter,&allowedClass,11,22);return true;
    }
#include "NativePamelaEligibilityObservationMembers.hpp"
};
int main(){unsigned checks=0;const auto check=[&](bool ok){++checks;assert(ok);};
    PredicateHost h;const auto now=GetTickCount64();
    check(h.bundleKill.policy.PamelaUsed({1,2,3},4,now-100,true,100,100));
    check(h.bundleKill.policy.ObservePamelaNativeContext({1,2,3},10,now-50,false));
    int result=0;
    check(h.NativePamelaEligibility(12,4,NativePredicate,nullptr,nullptr,result)&&result==7);
    check(!hunter&&allowedClass==11&&forwarded==1&&!h.fabricatedScopes&&!h.bundleKill.accepted);
    check(h.NativePamelaEligibility(12,99,NativePredicate,nullptr,nullptr,result)&&result==7&&forwarded==2);
    check(!hunter&&allowedClass==11); // temporary changes restored for every verified counselor
    check(!h.NativePamelaEligibility(999,4,NativePredicate,nullptr,nullptr,result)&&forwarded==2);
    for(unsigned fault=0;fault<6;++fault){
        h.observed=h.jasonCurrent=h.held=h.eligibilityCurrent=true;h.context.active=false;h.bundleJason.pawn.serial=3;
        switch(fault){case 0:h.observed=false;break;case 1:h.jasonCurrent=false;break;case 2:h.held=false;break;
            case 3:h.eligibilityCurrent=false;break;case 4:h.context.active=true;break;case 5:++h.bundleJason.pawn.serial;break;}
        check(!h.NativePamelaEligibility(12,4,NativePredicate,nullptr,nullptr,result)&&forwarded==2&&!hunter&&allowedClass==11);
    }
    h.observed=h.jasonCurrent=h.held=h.eligibilityCurrent=true;h.context.active=false;h.bundleJason.pawn.serial=3;
    h.bundleKill.policy.Expire(h.bundleKill.policy.until);
    check(!h.NativePamelaEligibility(12,4,NativePredicate,nullptr,nullptr,result)&&forwarded==2);
    std::printf("KP0 actual engine-origin eligibility member: %u checks PASS; female/other counselor/native forwarding, no fabricated tick lease, refusal/expiry and restoration.\n",checks);
}
