#pragma once
#include "DonorJasonTargeting.hpp"
namespace cfgdonorhunt {
// Active FJB5838-6379. All entries are weak bookkeeping; native host accessors
// must prove slots, world/type, property schema and reciprocal possession.
struct CounselorRoster:Targets {
    std::array<std::uintptr_t,JasonAITargetCapacity> controllers{};
    std::int32_t controllerCount=0;
    struct PendingDeath {std::uintptr_t Pawn=0;std::uint64_t ReadyAt=0;};
    std::array<PendingDeath,JasonAITargetCapacity> pendingDeaths{};
    std::uintptr_t LocalPlayerController=0,LocalCounselorTarget=0,Target=0,TrapPriorityVictim=0;
    std::uint64_t NextLocalRefreshAt=0,NextPruneAt=0,NextRosterRefreshAt=0,HumanPursuitStartedAt=0,
        TrapPriorityUntil=0,NextKillTeamTickAt=0;
    unsigned RosterRefreshLevel=0;
    bool LocalCounselorDeathEvidence=false,TrapPriorityMorphCompleted=false,FinalCounselorDeathObserved=false;
    void DonorRemoveCounselorTarget(std::uintptr_t target){
        if(!target)return;
        const auto index=DonorFindJasonAITargetIndex(target);
        if(index>=0){
            for(auto i=index;i+1<count;++i){const auto at=static_cast<unsigned>(i);
                actors[at]=actors[at+1];blockedUntil[at]=blockedUntil[at+1];blockStrikes[at]=blockStrikes[at+1];}
            if(count>0)--count;
            const auto last=static_cast<unsigned>(count);actors[last]=0;blockedUntil[last]=0;blockStrikes[last]=0;
        }
        if(Target==target)Target=0;
        if(TrapPriorityVictim==target){TrapPriorityVictim=0;TrapPriorityUntil=0;TrapPriorityMorphCompleted=false;}
    }
    template<class Host>bool DonorIsValidatedLiveCounselorPawn(Host& host,std::uintptr_t actor){
        if(!actor||!host.ReadableActor(actor)||!host.IsCounselorCharacter(actor))return false;
        float health=0;
        if(!host.ReadCounselorHealth(actor,health)||!std::isfinite(health)||health<=0)return false;
        std::uintptr_t controller=0;std::uint8_t dead=0;
        if(!host.ReadPawnController(actor,controller)||!controller||!host.ReadableController(controller)||
           !host.ReadCounselorDead(actor,dead)||dead!=0||host.HasCounselorEscaped(actor))return false;
        std::uintptr_t possessed=0;
        return host.ReadControllerPawn(controller,possessed)&&possessed==actor;
    }
    template<class Host>void DonorTrackCounselorController(Host& host,std::uintptr_t counselor){
        if(!counselor||!host.ReadableActor(counselor))return;
        std::uintptr_t controller=0;
        if(!host.ReadPawnController(counselor,controller)||!controller||!host.ReadableController(controller))return;
        for(std::int32_t i=0;i<controllerCount;++i)if(controllers[static_cast<unsigned>(i)]==controller)return;
        if(controllerCount<JasonAITargetCapacity)controllers[static_cast<unsigned>(controllerCount++)]=controller;
    }
    template<class Host>void DonorRegisterRespawnedCounselorsFromKnownControllers(Host& host){
        for(std::int32_t i=0;i<controllerCount;++i){
            const auto controller=controllers[static_cast<unsigned>(i)];
            if(!controller||!host.ReadableController(controller))continue;
            std::uintptr_t pawn=0;
            if(!host.ReadControllerPawn(controller,pawn)||!DonorIsValidatedLiveCounselorPawn(host,pawn))continue;
            DonorRegisterJasonAITarget(host,pawn);
        }
    }
    template<class Host>void DonorRefreshLocalCounselorTarget(Host& host,std::uint64_t now){
        if(now<NextLocalRefreshAt)return;
        NextLocalRefreshAt=now+250;
        if(!LocalPlayerController||!host.ReadableController(LocalPlayerController))return;
        std::uintptr_t possessed=0;
        if(!host.ReadControllerPawn(LocalPlayerController,possessed))return;
        const auto previous=LocalCounselorTarget;
        const bool previousEscaped=previous&&host.HasCounselorEscaped(previous);
        if(previous&&(previousEscaped||possessed!=previous||!DonorIsValidatedLiveCounselorPawn(host,previous))){
            float health=0;
            if(!previousEscaped&&host.ReadCounselorHealth(previous,health)&&std::isfinite(health)&&health<=0)
                LocalCounselorDeathEvidence=true;
            DonorRemoveCounselorTarget(previous);LocalCounselorTarget=0;HumanPursuitStartedAt=0;
        }
        if(possessed&&possessed!=LocalCounselorTarget&&DonorIsValidatedLiveCounselorPawn(host,possessed)){
            LocalCounselorTarget=possessed;LocalCounselorDeathEvidence=false;
            DonorRegisterJasonAITarget(host,possessed);NextKillTeamTickAt=0;
        }
    }
    template<class Host>void DonorPruneAndRefreshCounselorTargets(Host& host,std::uintptr_t world,std::uint64_t now){
        if(now>=NextPruneAt){
            NextPruneAt=now+1000;
            for(std::int32_t i=0;i<count&&i<JasonAITargetCapacity;++i)DonorTrackCounselorController(host,actors[static_cast<unsigned>(i)]);
            std::int32_t writeIndex=0;
            for(std::int32_t readIndex=0;readIndex<count&&readIndex<JasonAITargetCapacity;++readIndex){
                const auto ri=static_cast<unsigned>(readIndex);const auto target=actors[ri];
                if(!DonorIsValidatedLiveCounselorPawn(host,target)){
                    if(host.IsNativeCounselorDeathConfirmed(target)){
                        for(auto& pending:pendingDeaths){if(pending.Pawn==target)break;
                            if(!pending.Pawn){pending={target,now+15000};break;}}
                    }
                    if(Target==target)Target=0;if(LocalCounselorTarget==target)LocalCounselorTarget=0;
                    if(TrapPriorityVictim==target){TrapPriorityVictim=0;TrapPriorityUntil=0;TrapPriorityMorphCompleted=false;}
                    continue;
                }
                const auto wi=static_cast<unsigned>(writeIndex);
                if(wi!=ri){actors[wi]=target;blockedUntil[wi]=blockedUntil[ri];blockStrikes[wi]=blockStrikes[ri];}
                ++writeIndex;
            }
            for(auto i=writeIndex;i<JasonAITargetCapacity;++i){const auto at=static_cast<unsigned>(i);
                actors[at]=0;blockedUntil[at]=0;blockStrikes[at]=0;}
            count=writeIndex;DonorRegisterRespawnedCounselorsFromKnownControllers(host);
            const bool live=count>0||DonorIsValidatedLiveCounselorPawn(host,LocalCounselorTarget);
            bool pendingDeath=false;for(const auto& pending:pendingDeaths)pendingDeath=pendingDeath||pending.Pawn!=0;
            if(!live&&pendingDeath){
                pendingDeaths={};
                if(!FinalCounselorDeathObserved){FinalCounselorDeathObserved=true;host.ResetFinalDeathSpectatorState();}
            }else{
                for(auto& pending:pendingDeaths){
                    if(!pending.Pawn||now<pending.ReadyAt||pending.Pawn==host.LastHeldCounselor())continue;
                    host.RetireDeadAICounselorFromSpectating(pending.Pawn);pending={};
                }
            }
        }
        if(now<NextRosterRefreshAt)return;NextRosterRefreshAt=now+60000;
        if(controllerCount>0)return;
        unsigned levels=0;
        if(!world||!host.ReadWorldLevelCount(world,levels)||levels==0||levels>1024)return;
        if(RosterRefreshLevel>=levels)RosterRefreshLevel=0;
        std::uintptr_t level=0;
        if(!host.ReadWorldLevel(world,RosterRefreshLevel++,level)||!level)return;
        unsigned actorCount=0;
        if(!host.ReadLevelActorCount(level,actorCount)||actorCount==0||actorCount>100000)return;
        for(unsigned i=0;i<actorCount&&count<JasonAITargetCapacity;++i){std::uintptr_t actor=0;
            if(host.ReadLevelActor(level,i,actor)&&DonorIsValidatedLiveCounselorPawn(host,actor))DonorRegisterJasonAITarget(host,actor);}
    }
    template<class Host>void DonorRegisterLiveCounselorTargets(Host& host,std::uintptr_t world){
        unsigned levels=0;if(!world||!host.ReadWorldLevelCount(world,levels)||levels==0||levels>1024)return;
        for(unsigned li=0;li<levels;++li){std::uintptr_t level=0;unsigned actorCount=0;
            if(!host.ReadWorldLevel(world,li,level)||!level||!host.ReadLevelActorCount(level,actorCount)||
               actorCount==0||actorCount>100000)continue;
            for(unsigned ai=0;ai<actorCount;++ai){std::uintptr_t actor=0;
                if(host.ReadLevelActor(level,ai,actor)&&DonorIsValidatedLiveCounselorPawn(host,actor))DonorRegisterJasonAITarget(host,actor);}
        }
    }
};
}
