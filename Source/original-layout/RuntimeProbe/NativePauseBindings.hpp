#pragma once
#include <array>
#include <cstdint>
#include "Compatibility/CfgBuildProfile.hpp"
namespace cfgspectator::pausedisk {
constexpr std::uintptr_t Query=0x0/*CFG3.5 address withheld*/,Engine=cfgcompat::cfg35::engine::GlobalCandidateRva;
constexpr std::array<unsigned char,47> QueryCode{} /*CFG3.5 code signature withheld*/;
}
