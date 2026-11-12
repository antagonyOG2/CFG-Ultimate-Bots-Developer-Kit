#pragma once
#include "../KillBindings.generated.hpp"
#include "../RestrictedLoadoutBindings.generated.hpp"
namespace cfgcompat::cfg35::pamelafinisher {
// Exact4FC4BB54 image: SCContextKillComponent.WeaponRequirements array
// descriptor2D8B4F8 (kind22,dim1,offset358), class inner2D8B530(kind17).
// Eligibility356E30 full456-byte proof already in KillBindings:
// 356EC1 loads+358,356EC8 loads+360;356F80 tests counselor+1A60.
constexpr std::uintptr_t WeaponRequirements=0x358,WeaponRequirementsCount=0x360;
constexpr std::uintptr_t WeaponRequirementsDescriptor=0x0/*CFG3.5 address withheld*/,WeaponRequirementsInnerDescriptor=0x0/*CFG3.5 address withheld*/;
constexpr std::uintptr_t Hunter=cfgkill::disk::bIsHunter;
constexpr std::uintptr_t HeldGetter=0x0/*CFG3.5 address withheld*/;
inline constexpr auto EquippedItem=cfgloadout::restricteddisk::EquippedItem;
constexpr std::array<unsigned char,75> HeldGetterCode{} /*CFG3.5 code signature withheld*/;
}
