#pragma once
#include "VehicleBindings.generated.hpp"
namespace cfgswim::disk {
// CFG 3.5 exact-image static evidence; no runtime call authorization.
constexpr cfgvehicle::disk::Binding CanInteractUnderwater{0x4F49B0,0x304D9B8,0x304DE10,0x304DE18,36,40,{} /*CFG3.5 code signature withheld*/};
constexpr std::uintptr_t CanInteractUnderwaterExecutor=0x0/*CFG3.5 address withheld*/,CanInteractUnderwaterBody=0x0/*CFG3.5 address withheld*/;
constexpr std::array<unsigned char,289> CanInteractUnderwaterCode{} /*CFG3.5 code signature withheld*/;
constexpr std::array<unsigned char,117> CanInteractUnderwaterBodyCode{} /*CFG3.5 code signature withheld*/;
constexpr cfgvehicle::disk::Binding OnInteractUnderwater{0x4F5610,0x304DBB0,0x304DE10,0x304DE18,9,16,{} /*CFG3.5 code signature withheld*/};
constexpr std::uintptr_t OnInteractUnderwaterExecutor=0x0/*CFG3.5 address withheld*/,OnInteractUnderwaterBody=0x0/*CFG3.5 address withheld*/;
constexpr std::array<unsigned char,192> OnInteractUnderwaterCode{} /*CFG3.5 code signature withheld*/;
constexpr std::array<unsigned char,67> OnInteractUnderwaterBodyCode{} /*CFG3.5 code signature withheld*/;
constexpr std::uintptr_t WaterVolume=0xE48,ContextActor=0x1628;
}
