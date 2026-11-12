#pragma once
#include "HostedAdoption.hpp"
#include "CounselorTerminalObservation.hpp"
#include "SpectatorTerminalObservation.hpp"
#include "HostedRegisteredRoster.hpp"
#include "../RuntimeProbe/HostedBirth35.generated.hpp"
#include "../RuntimeProbe/PrimaryLookupEligibility.hpp"
#include "../RuntimeProbe/SpectatorClassBindings.hpp"
#include "../RuntimeProbe/HostedAuthority35.generated.hpp"
#include "../RuntimeProbe/HostedSpectator35.generated.hpp"
#include "../RuntimeProbe/HostedSpectatorRuntime.hpp"
#include <array>
#include <atomic>
#include <cstdint>

namespace cfgh216spectator {
constexpr unsigned MaximumPlayerStates=32,ReadLimit=8192;
template<class R> class Bounded {
    R& source_;
public:
    unsigned reads=0;
    explicit Bounded(R& source):source_(source){}
    template<class T> bool Read(std::uintptr_t at,T& output){
        if(reads>=ReadLimit)return false;++reads;return source_.Read(at,output);
    }
};
struct Request {
    const cfgobserver::Plan* plan=nullptr;
    const cfgprimary::HostedBinding* binding=nullptr;
    const cfghosted::adoption::Receipt* adoption=nullptr;
    const std::atomic<std::uint64_t>* activeGeneration=nullptr;
    std::uint64_t generation=0;
    std::uint32_t currentThread=0;
    std::uintptr_t gameState=0,viewerController=0,candidateState=0;
    bool candidateMustBeCounselor=true;
};
struct Observation {
    cfghosted::spectator::Proof proof{};
    cfghosted::spectator::PlayerStateIdentity candidate{};
    bool stockCounselor=false;
    unsigned reads=0,failureStep=0;
};
struct PlayerArrayHeader {
    std::uintptr_t data=0;
    std::int32_t count=0,capacity=0;
    bool operator==(const PlayerArrayHeader& b)const noexcept{return data==b.data&&count==b.count&&capacity==b.capacity;}
};

template<class R> bool Cache(Bounded<R>& r,std::uintptr_t module,std::uintptr_t reflection,
                             std::uintptr_t constructor,std::uintptr_t& value){
    value=0;std::uintptr_t a=0,b=0;
    if(!r.Read(module+reflection,a)||!r.Read(module+constructor,b)||!cfgprobe::MatchingClassCache(a,b))return false;
    value=a;return true;
}
template<class R> bool Member(Bounded<R>& r,const cfgprobe::lifetime::ArrayHeader& registry,
    const PlayerArrayHeader& list,std::uintptr_t state,std::uintptr_t stateType,std::uintptr_t& cell){
    cell=0;if(!state||!cfgprobe::ObjectPointer(state)||!cfgprobe::lifetime::Extent(list.data,static_cast<std::uintptr_t>(list.count)*8))return false;
    for(int i=0;i<list.count;++i){std::uintptr_t item=0;const auto at=list.data+8ULL*static_cast<unsigned>(i);
        if(!r.Read(at,item))return false;
        if(item==state)cell=at;
    }
    cfgprobe::lifetime::SlotObservation slot{};
    return cell&&cfgspawn::ObjectIs(r,state,stateType)&&cfgspawn::ObjectSlot(r,registry,state,slot);
}
template<class R> bool OwnedState(Bounded<R>& r,const cfgprobe::lifetime::ArrayHeader& registry,
    std::uintptr_t pawn,std::uintptr_t controller,std::uintptr_t state,
    std::uintptr_t stateType,const cfgprobe::lifetime::SlotObservation& controllerSlot,
    cfghosted::spectator::PlayerStateIdentity& output){
    struct Witness {std::uintptr_t object=0;std::int32_t serial=0;bool strongOwnedState=false;};
    output={};if(!controllerSlot.HasAssignedSerial()||controllerSlot.object!=controller||!pawn||!state)return false;
    cfgprobe::lifetime::SlotObservation stateSlot{};Witness owned{};
    if(!cfgspawn::ObjectSlot(r,registry,state,stateSlot)||!cfgspawn::ObjectIs(r,state,stateType)||
       !cfgspectator::OwnedPlayerState(r,registry,pawn,controller,state,stateType,owned)||
       owned.object!=state||owned.serial!=stateSlot.serial||owned.strongOwnedState!=(stateSlot.serial==0))return false;
    output={state,stateSlot.serial,{controller,controllerSlot.serial},owned.strongOwnedState};
    return cfghosted::spectator::ValidPlayerState(output);
}

// Same-callback, read-only graph proof used by both native callbacks. The
// caller supplies only native arguments and the existing immutable host proof;
// every policy boolean below is derived from current memory and repeated.
template<class R> bool Observe(R& source,const Request& request,Observation& output){
    output={};Bounded<R> r(source);unsigned step=1;
    const auto finish=[&](bool okay){
        if(okay){output.reads=r.reads;return true;}
        const auto used=r.reads;output={};output.reads=used;output.failureStep=step;return false;
    };
    if(!request.plan||!request.binding||!request.adoption||!request.activeGeneration||!request.generation||
       request.currentThread!=request.plan->threadId||request.viewerController==request.plan->controller||
       !request.candidateState||request.activeGeneration->load(std::memory_order_acquire)!=request.generation)return finish(false);
    const auto& p=*request.plan;const auto& binding=*request.binding;const auto& receipt=*request.adoption;
    step=2;if(!binding.Matches(p)||!cfgprimary::CurrentHostedBinding().Matches(p)||
       !cfgprimary::LookupVetoClearForObservation(r,p.module))return finish(false);
    cfgobserver::Plan fresh{};
    step=3;if(!cfgprimary::RetainedWorld(r,p,fresh,binding))return finish(false);
    step=31;if(!cfghosted::adoption::Current(r,fresh.objectArray,receipt,p))return finish(false);
    step=32;if(!cfghosted::native::WorldOwned(r,request.gameState,p.world))return finish(false);
    step=33;if(!cfgobserver::Same(r,p.world+cfghosted::disk35::WorldGameState,request.gameState))return finish(false);
    step=4;cfgnames::Words phase{};cfgnames::Text phaseText;cfgnames::Observation<Bounded<R>> names(r,p.module);
    if(!r.Read(p.mode+cfgescape::disk::MatchState,phase)){step=41;return finish(false);}
    if(!names.Decode(phase,phaseText)){step=42;return finish(false);}
    if(phaseText.View()!=u"InProgress"){step=43;return finish(false);}
    // Prove the live host's exact local-player/controller chain without the
    // offline ContextUnchanged predicate (which rejects every NetDriver).
    step=5;struct LocalPlayers{std::uintptr_t data=0;std::int32_t count=0,capacity=0;} locals{};
    std::uintptr_t localPlayer=0,localController=0;
    std::uint8_t localRole=0;
    if(!r.Read(p.gameInstance+cfgprobe::GameInstanceLocalPlayers,locals)||locals.data!=p.localPlayerData||
       locals.count!=1||locals.capacity<1||locals.capacity>4||!r.Read(locals.data,localPlayer)||localPlayer!=p.player||
       !r.Read(localPlayer+cfghosted::disk35::PlayerController,localController)||localController!=p.controller||
       !cfgobserver::Same(r,p.controller+cfghosted::disk35::ControllerPlayer,localPlayer)||
       !r.Read(p.controller+cfghosted::disk35::ActorRole,localRole)||localRole!=cfghosted::disk::AuthorityRole)return finish(false);

    step=6;cfgprobe::lifetime::ArrayHeader registry{};std::uintptr_t stateType=0,playerControllerType=0,
        scPlayerControllerType=0,aiControllerType=0,killerAIControllerType=0,spectatorPawnType=0,
        counselorType=0,killerType=0,gameStateType=0;
    if(!cfgprobe::lifetime::ReadArrayHeader(r,p.module+cfgprobe::lifetime::ObjectArrayRva,registry)||
       !Cache(r,p.module,cfgescape::disk::SCPlayerStateReflection,cfgescape::disk::SCPlayerStateConstructor,stateType)||
       !Cache(r,p.module,cfgprobe::NativeClasses[cfgprobe::PlayerControllerClass].reflectionCacheRva,
          cfgprobe::NativeClasses[cfgprobe::PlayerControllerClass].constructorCacheRva,playerControllerType)||
       !Cache(r,p.module,cfgspectator::disk::ControllerClassCache,
          cfgspectator::disk::ControllerConstructorCache,scPlayerControllerType)||
       !Cache(r,p.module,cfgspawn::Classes[2].reflection,cfgspawn::Classes[2].constructor,aiControllerType)||
       !Cache(r,p.module,cfgspawn::Classes[3].reflection,cfgspawn::Classes[3].constructor,killerAIControllerType)||
       !Cache(r,p.module,cfgspectator::disk::PawnClassCache,cfgspectator::disk::PawnConstructorCache,spectatorPawnType)||
       !Cache(r,p.module,cfgprobe::NativeClasses[cfgprobe::Counselor].reflectionCacheRva,
          cfgprobe::NativeClasses[cfgprobe::Counselor].constructorCacheRva,counselorType)||
       !Cache(r,p.module,cfgprobe::NativeClasses[cfgprobe::Killer].reflectionCacheRva,
          cfgprobe::NativeClasses[cfgprobe::Killer].constructorCacheRva,killerType)||
       !Cache(r,p.module,cfgescape::disk::SCGameStateReflection,cfgescape::disk::SCGameStateConstructor,gameStateType)||
       !cfgspawn::ObjectIs(r,request.gameState,gameStateType))return finish(false);
    step=7;cfgprobe::lifetime::SlotObservation worldSlot{},gameStateSlot{},viewerSlot{},viewerStateSlot{},jasonPawnSlot{},jasonControllerSlot{};
    if(!cfgspawn::ObjectSlot(r,registry,p.world,worldSlot)||!worldSlot.HasAssignedSerial()||!(worldSlot==p.objects[0])||
       !cfgspawn::ObjectSlot(r,registry,request.gameState,gameStateSlot)||!gameStateSlot.HasAssignedSerial()||
       !cfgspawn::ObjectSlot(r,registry,request.viewerController,viewerSlot)||!viewerSlot.HasAssignedSerial()||
       !cfgspawn::ObjectIs(r,request.viewerController,playerControllerType)||
       !cfgspawn::ObjectIs(r,request.viewerController,scPlayerControllerType)||
       !cfghosted::native::WorldOwned(r,request.viewerController,p.world)||
       !cfgspawn::ObjectSlot(r,registry,receipt.pawn.object,jasonPawnSlot)||!jasonPawnSlot.HasAssignedSerial()||
       !cfgspawn::ObjectSlot(r,registry,receipt.controller.object,jasonControllerSlot)||!jasonControllerSlot.HasAssignedSerial()||
       !cfgspawn::ObjectIs(r,receipt.controller.object,killerAIControllerType))return finish(false);

    step=8;PlayerArrayHeader list{},again{};
    if(!r.Read(request.gameState+cfghosted::birth35::PlayerArray,list)||list.count<1||list.count>MaximumPlayerStates||
       list.capacity<list.count||list.capacity>64||!cfgprobe::ObjectPointer(list.data)||
       !cfgprobe::lifetime::Extent(list.data,static_cast<std::uintptr_t>(list.capacity)*8))return finish(false);
    std::array<std::uintptr_t,MaximumPlayerStates> states{};
    for(int i=0;i<list.count;++i){if(!r.Read(list.data+8ULL*static_cast<unsigned>(i),states[static_cast<unsigned>(i)]))return finish(false);
        if(states[static_cast<unsigned>(i)]&&!cfgprobe::ObjectPointer(states[static_cast<unsigned>(i)]))return finish(false);
        for(int j=0;j<i;++j)if(states[static_cast<unsigned>(i)]&&states[static_cast<unsigned>(i)]==states[static_cast<unsigned>(j)])return finish(false);}
    const auto member=[&](std::uintptr_t state){for(int i=0;i<list.count;++i)if(states[static_cast<unsigned>(i)]==state)return true;return false;};

    // Current remote terminal viewer: positive SCPlayerController identity,
    // non-local controller, same host world, owner-linked PlayerState that is
    // an exact current PlayerArray member, and typed native dead/escaped flags.
    step=9;if(request.viewerController==p.controller)return finish(false);
    std::uintptr_t viewerState=0,viewerPawn=0,viewerSpectatorPawn=0,owner=0;
    if(!r.Read(request.viewerController+cfghosted::adoption::ControllerState,viewerState)||!viewerState||!member(viewerState)||
       !cfgspawn::ObjectIs(r,viewerState,stateType)||!r.Read(viewerState+cfghosted::adoption::StateOwner,owner)||owner!=request.viewerController||
       !cfgspawn::ObjectSlot(r,registry,viewerState,viewerStateSlot)||
       !r.Read(request.viewerController+cfghosted::adoption::ControllerPawn,viewerPawn)||
       !r.Read(request.viewerController+cfgprobe::ControllerSpectatorPawn,viewerSpectatorPawn))return finish(false);
    cfgescape::State terminal=cfgescape::State::Unknown;
    if(!cfgspectator::LocalTerminal(r,registry,p.module,request.viewerController,terminal)||!cfgescape::Terminal(terminal))return finish(false);
    bool typedViewerPawn=false;cfgprobe::lifetime::SlotObservation viewerSpectatorSlot{};
    if(viewerSpectatorPawn){cfgprobe::lifetime::SlotObservation spectatorSlot{};
        if(!cfgspawn::ObjectSlot(r,registry,viewerSpectatorPawn,spectatorSlot)||!spectatorSlot.HasAssignedSerial()||
           !cfgspawn::ObjectIs(r,viewerSpectatorPawn,spectatorPawnType)||
           !cfghosted::native::WorldOwned(r,viewerSpectatorPawn,p.world)||
           !cfgobserver::Same(r,viewerSpectatorPawn+cfghosted::adoption::PawnController,request.viewerController))return finish(false);
        viewerSpectatorSlot=spectatorSlot;
        typedViewerPawn=true;
    }
    if(viewerPawn&&viewerPawn!=viewerSpectatorPawn){cfgprobe::lifetime::SlotObservation pawnSlot{};cfgescape::State pawnState=cfgescape::State::Unknown;
        if(!cfgspawn::ObjectSlot(r,registry,viewerPawn,pawnSlot)||!pawnSlot.HasAssignedSerial()||
           !cfghosted::native::WorldOwned(r,viewerPawn,p.world)||
           !cfgobserver::Same(r,viewerPawn+cfghosted::adoption::PawnController,request.viewerController))return finish(false);
        if(cfgspawn::ObjectIs(r,viewerPawn,counselorType)){
            if(!cfgobserver::Same(r,viewerPawn+cfghosted::adoption::PawnState,viewerState)||
               !cfgescape::Counselor(r,registry,p.module,viewerPawn,counselorType,pawnState)||
               pawnState==cfgescape::State::Living||pawnState==cfgescape::State::Unpossessed)return finish(false);
        }else return finish(false);
    }else if(viewerPawn&&!viewerSpectatorPawn){
        cfgprobe::lifetime::SlotObservation pawnSlot{};cfgescape::State pawnState=cfgescape::State::Unknown;
        if(!cfgspawn::ObjectSlot(r,registry,viewerPawn,pawnSlot)||!pawnSlot.HasAssignedSerial()||
           !cfgspawn::ObjectIs(r,viewerPawn,counselorType)||!cfghosted::native::WorldOwned(r,viewerPawn,p.world)||
           !cfgobserver::Same(r,viewerPawn+cfghosted::adoption::PawnController,request.viewerController)||
           !cfgobserver::Same(r,viewerPawn+cfghosted::adoption::PawnState,viewerState)||
           !cfgescape::Counselor(r,registry,p.module,viewerPawn,counselorType,pawnState)||
           pawnState==cfgescape::State::Living||pawnState==cfgescape::State::Unpossessed)return finish(false);
        typedViewerPawn=true;
    }
    // Serial-zero terminal state is usable only while this same callback has
    // typed the controller's owned counselor/spectator pawn and its owner,
    // state, roster and slot links remain current.
    if(viewerStateSlot.serial<=0&&(!typedViewerPawn||(!viewerPawn&&!viewerSpectatorPawn)))return finish(false);

    step=10;// Exact adopted Jason state: receipt, current killer tuple, roster member,
    // owner/controller/pawn backlinks, current world and same-callback serial0 proof.
    std::uintptr_t killerPawn=0,killerState=0,jasonOwner=0,jasonPawnController=0,jasonPawnState=0;
    if(!r.Read(request.gameState+cfghosted::birth35::CurrentKiller,killerPawn)||killerPawn!=receipt.pawn.object||
       !r.Read(request.gameState+cfghosted::birth35::CurrentKillerState,killerState)||killerState!=receipt.controllerState||
       !member(killerState)||!cfgspawn::ObjectIs(r,killerPawn,killerType)||!cfgspawn::ObjectIs(r,killerState,stateType)||
       !r.Read(killerState+cfghosted::adoption::StateOwner,jasonOwner)||jasonOwner!=receipt.controller.object||
       !r.Read(killerPawn+cfghosted::adoption::PawnController,jasonPawnController)||jasonPawnController!=receipt.controller.object||
       !r.Read(killerPawn+cfghosted::adoption::PawnState,jasonPawnState)||jasonPawnState!=killerState||
       !cfghosted::native::WorldOwned(r,killerPawn,p.world)||!cfghosted::native::WorldOwned(r,receipt.controller.object,p.world))return finish(false);
    bool jasonLiving=false;
    if(!cfgspectator::KillerLiving(r,killerPawn,killerState,jasonLiving)||!jasonLiving)return finish(false);
    step=11;std::uintptr_t candidateCell=0,jasonCell=0,viewerCell=0;
    if(!Member(r,registry,list,request.candidateState,stateType,candidateCell)||
       !Member(r,registry,list,killerState,stateType,jasonCell)||
       !Member(r,registry,list,viewerState,stateType,viewerCell))return finish(false);
    cfghosted::spectator::PlayerStateIdentity jasonIdentity{};
    if(!OwnedState(r,registry,killerPawn,receipt.controller.object,killerState,stateType,jasonControllerSlot,jasonIdentity))return finish(false);
    std::uintptr_t candidatePawn=0,candidateOwner=0;
    step=12;if(request.candidateState==killerState){
        if(request.candidateMustBeCounselor)return finish(false);
        output.candidate=jasonIdentity;candidatePawn=killerPawn;candidateOwner=receipt.controller.object;
    }else{
        if(!request.candidateMustBeCounselor)return finish(false);
        std::uintptr_t stockOwner=0,stockPawn=0;cfgprobe::lifetime::SlotObservation stockControllerSlot{};
        cfgescape::State stockLife=cfgescape::State::Unknown;
        if(!r.Read(request.candidateState+cfghosted::adoption::StateOwner,stockOwner)||!stockOwner||
           !cfgspawn::ObjectSlot(r,registry,stockOwner,stockControllerSlot)||!stockControllerSlot.HasAssignedSerial()||
           (!cfgspawn::ObjectIs(r,stockOwner,playerControllerType)&&!cfgspawn::ObjectIs(r,stockOwner,aiControllerType))||
           !cfghosted::native::WorldOwned(r,stockOwner,p.world)||
           !r.Read(stockOwner+cfghosted::adoption::ControllerPawn,stockPawn)||!stockPawn||
           !cfgspawn::ObjectIs(r,stockPawn,counselorType)||!cfghosted::native::WorldOwned(r,stockPawn,p.world)||
           !cfgobserver::Same(r,stockPawn+cfghosted::adoption::PawnController,stockOwner)||
           !cfgobserver::Same(r,stockPawn+cfghosted::adoption::PawnState,request.candidateState)||
           !cfgobserver::Same(r,stockOwner+cfghosted::adoption::ControllerState,request.candidateState)||
           !Member(r,registry,list,request.candidateState,stateType,candidateCell)||
           !cfgescape::Counselor(r,registry,p.module,stockPawn,counselorType,stockLife)||stockLife!=cfgescape::State::Living||
           !OwnedState(r,registry,stockPawn,stockOwner,request.candidateState,stateType,stockControllerSlot,output.candidate))return finish(false);
        candidatePawn=stockPawn;candidateOwner=stockOwner;
        output.stockCounselor=true;
    }
    step=13;output.proof.world={p.world,worldSlot.serial};output.proof.viewer={request.viewerController,viewerSlot.serial};
    output.proof.registeredJason=jasonIdentity;output.proof.worldEpoch=request.generation;
    output.proof.knownHostedPhase=true;output.proof.terminalViewer=true;output.proof.livingViewer=false;
    output.proof.exactAdoptedJason=true;output.proof.remoteViewer=true;
    output.proof.currentHostedGeneration=true;output.proof.currentTerminalViewer=true;

    // Recheck mutable authority, array and identity links immediately before
    // publishing policy inputs; no UObject pointers/proofs are retained.
    step=14;std::uintptr_t rosterItem=0;
    if(!r.Read(request.gameState+cfghosted::birth35::PlayerArray,again)||!(again==list)||
       !r.Read(candidateCell,rosterItem)||rosterItem!=request.candidateState||
       !r.Read(jasonCell,rosterItem)||rosterItem!=killerState||
       !r.Read(viewerCell,rosterItem)||rosterItem!=viewerState)return finish(false);
    // Atomic generation was checked through its typed API above; this final
    // load is intentionally separate from UObject reads to catch retirement.
    step=15;cfgnames::Words finalPhase{};cfgnames::Text finalPhaseText;
    std::uintptr_t cacheAgain=0;
    if(request.activeGeneration->load(std::memory_order_acquire)!=request.generation||
       !r.Read(p.mode+cfgescape::disk::MatchState,finalPhase)||!(finalPhase==phase)||
       !names.Decode(finalPhase,finalPhaseText)||finalPhaseText.View()!=u"InProgress"||
       !Cache(r,p.module,cfgescape::disk::SCPlayerStateReflection,cfgescape::disk::SCPlayerStateConstructor,cacheAgain)||cacheAgain!=stateType||
       !Cache(r,p.module,cfgspectator::disk::ControllerClassCache,cfgspectator::disk::ControllerConstructorCache,cacheAgain)||cacheAgain!=scPlayerControllerType||
       !Cache(r,p.module,cfgspectator::disk::PawnClassCache,cfgspectator::disk::PawnConstructorCache,cacheAgain)||cacheAgain!=spectatorPawnType||
       !Cache(r,p.module,cfgprobe::NativeClasses[cfgprobe::Counselor].reflectionCacheRva,cfgprobe::NativeClasses[cfgprobe::Counselor].constructorCacheRva,cacheAgain)||cacheAgain!=counselorType||
       !Cache(r,p.module,cfgprobe::NativeClasses[cfgprobe::Killer].reflectionCacheRva,cfgprobe::NativeClasses[cfgprobe::Killer].constructorCacheRva,cacheAgain)||cacheAgain!=killerType||
       !Cache(r,p.module,cfgspawn::Classes[2].reflection,cfgspawn::Classes[2].constructor,cacheAgain)||cacheAgain!=aiControllerType||
       !Cache(r,p.module,cfgspawn::Classes[3].reflection,cfgspawn::Classes[3].constructor,cacheAgain)||cacheAgain!=killerAIControllerType||
       !cfgprimary::LookupVetoClearForObservation(r,p.module)||
       !cfgprimary::RetainedWorld(r,p,fresh,binding)||!(fresh.objectArray==registry)||
       !cfghosted::adoption::Current(r,fresh.objectArray,receipt,p)||
       !cfgobserver::Same(r,request.viewerController+cfghosted::adoption::ControllerState,viewerState)||
       !cfgobserver::Same(r,viewerState+cfghosted::adoption::StateOwner,request.viewerController)||
       !cfgobserver::Same(r,request.viewerController+cfghosted::adoption::ControllerPawn,viewerPawn)||
       !cfgobserver::Same(r,request.viewerController+cfgprobe::ControllerSpectatorPawn,viewerSpectatorPawn)||
       !cfgobserver::Same(r,killerPawn+cfghosted::adoption::PawnController,receipt.controller.object)||
       !cfgobserver::Same(r,killerState+cfghosted::adoption::StateOwner,receipt.controller.object))return finish(false);
    step=151;LocalPlayers localsAgain{};
    if(!r.Read(p.gameInstance+cfgprobe::GameInstanceLocalPlayers,localsAgain)||localsAgain.data!=p.localPlayerData||
       localsAgain.count!=locals.count||localsAgain.capacity!=locals.capacity||
       !cfgobserver::Same(r,localsAgain.data,p.player)||
       !cfgobserver::Same(r,p.player+cfghosted::disk35::PlayerController,p.controller)||
       !cfgobserver::Same(r,p.controller+cfghosted::disk35::ControllerPlayer,p.player))return finish(false);
    cfgescape::State finalTerminal=cfgescape::State::Unknown;
    if(!cfgspectator::LocalTerminal(r,registry,p.module,request.viewerController,finalTerminal)||
       !cfgescape::Terminal(finalTerminal)||finalTerminal!=terminal||
       !cfghosted::native::WorldOwned(r,request.viewerController,p.world))return finish(false);
    step=16;cfgprobe::lifetime::SlotObservation finalSlot{};
    if(!cfgspawn::ObjectSlot(r,registry,request.viewerController,finalSlot)||!(finalSlot==viewerSlot)||
       !cfgspawn::ObjectSlot(r,registry,viewerState,finalSlot)||!(finalSlot==viewerStateSlot)||
       !cfgspawn::ObjectSlot(r,registry,killerPawn,finalSlot)||!(finalSlot==jasonPawnSlot)||
       !cfgspawn::ObjectSlot(r,registry,receipt.controller.object,finalSlot)||!(finalSlot==jasonControllerSlot)||
       !cfgspawn::ObjectSlot(r,registry,killerState,finalSlot)||finalSlot.serial!=jasonIdentity.serial)return finish(false);
    if(!request.candidateMustBeCounselor){
        if(request.candidateState!=killerState||!cfghosted::spectator::SamePlayerState(output.candidate,jasonIdentity))return finish(false);
    }else{
        if(!output.stockCounselor||!cfghosted::spectator::ValidPlayerState(output.candidate))return finish(false);
        if(!cfgspawn::ObjectSlot(r,registry,candidateOwner,finalSlot)||
           finalSlot.serial!=output.candidate.ownerActor.serial||
           !cfgobserver::Same(r,candidateOwner+cfghosted::adoption::ControllerPawn,candidatePawn)||
           !cfgobserver::Same(r,candidateOwner+cfghosted::adoption::ControllerState,request.candidateState)||
           !cfgobserver::Same(r,candidatePawn+cfghosted::adoption::PawnController,candidateOwner)||
           !cfgobserver::Same(r,candidatePawn+cfghosted::adoption::PawnState,request.candidateState)||
           !cfgobserver::Same(r,request.candidateState+cfghosted::adoption::StateOwner,candidateOwner))return finish(false);
    }
    cfghosted::spectator::PlayerStateIdentity candidateAgain{};
    bool finalJasonLiving=false;
    if(!cfgspectator::KillerLiving(r,killerPawn,killerState,finalJasonLiving)||!finalJasonLiving)return finish(false);
    if(!cfgspawn::ObjectSlot(r,registry,candidateOwner,finalSlot)||!finalSlot.HasAssignedSerial()||
       !OwnedState(r,registry,candidatePawn,candidateOwner,request.candidateState,stateType,finalSlot,candidateAgain)||
       !cfghosted::spectator::SamePlayerState(output.candidate,candidateAgain)||
       !cfghosted::native::WorldOwned(r,candidatePawn,p.world)||!cfghosted::native::WorldOwned(r,candidateOwner,p.world))return finish(false);
    if(request.candidateMustBeCounselor){
        cfgescape::State finalStockLife=cfgescape::State::Unknown;
        if(!cfgescape::Counselor(r,registry,p.module,candidatePawn,counselorType,finalStockLife)||
           finalStockLife!=cfgescape::State::Living)return finish(false);
    }
    if(viewerSpectatorPawn){cfgprobe::lifetime::SlotObservation finalSpectator{};
        if(!cfgspawn::ObjectSlot(r,registry,viewerSpectatorPawn,finalSpectator)||!(finalSpectator==viewerSpectatorSlot)||
           !cfgobserver::Same(r,viewerSpectatorPawn+cfghosted::adoption::PawnController,request.viewerController))return finish(false);}
    if(viewerPawn&&viewerPawn!=viewerSpectatorPawn&&
       (!cfgobserver::Same(r,viewerPawn+cfghosted::adoption::PawnController,request.viewerController)||
        !cfgobserver::Same(r,viewerPawn+cfghosted::adoption::PawnState,viewerState)))return finish(false);
    step=17;std::uintptr_t finalGameState=0,finalKillerPawn=0,finalKillerState=0;
    if(!r.Read(p.world+cfghosted::disk35::WorldGameState,finalGameState)||finalGameState!=request.gameState||
       !r.Read(request.gameState+cfghosted::birth35::CurrentKiller,finalKillerPawn)||finalKillerPawn!=killerPawn||
       !r.Read(request.gameState+cfghosted::birth35::CurrentKillerState,finalKillerState)||finalKillerState!=killerState)
        return finish(false);
    return finish(true);
}
} // namespace cfgh216spectator
