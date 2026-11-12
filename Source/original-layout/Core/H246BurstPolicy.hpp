#pragma once
#include <cmath>
#include <cstdint>
// H2.46: let the existing J2.19 Shift foot-burst and 20 m / 7 s counselor Morph run while
// the donor movement lease is published. One shared implementation; the lease only changes
// admission (this header) and whether the donor driver is told to stand down (bit 9).
#ifndef CFG_H267_BURST_ANY_RANGE
#define CFG_H267_BURST_ANY_RANGE 1 // burst at any range over 4 m while in pursuit; same 3 s and 7 s cooldown
#endif
#ifndef CFG_H246_BURST
#define CFG_H246_BURST 1
#endif
#ifndef CFG_H246_TELEPORT_CONFIRM
#define CFG_H246_TELEPORT_CONFIRM 1
#endif
namespace cfgh246 {
// Burst wanted: 4 m..20 m, not owned, ready. Lease only blocks when the flag is off.
inline bool LeaseAdmits(bool leasePublished){return !leasePublished||CFG_H246_BURST;}
inline bool BurstWanted(bool leasePublished,float squared,bool owned,std::uint64_t now,std::uint64_t readyAt){
    return LeaseAdmits(leasePublished)&&std::isfinite(squared)&&squared>=400.f*400.f&&(CFG_H267_BURST_ANY_RANGE||squared<=2000.f*2000.f)&&!owned&&now>=readyAt;
}
// Cheap pre-check (no native reads) so the Morph queue, which raises external bit 10 for the
// donor driver, is only entered when the service can actually act. Superset of
// cfgpursuit::MorphDue: structure-stall is optimistic when the stall tracker says stalled.
struct MorphQueueInputs {
    bool leasePublished=false,optionalProbe=false,probeMorphFirst=false,donorNavPending=false,donorTrapIntent=false;
    bool morphDueSuperset=false,doorGraceOver=false,retryOver=false,moveOver=false,cabinsReady=false;
};
inline bool MorphQueue(const MorphQueueInputs& i){
    if(!i.leasePublished)return i.optionalProbe&&i.probeMorphFirst; // J2.19 offline behaviour unchanged
    if(!CFG_H246_BURST)return false;
    // Never overlap a donor native ring in flight or the donor's own trap-victim Morph.
    return i.optionalProbe&&i.probeMorphFirst&&!i.donorNavPending&&!i.donorTrapIntent&&
        i.morphDueSuperset&&i.doorGraceOver&&i.retryOver&&i.moveOver&&i.cabinsReady;
}
}
