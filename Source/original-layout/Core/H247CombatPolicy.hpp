#pragma once
#include <array>
#include <cstdint>
#include "DonorJasonPursuit.hpp"
// H2.47 combat fixes. One shared implementation each; nothing here duplicates a J2.19/donor rule.
#ifndef CFG_H247_MORPH_BUSY
#define CFG_H247_MORPH_BUSY 1
#endif
#ifndef CFG_H247_SEAT_WAIT
#define CFG_H247_SEAT_WAIT 1
#endif
namespace cfgh247 {
// Values the legacy (non native-Tick) hunt lane can observe without a donor Owner. The donor busy
// predicate is the SAME template (cfgdonorhunt::DonorIsJasonBridgeCombatBusy); only the host seam differs.
struct LegacyBusyView {
    std::uintptr_t jason=0;bool actorReadable=false;
    cfgdonorhunt::BusyState busy{};cfgdonorhunt::CombatState combat{};
    bool lockedReadable=true,locked=false;      // not observable from the legacy lane: treated as not locked
    bool specialReadable=false,special=false;   // legacy grab/kill predicate (IsAttemptingGrab||IsGrabKilling)
    bool flagsReadable=false;std::array<std::uint8_t,4> flags{};
    bool stunReadable=false,stunned=false,heldReadable=false;std::uintptr_t held=0;
};
struct LegacyBusyHost {
    using Position=cfgdonorhunt::Position;
    LegacyBusyView& v;cfgdonorhunt::ChaseState chase{};
    explicit LegacyBusyHost(LegacyBusyView& view):v(view){chase.Jason=view.jason;}
    auto& Chase(){return chase;}auto& Busy(){return v.busy;}auto& Combat(){return v.combat;}
    bool ReadableActor(std::uintptr_t a){return v.actorReadable&&a==v.jason;}
    bool GetLockedJasonInteractable(std::uintptr_t,bool& readable){readable=v.lockedReadable;return v.locked;}
    bool IsJasonInNativeSpecialMove(std::uintptr_t,bool& readable){readable=v.specialReadable;return v.special;}
    bool ReadBusyFlags(std::uintptr_t,std::array<std::uint8_t,4>& f){f=v.flags;return v.flagsReadable;}
    bool ModuleReady(){return true;}
    bool SafeIsStunned(std::uintptr_t,bool& s){s=v.stunned;return v.stunReadable;}
    bool SafeGetGrabbedCounselor(std::uintptr_t,std::uintptr_t& h){h=v.held;return v.heldReadable;}
    std::uintptr_t FindNearestJasonAICounselorTarget(std::uintptr_t,std::uintptr_t=0){return 0;}
    bool GetJasonAIActorLocation(std::uintptr_t,Position&){return false;}
};
// True when Jason is mid kill/grab/attack (donor "combat busy"); lease-mode legacy Morph and burst must not start.
inline bool LegacyCombatBusy(LegacyBusyView& view,std::uint64_t now,std::uint32_t* causes=nullptr){
    LegacyBusyHost host(view);
    return cfgdonorhunt::DonorIsJasonBridgeCombatBusy(host,now,false,causes);
}
// Lease-mode queue/service/burst admission. Offline (no lease) is J2.19-unchanged: never vetoed here.
inline bool MorphBusyVeto(bool leasePublished,bool busy){return CFG_H247_MORPH_BUSY&&leasePublished&&busy;}
}
