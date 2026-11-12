#pragma once
#include "OfflineLifecycleBindings.generated.hpp"
#include "CounselorBirthBindings.generated.hpp"
#include "NativePopulationBindings.generated.hpp"
namespace cfghosted::birthdisk {
// Verify-CfgHostedBirthBindings.ps1 ties these slots to the named Hunt class.
// Crucially Hunt uses the ALREADY pinned parent RestartPlayer, not OfflineBots.
constexpr std::uintptr_t HuntVtable=0x0/*CFG3.5 address withheld*/;
constexpr std::uintptr_t Restart=0x0/*CFG3.5 address withheld*/,RestartAtStart=0x0/*CFG3.5 address withheld*/,Getter=0x0/*CFG3.5 address withheld*/;
constexpr unsigned RestartSlot=0x738,RestartAtStartSlot=0x740,GetterSlot=0x640;
constexpr unsigned PlayerArray=0x380,BotFlags=0x394,BotMask=4;
constexpr unsigned CurrentKiller=0x420,CurrentKillerState=0x490;
constexpr std::uintptr_t BotSetter=0x0/*CFG3.5 address withheld*/;
constexpr std::array<unsigned char,8> BotSetterCode{} /*CFG3.5 code signature withheld*/;
using RestartParameters=cfgmenu::offlinelifecycledisk::RestartPlayerParameters;
static_assert(sizeof(RestartParameters)==8);
// SpawnActor, constructor, soft-class ConsumeActive and getter code arrays are
// reused from their exact CFG generated headers above, NEVER donor RVAs.
// No primary-killer handoff/replication approval is implied by these bindings.
}
