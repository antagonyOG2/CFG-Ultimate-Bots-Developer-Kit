#pragma once
#include <array>
#include <cstdint>

// H2.43 hosted killer-presentation copy (donor OfflineSetup.cpp:8828 CopyLifecycleKillerPresentation).
// PlayOutro needs PickedKillerSkin/PickedKillerWeapon on GameState.KillerPlayerState; the AI Jason's
// PlayerState has them empty until the two stock setters run.
//
// CFG_H243_PRESENTATION=1:
//   * roots hold ONLY UObjects. The H2.41 roots also listed two native CODE addresses
//     (SetSkin/SetWeapon + module) that ValidateBundleRootValues can never accept, so the
//     retention (clause 35) refused every copy. Code stays pinned by cfgbundle::Code (clauses 7/8).
//   * target/source slot identity is exact slot equality WITHOUT the positive-serial requirement
//     (cfgjason::OwnedComponentSlot): a newborn AI PlayerState may keep serial 0 and CFG must never
//     allocate one (EndingRuntimeMembers.hpp:104). Index, object and serial must still be unchanged.
//   * the copy runs after the adoption receipt is published (HostedH1ServicePrimaryPresentation), not
//     inside the takeover birth.
// CFG_H243_PRESENTATION=0 restores the H2.41 behaviour exactly (8 roots, strict SameSlot, in-birth copy).
#ifndef CFG_H243_PRESENTATION
#define CFG_H243_PRESENTATION 1
#endif
// H2.44: CFG's profile skin setter requires an ILLPlayerController, so it is a
// no-op on the AI Jason's PlayerState (live H2.43: setter returned, field stayed
// null, clause 53). Mirror the offline J2.19 bridge
// (EventObserver/OfflineEndingPresentation.hpp): after the native call, if the
// field is still null, publish the already-proven concrete skin with a null-only
// CAS, only on the adopted AI Jason's own bot PlayerState. Weapon stays native.
#ifndef CFG_H244_SKIN_BRIDGE
#define CFG_H244_SKIN_BRIDGE 1
#endif

namespace cfghostedpresent {
#if CFG_H243_PRESENTATION
inline std::array<std::uintptr_t,6> Roots(std::uintptr_t targetState,std::uintptr_t sourceState,std::uintptr_t sourceController,
    std::uintptr_t skin,std::uintptr_t skinClass,std::uintptr_t selectedClass,std::uintptr_t,std::uintptr_t)noexcept{
    return {targetState,sourceState,sourceController,skin,skinClass,selectedClass};
}
#else
inline std::array<std::uintptr_t,8> Roots(std::uintptr_t targetState,std::uintptr_t sourceState,std::uintptr_t sourceController,
    std::uintptr_t skin,std::uintptr_t skinClass,std::uintptr_t selectedClass,std::uintptr_t setSkin,std::uintptr_t setWeapon)noexcept{
    return {targetState,sourceState,sourceController,skin,skinClass,selectedClass,setSkin,setWeapon};
}
#endif

// Post-possession pre-native retries: any refusal before the first native entry is retry-safe.
// Clause 47/48 are pre-native admission; 49+ means a setter may have been entered (never replayed).
constexpr unsigned LastRetrySafeClause=48;
constexpr unsigned MaxPreNativeAttempts=6;
inline bool RetrySafe(unsigned clause)noexcept{return clause&&clause<=LastRetrySafeClause;}

// Ordered tail of HostedCopyKillerPresentation (clauses 35..66), host supplies every effect. Returns 0 on
// success or the refused clause number. The order and clause numbers are unchanged from H2.41.
//   Host: bool Retain(); bool World(); bool TargetSlot(); bool SourceSlot(); bool SourceOwner(); bool SourceController();
//         bool TargetCurrent(); bool SourceSkinUnchanged(); bool SourceWeaponUnchanged(bool initial);
//         bool NativeAllowed(); bool Begin(); int CallSkin() (0 returned,1 not entered,2 exception); bool Finish();
//         bool SkinApplied(); int CallWeapon(); bool WeaponApplied();
template<class Host> unsigned Sequence(Host& h,bool useSelectedDefaults)noexcept{
    if(!h.Retain())return 35;
    if(!h.World())return 36;
    if(!h.TargetSlot())return 37;
    if(!h.SourceSlot())return 38;
    if(!h.SourceOwner())return 39;
    if(!h.SourceController())return 40;
    if(!h.TargetCurrent())return 41;
    if(!useSelectedDefaults){
        if(!h.SourceSkinUnchanged())return 42;
        if(!h.SourceWeaponUnchanged(true))return 44;
        if(!h.SourceSkinUnchanged())return 46;
    }
    if(!h.NativeAllowed())return 47;
    if(!h.Begin())return 48;
    const int skin=h.CallSkin();
    const bool skinFinished=h.Finish();
    if(skin!=0)return 49;
    if(!skinFinished)return 50;
    if(!h.TargetSlot())return 51;
    if(!h.SkinApplied())return 53;
    if(!h.Retain())return 54;
    if(!h.SourceOwner())return 55;
    if(!h.SourceController())return 56;
    if(!h.TargetCurrent())return 57;
    if(!useSelectedDefaults&&!h.SourceWeaponUnchanged(false))return 59;
    if(!h.NativeAllowed())return 60;
    if(!h.Begin())return 61;
    const int weapon=h.CallWeapon();
    const bool weaponFinished=h.Finish();
    if(weapon!=0)return 62;
    if(!weaponFinished)return 63;
    if(!h.TargetSlot())return 64;
    if(!h.WeaponApplied())return 66;
    return 0;
}
}
