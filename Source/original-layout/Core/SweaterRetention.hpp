#pragma once
#include <cstdint>
namespace cfgsweater {
// CFG's native activation-completion callback clears the GC-traced pawn
// SweaterAbility property and ability owner, but does not destroy the ability.
// Keep ONLY that earned ownership link through the donor's cooldown. This is
// not a grant, a used-byte reset, a montage replay, a root or a new ability.
struct Retention {
    std::uintptr_t pawn=0,ability=0;
    bool earned=false,used=false;
};
template<class Host> bool RetainConsumed(Host& h,const Retention& t){
    if(!t.pawn||!t.ability||!t.earned||!t.used||!h.Fresh(t))return false;
    if(!h.Cleared(t)||!h.Reserve())return false;
    // No retry after either exchange is attempted. Native replacement wins.
    return h.Owner(t)&&h.Ability(t)&&h.Readback(t);
}
}
