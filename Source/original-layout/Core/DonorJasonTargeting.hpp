#pragma once
#include <array>
#include <cfloat>
#include <cmath>
#include <cstdint>
namespace cfgdonorhunt {
struct Position {float X=0,Y=0,Z=0;};
// Transcribed active rc1 FEAT 697-698,770-853,4780-4899.
// Entries are weak observations. Host must re-prove native actor/controller
// identity/living/world before each read; this list does not retain UObjects.
struct Targets {
    static constexpr std::int32_t JasonAITargetCapacity=64;
    std::array<std::uintptr_t,JasonAITargetCapacity> actors{};
    std::array<std::uint64_t,JasonAITargetCapacity> blockedUntil{};
    std::array<std::uint8_t,JasonAITargetCapacity> blockStrikes{};
    std::int32_t count=0;
    std::int32_t DonorFindJasonAITargetIndex(std::uintptr_t actor)const{
        if(!actor)return -1;
        for(std::int32_t i=0;i<count;++i)if(actors[static_cast<unsigned>(i)]==actor)return i;
        return -1;
    }
    bool DonorIsJasonAITargetBlocked(std::uintptr_t actor,std::uint64_t now)const{
        const auto index=DonorFindJasonAITargetIndex(actor);
        return index>=0&&blockedUntil[static_cast<unsigned>(index)]>now;
    }
    void DonorBlockJasonAITarget(std::uintptr_t actor,std::uint64_t now){
        const auto index=DonorFindJasonAITargetIndex(actor);if(index<0)return;
        const auto i=static_cast<unsigned>(index);const std::uint8_t strikes=++blockStrikes[i];
        blockedUntil[i]=now+(strikes>=2?600000ULL:90000ULL);
    }
    template<class Host>void DonorRegisterJasonAITarget(Host& host,std::uintptr_t actor){
        if(!actor||!host.ReadableActor(actor))return;
        for(std::int32_t i=0;i<count;++i){if(actors[static_cast<unsigned>(i)]==actor)return;}
        if(count>=JasonAITargetCapacity)return;
        actors[static_cast<unsigned>(count++)]=actor;
    }
    template<class Host>bool DonorHasAlternativeUsableJasonAITarget(Host& host,std::uintptr_t excludedTarget){
        const auto now=host.Now();
        for(std::int32_t i=0;i<count;++i){
            const auto candidate=actors[static_cast<unsigned>(i)];
            if(!candidate||candidate==excludedTarget||host.IsJasonAITargetBlocked(candidate,now)||
               !host.ReadableActor(candidate))continue;
            // Donor Pawn.Controller raw access is replaced by the exact
            // validated 3.5 host accessor; no donor field offset copied.
            if(!host.ReadablePawnController(candidate))continue;
            return true;
        }
        return false;
    }
    template<class Host>std::uintptr_t DonorFindNearestJasonAICounselorTarget(Host& host,
        std::uintptr_t jason,std::uintptr_t excluded=0){
        if(!jason||!host.ReadableActor(jason))return 0;
        Position jasonLocation{};
        if(!host.GetJasonAIActorLocation(jason,jasonLocation))return 0;
        const auto now=host.Now();
        std::uintptr_t bestTarget=0;float bestDistanceSquared=FLT_MAX;
        for(std::int32_t i=0;i<count;++i){
            const auto candidate=actors[static_cast<unsigned>(i)];
            if(!candidate||candidate==jason||candidate==excluded||host.IsJasonAITargetBlocked(candidate,now)||
               !host.ReadableActor(candidate))continue;
            if(!host.ReadablePawnController(candidate))continue;
            Position candidateLocation{};
            if(!host.GetJasonAIActorLocation(candidate,candidateLocation))continue;
            const float dx=candidateLocation.X-jasonLocation.X;
            const float dy=candidateLocation.Y-jasonLocation.Y;
            const float dz=candidateLocation.Z-jasonLocation.Z;
            const float distanceSquared=(dx*dx)+(dy*dy)+(dz*dz);
            if(!std::isfinite(distanceSquared)||distanceSquared<0.0f)continue;
            if(!bestTarget||distanceSquared<bestDistanceSquared){bestTarget=candidate;bestDistanceSquared=distanceSquared;}
        }
        return bestTarget;
    }
};
}
