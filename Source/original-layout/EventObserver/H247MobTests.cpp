#include "../Core/LeanConvergencePolicy.hpp"
#include <cassert>
#include <cstdio>
// H2.47 mob decision fixture (policy only, no game access). Mirrors the donor:
// armed -> fight; unarmed -> grant melee class, else leave on stock tree; Clear (flee) only after fighting.
int main(){
    using cfglean::MobDecide;using cfglean::MobStep;
    // armed on the desired class (or no class known): engage, never re-grant, never flee
    assert(MobDecide(true,true,true,false,false)==MobStep::Engage);
    assert(MobDecide(true,false,false,false,false)==MobStep::Engage);
    // armed with a different class and a desired class available: grant (existing lean behaviour)
    assert(MobDecide(true,false,true,false,false)==MobStep::Grant);
    // grant refused while armed: keep what it holds and fight
    assert(MobDecide(true,false,true,true,false)==MobStep::Engage);
    // unarmed with a grantable class: grant axe/machete
    assert(MobDecide(false,false,true,false,false)==MobStep::Grant);
    // unarmed, grant refused or no class: never fight, never publish flee for a bot never in the mob
    assert(MobDecide(false,false,true,true,false)==MobStep::Hold);
    assert(MobDecide(false,false,false,false,false)==MobStep::Hold);
    // unarmed after fighting (lost weapon): release to the stock flee/loot tree like the donor
    assert(MobDecide(false,false,true,true,true)==MobStep::Release);
    assert(MobDecide(false,false,false,false,true)==MobStep::Release);
    static_assert(CFG_H247_MOB==0||CFG_H247_MOB==1,"boolean flag");
    // Grant-stage bins are validated by the production report compile and capture layout.
    std::printf("H247 MobDecide PASS (flag=%d): armed engage, unarmed grant/hold, release only after fighting.\n",CFG_H247_MOB);
    return 0;
}
