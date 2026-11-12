#include "../Core/AdditionalJasonJournal.hpp"
#include <cstdio>
#include <initializer_list>
int main(){unsigned checks=0,failed=0;const auto check=[&](bool v){++checks;if(!v)++failed;};
    using namespace cfgadditionaljournal;
    Journal j;check(!j.Primary({1,2})&&!j.Attributed({1,2}));check(!j.Capture({0,2}));
    check(j.Capture({100,200}));check(!j.Capture({300,400}));check(j.primary.pawn==100&&j.primary.state==200);
    check(j.Primary({100,200})&&!j.Primary({100,0}));check(!j.Attributed({300,200}));
    j.Returned(true,300);j.genericState=400;j.killerState=500;
    for(auto pawn:{100u,300u})for(auto state:{0u,200u,400u,500u})check(j.Attributed({pawn,state}));
    for(auto pawn:{0u,600u})check(!j.Attributed({pawn,200}));check(!j.Attributed({300,600}));
    // Model each native publication/unpossession/possession. Restore before
    // yielding and BEFORE native RemoveCharacter's equality guard executes.
    for(auto published: {Pair{300,200},Pair{300,400},Pair{300,0},Pair{300,500},Pair{100,500}}){
        auto live=published;check(j.Attributed(live));live=j.primary;check(j.Primary(live));
        if(live.pawn==j.extra)live.pawn=0; // exact native removal semantics
        check(j.Primary(live));
    }
    Journal null;null.Capture({100,200});null.Returned(true,0);
    check(null.Primary({100,200})&&!null.extra&&!null.uncertain&&!null.Attributed({300,200}));
    Journal exception;exception.Capture({100,200});exception.Returned(false,0);
    check(exception.uncertain&&exception.dispatched&&!exception.Attributed({300,200}));
    check(!j.Primary({0,200})&&!j.Attributed({0,200})); // unexpected post-destroy null is NOT success
    std::printf("F1 immutable primary journal / attributed mutation / pre-destroy restoration:%u checks,%u failures; mocked native effects.\n",checks,failed);
    return failed?1:0;
}
