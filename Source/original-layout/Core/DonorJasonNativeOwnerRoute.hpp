#pragma once
namespace cfgdonorhunt {
enum class NativeOwnerRoute : unsigned { Refuse, OfflineProof, HostedAdoption };
// A hosted miss never falls through to the offline linked-owner proof.
constexpr NativeOwnerRoute SelectNativeOwnerRoute(bool hostedRoutine,bool hostedAdoptionCurrent) noexcept {
    return !hostedRoutine?NativeOwnerRoute::OfflineProof:
        (hostedAdoptionCurrent?NativeOwnerRoute::HostedAdoption:NativeOwnerRoute::Refuse);
}
}
