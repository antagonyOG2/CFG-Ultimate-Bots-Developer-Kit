#pragma once
#include <cstddef>
#include <cstdint>

// Historical hosted-spectator fixture for the pre-3.5 image only.
// Verify-CfgHostedSpectatorBindings.ps1 checks that legacy disk snapshot; it is
// not a CFG 3.5 binding source. No callable adapter or hook admission follows.
namespace cfghosted::disk {
constexpr char HistoricalImageSHA256[]="53B91EA6BC2D9EC69F5AAF41EB67ECB4E5393E71F8AD4B3276F2D74BC27E4BC6";
constexpr std::uintptr_t CanSpectateFactory=0x0/*CFG3.5 address withheld*/;
constexpr std::uintptr_t CanSpectateExecutor=0x0/*CFG3.5 address withheld*/;
constexpr std::uintptr_t HuntVtable=0x0/*CFG3.5 address withheld*/,HuntCanSpectate=0x0/*CFG3.5 address withheld*/;
constexpr std::uintptr_t SCControllerVtable=0x0/*CFG3.5 address withheld*/,BaseControllerVtable=0x0/*CFG3.5 address withheld*/;
constexpr std::uintptr_t PreviousExecutor=0x0/*CFG3.5 address withheld*/,PreviousBody=0x0/*CFG3.5 address withheld*/;
constexpr std::uintptr_t ViewableSelector=0x0/*CFG3.5 address withheld*/,ViewAPlayer=0x0/*CFG3.5 address withheld*/;
constexpr unsigned CanSpectateSlot=0x620,SelectorSlot=0xC10,ViewAPlayerSlot=0xC18;
// Reflected CanSpectate parameter packet. The native virtual instead accepts
// RCX=mode, RDX=viewer controller, R8=target PlayerState and returns bool in AL.
struct CanSpectateFrame {
    std::uint64_t viewer=0,target=0;
    std::uint8_t result=0,padding[7]{};
};
static_assert(sizeof(CanSpectateFrame)==24&&offsetof(CanSpectateFrame,target)==8&&
    offsetof(CanSpectateFrame,result)==16,"CFG owned CanSpectate frame");
// Selector is NOT a reflected UFunction: native RCX=controller, EDX=signed
// int32 direction, RAX=selected PlayerState/null. Never pass an FFrame here.
constexpr char SelectorSHA256[]="5B61A0CA8E5640748851CA0C570C0BEAFF8AF7992DC1D3FFE1330F3E375A7760";
constexpr unsigned SelectorBytes=463;
constexpr char HuntCanSpectateSHA256[]="56DDEFC954B3F83C94B48ADF8FEB8828AC0A3DF6BB5175A73AF2B98D5FEB343D";
constexpr unsigned HuntCanSpectateBytes=592;
}
