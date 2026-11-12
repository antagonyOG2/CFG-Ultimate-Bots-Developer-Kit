#include "PamelaFinisherEligibility.hpp"
#include "../Core/PamelaFinisherPolicy.hpp"
#include <cassert>
#include <cstdio>
using namespace cfgkill;
static volatile char hunter=0;
static std::uintptr_t allowed=11;
static unsigned calls=0;
static int __fastcall Native(void*,void*,const EligibilityVector*,const EligibilityVector*){
    ++calls;assert(hunter==1&&allowed==22);return 3;
}
static int __fastcall Refuse(void* c,void* p,const EligibilityVector* l,const EligibilityVector* d){
    Native(c,p,l,d);return 0;
}
static int __fastcall Fault(void* c,void* p,const EligibilityVector* l,const EligibilityVector* d){
    Native(c,p,l,d);RaiseException(0xE0424242,0,0,nullptr);return 0;
}
static bool FaultCall(){
    bool caught=false;
    __try{CallEligibilityWithHeldWeapon(Fault,nullptr,nullptr,nullptr,nullptr,&hunter,&allowed,11,22);}
    __except(EXCEPTION_EXECUTE_HANDLER){caught=true;}return caught;
}
static int __fastcall Unmodified(void*,void*,const EligibilityVector*,const EligibilityVector*){
    ++calls;assert(hunter==0&&allowed==33);return 0;
}
struct Candidates {
    bool human=true;unsigned humanChecks=0,aiChecks=0;
    bool PamelaCandidate(unsigned i,std::uintptr_t& pawn,std::uintptr_t& weapon,Weapon& kind){
        if(!i){++humanChecks;if(!human)return false;}else ++aiChecks;
        pawn=i?100+i:4;weapon=6;kind=Weapon::Other;return true;
    }
};
int main(){
    for(const auto original:{Native,Refuse}){
        hunter=0;allowed=11;calls=0;
        const auto result=CallEligibilityWithHeldWeapon(original,nullptr,nullptr,nullptr,nullptr,&hunter,&allowed,11,22);
        assert(result==(original==Native?3:0)&&calls==1&&hunter==0&&allowed==11);
    }
    hunter=0;allowed=11;calls=0;
    assert(FaultCall()&&calls==1&&hunter==0&&allowed==11);
    hunter=1;allowed=11;calls=0;
    assert(CallEligibilityWithHeldWeapon(Native,nullptr,nullptr,nullptr,nullptr,&hunter,&allowed,11,22)==3);
    assert(hunter==1&&allowed==11&&calls==1);hunter=0;
    allowed=33;calls=0;
    assert(!CallEligibilityWithHeldWeapon(Unmodified,nullptr,nullptr,nullptr,nullptr,&hunter,&allowed,11,22));
    assert(!hunter&&allowed==33&&calls==1);
    Policy p;assert(p.PamelaUsed({1,2,3},4,100,true,100,100)&&p.PamelaAccept({1,2,3},101));
    p.finisher=4;p.weapon=6;assert(p.ObserveContext(10,102,false));
    EligibilityTicket t{{1,2,3},10,12,4,true,true,true,true,true};
    assert(PamelaOverrideAllowed(p,t,12,4,103,4));
    assert(!PamelaOverrideAllowed(p,t,99,4,103,4)); // different component
    t.context=99;assert(!PamelaOverrideAllowed(p,t,12,4,103,4));t.context=10;
    assert(!PamelaOverrideAllowed(p,t,12,4,103,999)); // stock-only AI fallback
    assert(!PamelaOverrideAllowed(p,t,12,5,103,4));
    assert(!PamelaOverrideAllowed(p,t,12,4,p.until,4));
    p.pamela=false;assert(!PamelaOverrideAllowed(p,t,12,4,103,4));
    Candidates c;unsigned cursor=8;std::uintptr_t pawn=0,weapon=0;Weapon kind{};
    for(unsigned i=0;i<4;++i){assert(SelectPamelaFinisher(c,4,cursor,pawn,weapon,kind)&&pawn==4);}
    assert(c.humanChecks==4&&!c.aiChecks); // closer AI never steals human's action
    c.human=false;assert(SelectPamelaFinisher(c,4,cursor,pawn,weapon,kind)&&pawn==101&&c.aiChecks==1);
    c.human=true;assert(SelectPamelaFinisher(c,4,cursor,pawn,weapon,kind)&&pawn==4&&c.aiChecks==1);
    std::puts("B1 scoped Pamela finisher PASS: normal/refusal/SEH restore, preexisting Hunter, changed-list refusal, foreign context/window/finisher, human preferred each phase, stock-only AI fallback.");
}
