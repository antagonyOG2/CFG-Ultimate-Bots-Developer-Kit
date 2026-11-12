#include "../RuntimeProbe/OfflineLifecycleBindings.generated.hpp"
#include "NativeOfflineLifecycleCalls.hpp"
#include "NativeOfflineCounselorBirth.hpp"
#include "NativeOfflineStartup.hpp"
template class cfgmenu::offlinepopulation::Birth<cfgmenu::promotion::CurrentProcessReader>;
template class cfgmenu::offlinestartup::Startup<cfgmenu::promotion::CurrentProcessReader>;
// Compile every adapter path without executing native game code.
bool CompileLifecycleAdapter(cfgmenu::offlinelifecycle::Calls& calls,cfgmenu::promotion::CurrentProcessReader& reader){
    return calls.Current(reader)&&calls.RestartSelectedHuman(reader)&&calls.ParentIntro(reader)&&calls.ParentMatchStart(reader);
}
namespace b = cfgmenu::offlinelifecycledisk;
static_assert(!b::RuntimeRouteApproved && !b::IntroSemanticNameVerified);
static_assert(b::RestartPlayerExecutorCode.size() == 132);
static_assert(b::OfflineBotsRestartPlayerCode.size() == 100);
// Split unwind entries must be represented by complete spans.
static_assert(b::ParentRestartPlayerCode.size() == 0x117);
static_assert(b::SandboxRestartPlayerCode.size() == 0x8FF);
static_assert(b::OfflineIntro958Code.size() == 122);
static_assert(b::OfflineBotsIntro958Code.size() == 1619);
static_assert(sizeof(b::RestartPlayerParameters) == 8);
static_assert(b::StartingPlayerIsShared && !b::MatchStartedSemanticNameVerified);
static_assert(b::OfflineStartingPlayer630 == b::OfflineBotsStartingPlayer630);
static_assert(b::OfflineStartingPlayer630 == b::SandboxStartingPlayer630);
static_assert(b::OfflineStartingPlayer630 == b::SharedStartingPlayer630);
static_assert(b::SharedStartingPlayer630Code.size() == 142);
static_assert(b::StartingPlayerExecutorCode.size() == 132);
static_assert(b::OfflineMatchStartedCode.size() == 0x26E);
static_assert(b::OfflineBotsMatchStartedCode.size() == 0x5ED);
static_assert(b::SandboxMatchStartedCode.size() == 0x515);
int main() { return 0; }
