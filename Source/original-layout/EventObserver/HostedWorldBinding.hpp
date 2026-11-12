#pragma once
#include "OfflineContextObservation.hpp"
#include "../RuntimeProbe/HostedAuthorityBindings.hpp"
#include "../RuntimeProbe/HostedAuthority35.generated.hpp"
#include <array>
#include <cstdint>

// H1 hosted retention seam (mailbox #349). RetainedOfflineWorld is NOT changed:
// an offline plan can never pass with a NetDriver. A hosted plan is a distinct,
// explicitly bound context that can never pass WITHOUT its exact NetDriver.
//
// cfgobserver::Plan is the fixed binary transfer protocol shared with the
// unchanged injected hook, so its layout cannot grow a "kind" field. The kind is
// instead decided ONCE, when a hosted plan is built, and recorded in a
// HostedBinding that is immutable until retired. RetainedWorld() never infers the
// kind from the presence of a NetDriver; it asks the binding whether this exact
// saved plan (module/world/mode/controller/pawn/identities/thread) is the bound one.
namespace cfgprimary {
enum class PlanKind : std::uint32_t { Offline=0, Hosted=1 };

struct HostedBinding {
    bool active=false;
    cfgobserver::Plan plan{};                 // immutable identity of the hosted plan
    std::uintptr_t netDriver=0;               // typed NetDriver proved at build time
    cfgprobe::lifetime::SlotObservation driver{};
    std::uintptr_t modeClass=0;               // concrete class of the mode (Hunt-derived at build time)
    std::uintptr_t huntClass=0;               // fresh SCGameMode_Hunt class cache value at build time
    bool Matches(const cfgobserver::Plan& p) const noexcept {
        return active&&p.module==plan.module&&p.world==plan.world&&p.mode==plan.mode&&p.controller==plan.controller&&
            p.pawn==plan.pawn&&p.gameInstance==plan.gameInstance&&p.player==plan.player&&p.threadId==plan.threadId&&
            p.objects==plan.objects;
    }
    PlanKind KindOf(const cfgobserver::Plan& p) const noexcept { return Matches(p)?PlanKind::Hosted:PlanKind::Offline; }
    void Retire() noexcept { *this=HostedBinding{}; }
};

// Sibling of RetainedOfflineWorld: every check EXCEPT NetDriver==0, which is replaced by
// the exact saved typed driver (listen host: ServerConnection null, driver->World == world),
// the Hunt mode class, and local authority.
template<class R> bool RetainedHostedWorld(R& r,const cfgobserver::Plan& saved,const HostedBinding& b,cfgobserver::Plan& output){
    using namespace cfgprobe;
    if(!cfgobserver::SanePlan(saved)||!b.Matches(saved)||!ObjectPointer(b.netDriver)||!b.driver.HasAssignedSerial()||
       b.driver.object!=b.netDriver||!ObjectPointer(b.modeClass)||!ObjectPointer(b.huntClass))return false;
    cfgobserver::Plan candidate=saved;
    std::uint8_t flag=0,role=0;std::uint32_t thread=0;
    if(!lifetime::ReadArrayHeader(r,saved.module+lifetime::ObjectArrayRva,candidate.objectArray)||
       !cfgobserver::Same(r,saved.module+EngineGlobalCandidate,saved.engine)||
       !cfgobserver::Same(r,saved.engine+EngineGameViewport,saved.viewport)||
       !cfgobserver::Same(r,saved.viewport+ViewportWorld,saved.world)||
       !cfgobserver::Same(r,saved.world+WorldNetDriver,b.netDriver)||
       !cfgobserver::Same(r,b.netDriver+cfghosted::disk35::DriverWorld,saved.world)||
       !cfgobserver::Same(r,b.netDriver+cfghosted::disk35::ServerConnection,std::uintptr_t{})||
       !cfgobserver::Same(r,saved.world+WorldGameMode,saved.mode)||
       !cfgobserver::Same(r,saved.world+WorldGameInstance,saved.gameInstance)||
       !cfgobserver::Same(r,saved.mode+ObjectClass,b.modeClass)||
       !cfgobserver::Same(r,saved.module+cfghosted::disk35::HuntReflection,b.huntClass)||
       !cfgobserver::Same(r,saved.module+cfghosted::disk35::HuntConstructor,b.huntClass)||
       !r.Read(saved.controller+cfghosted::disk35::ActorRole,role)||role!=cfghosted::disk::AuthorityRole||
       !r.Read(saved.module+dispatch::ThreadFlagRva,flag)||flag!=saved.dispatch.flag||
       !r.Read(saved.module+dispatch::ThreadIdRva,thread)||thread!=saved.dispatch.threadId)return false;
    const std::array<lifetime::SlotObservation,3> identities{{saved.objects[0],saved.objects[1],b.driver}};
    if(!lifetime::RevalidateBatch(r,saved.module+lifetime::ObjectArrayRva,candidate.objectArray,identities))return false;
    output=candidate;return true;
}

inline HostedBinding& CurrentHostedBinding() noexcept { static HostedBinding binding; return binding; }
} // namespace cfgprimary
