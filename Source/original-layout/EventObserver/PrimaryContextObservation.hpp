#pragma once
#include "OfflineContextObservation.hpp"
#include "HostedWorldBinding.hpp"
#include "../RuntimeProbe/ScopedLookupEligibility.hpp"
namespace cfgprimary {
// Only the global UObject storage header may be refreshed. The four positive
// identities, human ownership chain, offline world/mode and thread dispatcher
// remain exactly the installation's observations. No pointer/serial adoption.
template<class R> bool RefreshKnownObjectStorage(R& r,const cfgobserver::Plan& saved,cfgobserver::Plan& output) {
    if (!cfgobserver::SanePlan(saved)) return false;
    cfgprobe::lifetime::ArrayHeader fresh{};
    if (!cfgprobe::lifetime::ReadArrayHeader(r,saved.module+cfgprobe::lifetime::ObjectArrayRva,fresh)) return false;
    auto candidate=saved; candidate.objectArray=fresh;
    if (!cfgobserver::ContextUnchanged(r,candidate)) return false;
    output=candidate; return true;
}
// Retain ONLY the previously proved offline world/mode and native owner thread.
// AI can continue after its human dies/escapes; no replacement pawn/controller
// is accepted as a mutation target. Callers must independently revalidate every
// actual AI actor, native registry/resource and no-GC state around native use.
template<class R> bool RetainedOfflineWorld(R& r,const cfgobserver::Plan& saved,cfgobserver::Plan& output){
    using namespace cfgprobe;
    if(!cfgobserver::SanePlan(saved))return false;
    cfgobserver::Plan candidate=saved;
    std::uint8_t flag=0;std::uint32_t thread=0;
    if(!lifetime::ReadArrayHeader(r,saved.module+lifetime::ObjectArrayRva,candidate.objectArray)||
       !cfgobserver::Same(r,saved.module+EngineGlobalCandidate,saved.engine)||
       !cfgobserver::Same(r,saved.engine+EngineGameViewport,saved.viewport)||
       !cfgobserver::Same(r,saved.viewport+ViewportWorld,saved.world)||
       !cfgobserver::Same(r,saved.world+WorldNetDriver,std::uintptr_t{})||
       !cfgobserver::Same(r,saved.world+WorldGameMode,saved.mode)||
       !cfgobserver::Same(r,saved.world+WorldGameInstance,saved.gameInstance)||
       !r.Read(saved.module+dispatch::ThreadFlagRva,flag)||flag!=saved.dispatch.flag||
       !r.Read(saved.module+dispatch::ThreadIdRva,thread)||thread!=saved.dispatch.threadId)return false;
    const std::array<lifetime::SlotObservation,2> identities{{saved.objects[0],saved.objects[1]}};
    if(!lifetime::RevalidateBatch(r,saved.module+lifetime::ObjectArrayRva,candidate.objectArray,identities))return false;
    output=candidate;return true;
}
// H1 (mailbox #349): the ONE retained-world admission for the native Jason driver call sites.
// A saved plan that matches the immutable HostedBinding (decided once, when the hosted plan is
// built) takes the hosted sibling and NEVER falls back to the offline function; every other
// plan takes RetainedOfflineWorld, byte-identical, which still refuses any NetDriver.
template<class R>bool RetainedWorld(R& r,const cfgobserver::Plan& saved,cfgobserver::Plan& output,const HostedBinding& binding){
    if(binding.Matches(saved))return RetainedHostedWorld(r,saved,binding,output);
    return RetainedOfflineWorld(r,saved,output);
}
template<class R>bool RetainedWorld(R& r,const cfgobserver::Plan& saved,cfgobserver::Plan& output){
    return RetainedWorld(r,saved,output,CurrentHostedBinding());
}
// Registry access may outlive the human's possession ONLY inside the exact
// completed-callback lease. This is a context observation, not call authority;
// registry synchronization, GC vetoes and the actual tick proof remain required.
enum class RetentionContext { Human,ScopedWorld };
template<class R>bool RefreshRetentionContext(R& r,const cfgobserver::Plan& saved,
    cfgobserver::Plan& output,RetentionContext kind){
    if(kind==RetentionContext::Human)return RefreshKnownObjectStorage(r,saved,output);
    return kind==RetentionContext::ScopedWorld&&ScopedLookupEligibility<R>::Current(r,saved.module)&&
        RetainedWorld(r,saved,output);
}
}
