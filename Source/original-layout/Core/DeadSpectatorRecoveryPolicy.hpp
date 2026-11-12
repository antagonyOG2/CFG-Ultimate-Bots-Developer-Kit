#pragma once
#include <cstdint>
#include <array>
namespace cfgspectatorrecovery {
// Scheduling only, never camera/actor authority. Two current dead-target
// observations; one handoff attempt per identity, including ambiguous returns.
struct State {
    std::uintptr_t object=0;int serial=0;std::uint64_t since=0,next=0;bool attempted=false;
    struct Receipt {std::uintptr_t object=0;int serial=0;};
    std::array<Receipt,16> consumed{};unsigned consumedCount=0;
    bool Due(std::uint64_t now){if(now<next)return false;next=now+500;return true;}
    void Lost(){object=0;serial=0;since=0;attempted=false;}
    bool Dead(std::uintptr_t pawn,int identity,std::uint64_t now){
        if(!pawn||identity<=0){Lost();return false;}
        if(object!=pawn||serial!=identity){object=pawn;serial=identity;since=now;attempted=false;return false;}
        for(unsigned i=0;i<consumedCount;++i)if(consumed[i].object==pawn&&consumed[i].serial==identity)return false;
        return consumedCount<consumed.size()&&!attempted&&now>=since&&now-since>=500;
    }
    void Consume(){attempted=true;if(consumedCount<consumed.size())consumed[consumedCount++]={object,serial};}
};
}
