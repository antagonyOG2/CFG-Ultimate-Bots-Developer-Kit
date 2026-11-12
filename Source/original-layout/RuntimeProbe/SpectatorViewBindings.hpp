#pragma once
#include "CameraBindings.generated.hpp"
#include "SpectatorNativeBindings.generated.hpp"
namespace cfgspectatorview::disk {
constexpr std::uintptr_t ControllerCamera=0x400,PCOwner=0x368,Active=0xEA0,Pending=0x1410;
// One generated, exact-image constructor/vtable proof owns the code pin.
constexpr std::uintptr_t Getter=cfgspectator::disk::NativeViewTarget;
constexpr const auto& GetterCode=cfgspectator::disk::NativeViewTargetCode;
}
