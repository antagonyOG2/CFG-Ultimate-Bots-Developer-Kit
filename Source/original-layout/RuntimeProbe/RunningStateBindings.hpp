#pragma once
#include "VehicleBindings.generated.hpp"
namespace cfgrun::disk {
constexpr cfgvehicle::disk::Binding ServerSetRunning{0x4B7280,0x304A200,0x304A290,0x304A298,1,1,{} /*CFG3.5 code signature withheld*/};
constexpr std::uintptr_t Executor=0x0/*CFG3.5 address withheld*/,Implementation=0x0/*CFG3.5 address withheld*/,Toggle=0x0/*CFG3.5 address withheld*/,Running=0xE45,SetBit=0x0/*CFG3.5 address withheld*/;
constexpr std::array<unsigned char,182> ExecutorCode{} /*CFG3.5 code signature withheld*/;
constexpr std::array<unsigned char,121> ImplementationCode{} /*CFG3.5 code signature withheld*/;
constexpr std::array<unsigned char,114> ToggleCode{} /*CFG3.5 code signature withheld*/;
constexpr std::array<unsigned char,8> SetBitCode{} /*CFG3.5 code signature withheld*/;
}
