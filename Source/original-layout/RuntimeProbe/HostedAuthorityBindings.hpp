#pragma once
#include <cstdint>
namespace cfghosted::disk {
// Verify-CfgHostedAuthorityBindings.ps1: pinned CFG named class registration
// plus constructor cache loads; no donor RVA or runtime activation implied.
constexpr std::uintptr_t HuntReflection=0x0/*CFG3.5 address withheld*/,HuntConstructor=0x0/*CFG3.5 address withheld*/;
constexpr std::uintptr_t WorldReflection=0x0/*CFG3.5 address withheld*/,WorldConstructor=0x0/*CFG3.5 address withheld*/;
constexpr std::uintptr_t NetDriverReflection=0x0/*CFG3.5 address withheld*/,NetDriverConstructor=0x0/*CFG3.5 address withheld*/;
constexpr std::uintptr_t ActorRole=0x110;
constexpr std::uint8_t AuthorityRole=3;
}
