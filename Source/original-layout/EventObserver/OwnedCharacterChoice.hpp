#pragma once
#include "../RuntimeProbe/SoftClassMarshaling.generated.hpp"
#include <array>
namespace cfgmenu::choices {
struct alignas(softdisk::Alignment) Packet {std::array<unsigned char,softdisk::PacketBytes> bytes{};};
static_assert(sizeof(Packet)==40&&alignof(Packet)==8);
enum class Result {Refused,Submitted,Indeterminate,CleanupFailed};
// Synchronous normal-native marshaling seam. Host must validate exact current
// widget/player-state/function/property/code identity and native owner thread.
// This is NOT a raw memcpy ownership adapter or an asset loader. Host native
// methods use the proven Initialize/Copy/Destroy property ABI; never delete[]
// native allocation or destroy borrowed catalogue/preview byte snapshots.
// Native request submission is not proof of successful saved selection/spawn.
template<class Host> Result SubmitPreview(Host& host,bool explicitUserChoice){
    if(!explicitUserChoice||!host.CurrentChoice()||!host.BindingsReady())return Result::Refused;
    Packet packet;
    if(!host.Initialize(packet))return host.Faulted()?Result::Indeterminate:Result::Refused;
    // Getter creates an OWNED40-byte return. Request borrows it synchronously,
    // independently copies its own temporaries and never consumes our storage.
    const bool copied=host.GetPreview(packet);
    bool submitted=false;
    if(copied&&!host.Faulted()&&host.CurrentChoice())submitted=host.RequestChoice(packet);
    // A native exception can leave allocation state indeterminate. Quarantine
    // rather than attempting a second native call/double-free on unknown state.
    if(host.Faulted())return Result::Indeterminate;
    if(!host.Destroy(packet))return host.Faulted()?Result::Indeterminate:Result::CleanupFailed;
    return submitted?Result::Submitted:Result::Refused;
}
}
