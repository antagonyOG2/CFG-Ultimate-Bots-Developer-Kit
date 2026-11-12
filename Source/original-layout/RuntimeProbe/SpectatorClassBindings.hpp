#pragma once
#include "VehicleBindings.generated.hpp"
namespace cfgspectator::disk {
constexpr std::uintptr_t PawnClassCache=0x0/*CFG3.5 address withheld*/,PawnConstructorCache=0x0/*CFG3.5 address withheld*/;
constexpr std::uintptr_t ControllerClassCache=0x0/*CFG3.5 address withheld*/,ControllerConstructorCache=0x0/*CFG3.5 address withheld*/;
constexpr std::uintptr_t SpectatingPlayer=0x860;
constexpr cfgvehicle::disk::Binding ClientSetSpectatorCamera{0x50A330,0x304ECA8,ControllerClassCache,ControllerConstructorCache,24,24,{} /*CFG3.5 code signature withheld*/};
}
