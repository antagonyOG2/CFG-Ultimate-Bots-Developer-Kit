#pragma once
#include <array>
#include <cstdint>
namespace cfgradioobjective::disk {
// CFG3.5: named SCGameState.CBRadio property, registration and instance vtable.
// Verify-CfgRadioObjectiveBindings.ps1 also checks these active consumer pins.
constexpr std::uintptr_t Cache=0x588,GetterSlot=0x788,Getter=0x0/*CFG3.5 address withheld*/;
constexpr std::uintptr_t GetterFactory=0x0/*CFG3.5 address withheld*/,GetterCache=0x0/*CFG3.5 address withheld*/,Executor=0x0/*CFG3.5 address withheld*/;
constexpr std::array<unsigned char,8> GetterCode{} /*CFG3.5 code signature withheld*/;
constexpr std::array<unsigned char,48> ExecutorCode{} /*CFG3.5 code signature withheld*/;
}
