#pragma once
#include <cstdint>
namespace cfgdonorhunt {
enum class BusyCause : unsigned {
    NoJason,ActorUnreadable,KnifeSequence,AttackPressed,GrabKillReady,DoorBreak,
    StartupTrapSetup,StartupTrapAttempt,StartupTrapCount,StartupTrapPhone,
    LockedInteractable,LockedReadFailure,NativeSpecialMove,SpecialMoveReadFailure,
    BusyFlagsReadFailure,BusyFlag0,BusyFlag1,BusyFlag2,BusyFlag3,
    StunnedReadFailure,GrabbedReadFailure,Stunned,GrabbedCounselor,
    NearbyCounselor,RecentNearbyLatch,Count
};
constexpr std::uint32_t BusyCauseBit(BusyCause cause)noexcept{
    return std::uint32_t{1}<<static_cast<unsigned>(cause);
}
}
