#include "../Core/J21cFeatureFlags.hpp"
#include "../Core/LeanConvergencePolicy.hpp"
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <limits>

// Policy fixture, deliberately without game access, native dispatch or writes.
// Production J21LeanDue provides the typed read/identity callback to Step.
int main(){
    using cfglean::SweaterLatched;using cfglean::SweaterScan;
    assert(!SweaterLatched(false,false,true,true));
    assert(!SweaterLatched(false,true,false,true));
    assert(!SweaterLatched(false,true,true,false));
    assert(SweaterLatched(true,false,false,false));
    assert(SweaterLatched(true,true,false,false));
    assert(SweaterLatched(false,true,true,true));
    static_assert(CFG_J3_BOT_SWEATER==0||CFG_J3_BOT_SWEATER==1,"J3 sweater must be boolean");
    assert(SweaterLatched(false,CFG_J3_BOT_SWEATER,true,true)==(CFG_J3_BOT_SWEATER!=0));

    SweaterScan scan;unsigned probes=0;unsigned observed[8]{};
    auto sample=[&](unsigned index){observed[probes++]=index;return index==2;};
    assert(!scan.Step(0,3,sample));assert(probes==1&&observed[0]==0);
    assert(!scan.Step(1,3,sample));assert(!scan.Step(210,3,sample));assert(probes==1);
    assert(!scan.Step(211,3,sample));assert(probes==2&&observed[1]==1);
    assert(!scan.Step(421,3,sample));assert(probes==2);
    assert(scan.Step(422,3,sample));assert(probes==3&&observed[2]==2);
    assert(!scan.Step(633,3,sample));assert(probes==4&&observed[3]==0);
    assert(!scan.Step(844,0,sample));assert(probes==4); // zero roster never advances
    assert(!scan.Step(844,2,sample));assert(probes==5&&observed[4]==1); // roster shrink
    assert(!scan.Step(1055,2,sample));assert(probes==6&&observed[5]==0);

    SweaterScan world2;assert(!world2.Step(0,1,[&](unsigned index){return index!=0;}));
    assert(world2.cursor==0&&world2.next==211); // world reset starts fresh
    SweaterScan failClosed;unsigned attempts=0;
    assert(!failClosed.Step(0,1,[&](unsigned){++attempts;return false;}));
    assert(!failClosed.Step(210,1,[&](unsigned){++attempts;return true;}));assert(attempts==1);
    assert(failClosed.Step(211,1,[&](unsigned){++attempts;return true;}));assert(attempts==2);
    SweaterScan nearMax;
    const auto max=std::numeric_limits<std::uint64_t>::max();
    assert(!nearMax.Step(max-100,1,[&](unsigned){return false;}));assert(nearMax.next==max);
    assert(!nearMax.Step(max-1,1,[&](unsigned){return true;}));
    std::printf("G2 BotSweaterConvergence PASS (compile flag=%d): latch, 211-ms single-bot cursor, wrap, roster change, world reset, callback failure.\n",CFG_J3_BOT_SWEATER);
    return 0;
}
