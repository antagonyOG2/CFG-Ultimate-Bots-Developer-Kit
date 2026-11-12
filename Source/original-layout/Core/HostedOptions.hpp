#pragma once
#include <array>
#include <cstdint>
namespace cfghostedoptions {
// Process-lifetime, owner-thread choices. These are preferences, never authority.
struct Values {
    unsigned jason=0,difficulty=0,fill=0,jasonCount=1;
    bool Valid()const noexcept{return jason<=1&&difficulty<=3&&fill<=1&&jasonCount>=1&&jasonCount<=2;}
    bool operator==(const Values& b)const noexcept{return jason==b.jason&&difficulty==b.difficulty&&fill==b.fill&&jasonCount==b.jasonCount;}
    unsigned Difficulty()const noexcept{return difficulty?difficulty-1:1;}
    unsigned Mask()const noexcept{return jason|(fill<<1);}
};
struct Store {
    Values values{};std::uint64_t revision=0;
    bool Save(const Values& next)noexcept{
        if(!next.Valid())return false;
        if(!(next==values)){values=next;++revision;}return true;
    }
};
inline Store& Current()noexcept{static Store choices;return choices;}
// One match takes an immutable snapshot; menu edits cannot change an in-flight
// birth or replay it. A different world must acquire its own host admission.
struct Match {
    Values values{};std::uintptr_t world=0;std::uint64_t revision=0;
    bool Capture(std::uintptr_t next,bool listenHostHuntInProgress,const Store& store)noexcept{
        if(!next||!listenHostHuntInProgress||!store.values.Valid())return false;
        if(world==next)return true;
        world=next;values=store.values;revision=store.revision;return true;
    }
    void Retire()noexcept{*this={};}
};
inline bool WaitingForMatchBadge(bool storedJasonOn,bool disarmed,bool armed,bool binding,bool adopted)noexcept{
    return storedJasonOn&&!disarmed&&!armed&&!binding&&!adopted;
}
}
