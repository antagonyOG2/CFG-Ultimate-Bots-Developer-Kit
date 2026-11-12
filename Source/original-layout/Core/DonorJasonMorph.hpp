#pragma once
#include "DonorJasonChase.hpp"
namespace cfgdonorhunt {
inline std::uint64_t DonorJasonAIMorphReadyAt(const ChaseState& s){
    if(s.LastMorphTeleportAt==0)return 0;
    return s.LastMorphTeleportAt+s.MorphCooldownMs;
}
inline std::uint64_t DonorJasonAIMorphRemainingMs(const ChaseState& s,std::uint64_t now){
    const auto ready=DonorJasonAIMorphReadyAt(s);
    return ready==0||now>=ready?0:ready-now;
}
inline void DonorMarkJasonAIMorphTeleportUsed(ChaseState& s,std::uint64_t now){
    s.LastMorphTeleportAt=now;s.NextDistanceTeleportAt=now+s.MorphCooldownMs;
}
// Active FEAT6437-6678; candidate ORDER, dimensions and vertical/horizontal
// rejection rules preserved. Host resolves/proves K2_TeleportTo28byte frame.
template<class Host>bool DonorRunJasonAINavRingTeleportOnGameThread(Host& host,
    std::uintptr_t jason,std::uintptr_t target){
    if(!jason||!host.ReadableActorAndClass(jason)||!target)return false;
    Position targetLocation{};
    if(!host.GetJasonAIActorLocation(target,targetLocation)||!host.TeleportFunction(jason))return false;
    struct Offset {float X,Y;};
    constexpr Offset offsets[]={{2200,0},{-2200,0},{0,2200},{0,-2200},
        {1556,1556},{1556,-1556},{-1556,1556},{-1556,-1556},
        {3000,0},{-3000,0},{0,3000},{0,-3000},
        {2121,2121},{2121,-2121},{-2121,2121},{-2121,-2121}};
    const float targetGroundZ=targetLocation.Z-90.0f;
    const Position extent{350,350,110};
    for(const auto& offset:offsets){
        Position candidate=targetLocation,projected{};
        candidate.X+=offset.X;candidate.Y+=offset.Y;candidate.Z=targetGroundZ+25.0f;
        if(!host.ProjectJasonAINavPointOnGameThread(candidate,extent,projected))continue;
        const float verticalDelta=std::fabs(projected.Z-targetGroundZ);
        if(!std::isfinite(verticalDelta)||verticalDelta>120.0f)continue;
        const float dx=projected.X-targetLocation.X,dy=projected.Y-targetLocation.Y;
        const float horizontal=std::sqrt(dx*dx+dy*dy);
        if(!std::isfinite(horizontal)||horizontal<1700.0f)continue;
        bool result=false;
        if(!host.SafeTeleport(jason,projected,result)||!result)continue;
        return true;
    }
    return false;
}
template<class Host>bool DonorRunOfflineBotsInitialTeleportOnGameThread(Host& host){
    auto& s=host.Chase();const auto target=host.FindNearestJasonAICounselorTarget(s.Jason);
    if(!DonorRunJasonAINavRingTeleportOnGameThread(host,s.Jason,target))return false;
    s.Target=target;return true;
}
inline void DonorResetChaseAfterMorph(ChaseState& s){
    s.PathLocked=false;s.PathLockUntil=0;s.LastAcceptedMoveAt=0;
    s.ConsecutiveStuckChecks=0;s.ConsecutiveNearTargetStuckDeferrals=0;s.HaveLastLocation=false;
}
// Active FEAT6689-6850, plus FJB19944-20003 below. No distance bypasses the
// shared20second recharge. Failed native navigation retries after3seconds.
template<class Host>bool DonorRunJasonAIDistanceTeleportOnGameThread(Host& host){
    auto& s=host.Chase();if(!s.Active||!s.Jason||!host.ReadableActor(s.Jason))return false;
    const auto target=host.FindNearestJasonAICounselorTarget(s.Jason);if(!target)return false;
    Position origin{},destination{};
    if(!host.GetJasonAIActorLocation(s.Jason,origin)||!host.GetJasonAIActorLocation(target,destination))return false;
    const float dx=destination.X-origin.X,dy=destination.Y-origin.Y,dz=destination.Z-origin.Z;
    const float distance=std::sqrt(dx*dx+dy*dy+dz*dz);
    if(!std::isfinite(distance)||distance<=9000.0f)return false;
    const auto now=host.Now();
    if(DonorJasonAIMorphRemainingMs(s,now)>0||now<s.NextDistanceTeleportAt)return false;
    if(!host.RunOfflineBotsInitialTeleportOnGameThread()){
        if(DonorNavigationPending(host,0))return true;
        s.NextDistanceTeleportAt=now+3000;return false;
    }
    host.MarkJasonAIMorphTeleportUsed();DonorResetChaseAfterMorph(s);return true;
}
template<class Host>bool DonorTrySharedFiftyMeterHuntMorph(Host& host,std::uint64_t now,unsigned* refusal=nullptr){
    auto& s=host.Chase();
    if(refusal)*refusal=0;
    const auto reject=[&](unsigned reason){if(refusal)*refusal=reason;return false;};
    if(!s.Active||host.World()!=s.World||!s.InitialTeleportAttempted||host.StartupTrapSetupActive()||
       host.KnifeSequenceActive()||host.DoorBreakActive()||now<s.DoorTraversalGraceUntil||
       now<s.NextDistanceTeleportAt||DonorJasonAIMorphRemainingMs(s,now)!=0)return reject(1);
    // The donor's static sampled-world/Jason/nextProbe state is per-Jason,
    // epoch-reset state here, so the two controllers cannot suppress each other.
    if(now<s.nextFiftyMeterProbeAt)return reject(2);
    s.nextFiftyMeterProbeAt=now+500;
    const auto target=host.FindNearestJasonAICounselorTarget(s.Jason);Position origin{},destination{};
    if(!target||!host.GetJasonAIActorLocation(s.Jason,origin)||!host.GetJasonAIActorLocation(target,destination))return reject(3);
    const float dx=destination.X-origin.X,dy=destination.Y-origin.Y,dz=destination.Z-origin.Z;
    const float squared=dx*dx+dy*dy+dz*dz;
    if(!std::isfinite(squared)||squared<=5000.0f*5000.0f||squared>9000.0f*9000.0f)return reject(4);
    std::uint32_t busyCauses=0;
    if(host.IsJasonBridgeCombatBusy(now,false,&busyCauses))return reject(5);
    if(!host.RunOfflineBotsInitialTeleportOnGameThread()){
        if(DonorNavigationPending(host,0))return true;
        s.NextDistanceTeleportAt=now+3000;return reject(6);
    }
    host.MarkJasonAIMorphTeleportUsed();DonorResetChaseAfterMorph(s);return true;
}
}
