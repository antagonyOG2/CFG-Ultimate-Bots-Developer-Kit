#pragma once
#include <cstdint>
#include <cmath>
namespace cfggrab {
// Donor stock bridge's close-combat alternation and one equipped kill per hold.
// Native eligibility and native pocket-knife transitions always retain control.
struct Policy {
    std::uintptr_t held=0;unsigned decisions=0,slot=0,nextSlot=0,probed=0;bool committed=false;
    std::uint64_t readyAt=0,nextProbe=0;
    bool ChooseGrab(float squared){
        if(!std::isfinite(squared)||squared<0||squared>260.f*260.f)return false;
        ++decisions;return squared<=220.f*220.f&&decisions%2==0;
    }
    void Observe(std::uintptr_t pawn,std::uint64_t now){
        if(pawn==held)return;held=pawn;committed=false;slot=nextSlot;probed=0;readyAt=pawn?now+300:0;nextProbe=readyAt;
    }
    bool Due(std::uint64_t now)const{return held&&!committed&&now>=readyAt&&now>=nextProbe;}
    void Refused(std::uint64_t now){slot=(slot+1)%4;++probed;nextProbe=now+(probed%4?250:2000);}
    void Commit(){committed=true;nextSlot=(slot+1)%4;} // before dispatch, including an ambiguous exception
};
}
