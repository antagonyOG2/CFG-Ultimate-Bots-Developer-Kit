#pragma once
#include "J21WorkPolicy.hpp"
#include <array>
namespace cfgj21 {
// Approved269 controller-tick epoch; deliberately NOT named engine frame.
struct EpochBudget {
    std::uintptr_t world=0;std::uint64_t epoch=0,used=0,primaryTicks=0,extraTicks=0,peFallbacks=0,deferred=0;
    double primarySeconds=0,extraSeconds=0;unsigned source=2,electedSource=2;Cost frames{};bool bothAlive=false;
    std::uint64_t handoffDeferrals=0;
    // Completed controller epochs with no core PE opportunity. This is a
    // scheduling diagnostic, not proof that an engine callback was available.
    std::uint64_t peStarvedEpochs=0,peOpportunities=0;bool peSeen=false;
    std::array<std::uint64_t,2> lastServiced{{UINT64_MAX,UINT64_MAX}};
    std::array<unsigned,2> roundRobin{{0,0}};
    std::array<std::uint64_t,2> lastOwedHunt{{0,0}};std::uint64_t owedHunts=0;
    void Advance(unsigned by){if(epoch){frames.Observe(used);if(!peSeen)++peStarvedEpochs;}
        ++epoch;used=0;source=by;peSeen=false;}
    void CorePE(){++peOpportunities;peSeen=true;}
    void Tick(std::uintptr_t currentWorld,unsigned role,bool primaryOwnedAlive,float delta){
        if(world!=currentWorld){*this={};world=currentWorld;}
        if(role==0){++primaryTicks;if(delta>0&&delta<10)primarySeconds+=delta;}
        else{++extraTicks;if(delta>0&&delta<10)extraSeconds+=delta;}
        // Exactly one elected source. The non-advancing Jason shares it; source
        // handoff is reported and can be checked against live tick/delta counts.
        if((primaryOwnedAlive&&role==0)||(!primaryOwnedAlive&&role==1)){
            // No verified global frame exists. On source change defer ONE
            // advancing turn rather than let both controllers reset a shared
            // budget in the handoff frame. Next elected Tick advances normally.
            if(epoch&&electedSource<2&&electedSource!=role){electedSource=role;++handoffDeferrals;return;}
            electedSource=role;Advance(role);
        }
    }
    void Fallback(std::uintptr_t currentWorld,bool anyOwnedAlive){
        if(world!=currentWorld){*this={};world=currentWorld;}
        if(!anyOwnedAlive){++peFallbacks;Advance(2);}
    }
    void Charge(std::uint64_t us){used+=us;}
    bool Optional(unsigned role){
        if(used>=1000||lastServiced[role]==epoch){++deferred;return false;}
        if(bothAlive&&((epoch&1u)!=role)){++deferred;return false;}
        lastServiced[role]=epoch;return true;
    }
    bool Slice(){if(used>=2000){++deferred;return false;}return true;}
    bool OwedHunt(unsigned role,std::uint64_t now,std::uint64_t decision,bool bound){
        if(bound&&(now<decision||now-decision<1000))return false;
        if(lastOwedHunt[role]&&(now<lastOwedHunt[role]||now-lastOwedHunt[role]<100))return false;
        lastOwedHunt[role]=now;++owedHunts;return true;
    }
    unsigned Choose(unsigned role,unsigned dueMask){
        if(!dueMask||!Optional(role))return 3;
        for(unsigned n=0;n<3;++n){const auto lane=(roundRobin[role]+n)%3;
            if(dueMask&(1u<<lane)){roundRobin[role]=(lane+1)%3;return lane;}}
        return 3;
    }
};
// Cache only positive class identity/name observations, no root/GC witness,
// volatile state, failures or native-call capability. Epoch alone is NOT enough:
// callback Begin/End and every native attempt invalidate all callback entries.
template<class Slot,class Name,unsigned Capacity=64>struct NameMemo {
    struct Entry {Slot slot{};Name name{};bool used=false;};
    std::array<Entry,Capacity> entries{};std::uintptr_t world=0;bool active=false;
    void Clear(){entries={};}void Begin(std::uintptr_t w){world=w;active=true;Clear();}
    void End(){Clear();active=false;world=0;}
    bool Find(std::uintptr_t w,std::uintptr_t object,Name& name){if(!active||world!=w)return false;
        for(const auto& e:entries)if(e.used&&e.slot.object==object){name=e.name;return true;}return false;}
    void Put(const Slot& slot,const Name& name){if(!active||!slot.HasAssignedSerial())return;
        for(auto& e:entries)if(!e.used){e={slot,name,true};return;}}
};
}
