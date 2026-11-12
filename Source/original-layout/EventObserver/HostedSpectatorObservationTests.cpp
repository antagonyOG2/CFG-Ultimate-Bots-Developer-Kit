#include "HostedSpectatorObservation.hpp"
#include <array>
#include <cassert>
#include <cstring>
#include <map>
#include <cstdio>

namespace {
namespace d=cfghosted::disk35;
namespace b=cfghosted::birth35;
struct Reader {
    std::map<std::uintptr_t,std::array<unsigned char,16>> cells;unsigned reads=0;std::uintptr_t lastAddress=0;bool lastReadPresent=false;
    std::uintptr_t mutateAddress=0;unsigned mutateOnRead=0,mutateReads=0;bool mutationApplied=false;
    std::array<unsigned char,16> mutation{};
    template<class T> void Put(std::uintptr_t at,const T& value){
        static_assert(sizeof(T)<=16,"sparse cell bound");auto& bytes=cells[at];bytes={};std::memcpy(bytes.data(),&value,sizeof(T));
    }
    template<class T> bool Read(std::uintptr_t at,T& value){
        if(at==mutateAddress&&mutateOnRead&&++mutateReads==mutateOnRead){cells[at]=mutation;mutationApplied=true;}
        ++reads;lastAddress=at;const auto found=cells.find(at);lastReadPresent=found!=cells.end();if(!lastReadPresent)return false;
        T next{};std::memcpy(&next,found->second.data(),sizeof(T));value=next;return true;
    }
    template<class T> void MutateOnRead(std::uintptr_t at,unsigned ordinal,const T& value){
        static_assert(sizeof(T)<=16,"mutation cell bound");mutateAddress=at;mutateOnRead=ordinal;mutateReads=0;mutationApplied=false;
        mutation={};std::memcpy(mutation.data(),&value,sizeof(T));
    }
};
struct Fixture {
    static constexpr std::uintptr_t Module=0x10000000,Engine=0x20000000,Viewport=0x20010000,
        World=0x20020000,Mode=0x20030000,HostController=0x20040000,HostPawn=0x20050000,
        HostPlayer=0x20060000,GameInstance=0x20070000,LocalPlayerData=0x20080000,Driver=0x20090000,
        GameState=0x20100000,RemoteController=0x20110000,RemoteState=0x20120000,RemoteSpectator=0x20130000,
        JasonPawn=0x20140000,JasonController=0x20150000,JasonState=0x20160000,
        StockPawn=0x20170000,StockController=0x20180000,StockState=0x20190000,
        ObjectData=0x30000000,PlayerData=0x30100000;
    Reader r;cfgobserver::Plan plan{};cfgprimary::HostedBinding binding{};cfghosted::adoption::Receipt receipt{};
    std::atomic<std::uint64_t> active{41};std::uintptr_t stateType=0x41000000,pcType=0x42000000,
        counselorType=0x43000000,killerType=0x44000000,killerControllerType=0x45000000,
        spectatorType=0x46000000,gameStateType=0x47000000,modeType=0x48000000,huntType=0x49000000;
    unsigned objects=0;
    void Cache(std::uintptr_t a,std::uintptr_t b,std::uintptr_t type){r.Put(Module+a,type);r.Put(Module+b,type);}
    void Object(std::uintptr_t object,std::uintptr_t type,std::uintptr_t outer,std::int32_t serial){
        const auto index=static_cast<std::int32_t>(objects++);r.Put(object+cfgprobe::ObjectClass,type);
        r.Put(object+0x20,outer);r.Put(object+cfgprobe::lifetime::ObjectIndexOffset,index);
        r.Put(ObjectData+static_cast<std::uintptr_t>(index)*cfgprobe::lifetime::ItemBytes+cfgprobe::lifetime::ItemObjectOffset,object);
        r.Put(ObjectData+static_cast<std::uintptr_t>(index)*cfgprobe::lifetime::ItemBytes+8,std::uint32_t{});
        r.Put(ObjectData+static_cast<std::uintptr_t>(index)*cfgprobe::lifetime::ItemBytes+cfgprobe::lifetime::ItemSerialOffset,serial);
        r.Put(type+cfgprobe::ClassSuper,std::uintptr_t{});
    }
    void StateLinks(std::uintptr_t pawn,std::uintptr_t controller,std::uintptr_t state){
        r.Put(pawn+cfghosted::adoption::PawnController,controller);r.Put(controller+cfghosted::adoption::ControllerPawn,pawn);
        r.Put(pawn+cfghosted::adoption::PawnState,state);r.Put(controller+cfghosted::adoption::ControllerState,state);
        r.Put(state+cfghosted::adoption::StateOwner,controller);
    }
    Fixture(){
        using namespace cfgprobe;plan.bytes=sizeof(plan);plan.threadId=77;plan.module=Module;plan.engine=Engine;plan.viewport=Viewport;
        plan.world=World;plan.mode=Mode;plan.controller=HostController;plan.pawn=HostPawn;plan.player=HostPlayer;
        plan.gameInstance=GameInstance;plan.localPlayerData=LocalPlayerData;plan.dispatch={1,77,0x200A0000,Module+dispatch::NativeActorDispatcherRva};
        Cache(cfghosted::disk35::WorldReflection,cfghosted::disk35::WorldConstructor,0x4A00000);
        Cache(cfghosted::disk35::NetDriverReflection,cfghosted::disk35::NetDriverConstructor,0x4A10000);
        Cache(cfghosted::disk35::HuntReflection,cfghosted::disk35::HuntConstructor,huntType);
        Cache(cfghosted::disk35::SCGameInstanceReflection,cfghosted::disk35::SCGameInstanceConstructor,0x4A20000);
        Cache(cfghosted::disk35::ILLLocalPlayerReflection,cfghosted::disk35::ILLLocalPlayerConstructor,0x4A30000);
        Cache(cfghosted::disk35::PlayerControllerReflection,cfghosted::disk35::PlayerControllerConstructor,pcType);
        Cache(cfgprobe::NativeClasses[cfgprobe::PlayerControllerClass].reflectionCacheRva,cfgprobe::NativeClasses[cfgprobe::PlayerControllerClass].constructorCacheRva,pcType);
        Cache(cfgspectator::disk::ControllerClassCache,cfgspectator::disk::ControllerConstructorCache,pcType);
        Cache(cfgspectator::disk::PawnClassCache,cfgspectator::disk::PawnConstructorCache,spectatorType);
        Cache(cfgescape::disk::SCPlayerStateReflection,cfgescape::disk::SCPlayerStateConstructor,stateType);
        Cache(cfgescape::disk::SCGameStateReflection,cfgescape::disk::SCGameStateConstructor,gameStateType);
        Cache(cfgprobe::NativeClasses[cfgprobe::Counselor].reflectionCacheRva,cfgprobe::NativeClasses[cfgprobe::Counselor].constructorCacheRva,counselorType);
        Cache(cfgprobe::NativeClasses[cfgprobe::Killer].reflectionCacheRva,cfgprobe::NativeClasses[cfgprobe::Killer].constructorCacheRva,killerType);
        Cache(cfgspawn::Classes[2].reflection,cfgspawn::Classes[2].constructor,0x4A40000);
        Cache(cfgspawn::Classes[3].reflection,cfgspawn::Classes[3].constructor,killerControllerType);
        r.Put(Module+cfgprobe::lifetime::ObjectArrayRva+cfgprobe::lifetime::ArrayDataOffset,ObjectData);
        r.Put(Module+cfgprobe::lifetime::ObjectArrayRva+cfgprobe::lifetime::ArrayCountOffset,std::int32_t{17});
        r.Put(Module+cfgprimary::SerializationFlag,std::uint8_t{});r.Put(Module+cfgprimary::GarbageCollectionCounter,std::uint32_t{});
        r.Put(Module+cfgprobe::dispatch::ThreadFlagRva,std::uint8_t{1});r.Put(Module+cfgprobe::dispatch::ThreadIdRva,std::uint32_t{77});
        r.Put(Module+cfgprobe::EngineGlobalCandidate,Engine);r.Put(Engine+cfgprobe::EngineGameViewport,Viewport);
        r.Put(Viewport+cfgprobe::ViewportWorld,World);r.Put(World+d::WorldGameMode,Mode);r.Put(World+d::WorldNetDriver,Driver);
        r.Put(World+d::WorldGameState,GameState);r.Put(World+d::WorldGameInstance,GameInstance);r.Put(World+cfgprobe::WorldGameMode,Mode);
        r.Put(World+cfghosted::disk35::WorldNetDriver,Driver);r.Put(World+cfghosted::disk35::WorldGameMode,Mode);
        r.Put(World+cfghosted::disk35::WorldGameInstance,GameInstance);r.Put(World+cfghosted::disk35::WorldGameState,GameState);
        r.Put(Driver+d::DriverWorld,World);r.Put(Driver+d::ServerConnection,std::uintptr_t{});
        r.Put(Mode+cfgprobe::ObjectClass,modeType);r.Put(Module+d::HuntReflection,huntType);r.Put(Module+d::HuntConstructor,huntType);
        r.Put(Mode+d::ActorRole,cfghosted::disk::AuthorityRole);r.Put(Mode+cfgescape::disk::MatchState,cfgnames::Words{0,0});
        r.Put(Module+cfgnames::disk::Global,std::uintptr_t{0x31000000});r.Put(0x31000000,std::uintptr_t{0x31001000});
        r.Put(0x31000400,std::array<std::int32_t,2>{{1,1}});r.Put(0x31001000,std::uintptr_t{0x31002000});
        r.Put(0x31002000,std::uint32_t{});
        std::array<char,8> phaseFirst{{'I','n','P','r','o','g','r','e'}},phaseSecond{{'s','s',0,0,0,0,0,0}};
        r.Put(0x31002010,phaseFirst);r.Put(0x31002018,phaseSecond);
        Object(World,0x4A00000,World,101);Object(Mode,modeType,World,102);Object(HostController,pcType,World,103);Object(HostPawn,counselorType,World,104);
        Object(HostPlayer,0x4A30000,GameInstance,105);Object(GameInstance,0x4A20000,World,106);Object(Driver,0x4A10000,World,107);
        Object(GameState,gameStateType,World,108);Object(RemoteController,pcType,World,109);Object(RemoteState,stateType,World,0);
        Object(RemoteSpectator,spectatorType,World,111);Object(JasonPawn,killerType,World,112);Object(JasonController,killerControllerType,World,113);
        Object(JasonState,stateType,World,0);Object(StockPawn,counselorType,World,115);Object(StockController,pcType,World,116);
        Object(StockState,stateType,World,117);
        plan.objectArray={ObjectData,static_cast<std::int32_t>(objects)};
        plan.objects={{{World,0,101},{Mode,1,102},{HostController,2,103},{HostPawn,3,104}}};
        binding.active=true;binding.plan=plan;binding.netDriver=Driver;binding.driver={Driver,6,107};binding.modeClass=modeType;binding.huntClass=huntType;
        cfgprimary::CurrentHostedBinding()=binding;
        r.Put(GameInstance+cfghosted::disk35::GameInstanceLocalPlayers,std::array<std::uintptr_t,2>{{LocalPlayerData,1}});
        struct Locals{std::uintptr_t data;std::int32_t count,capacity;};r.Put(GameInstance+cfghosted::disk35::GameInstanceLocalPlayers,Locals{LocalPlayerData,1,1});
        r.Put(LocalPlayerData,HostPlayer);r.Put(HostPlayer+cfghosted::disk35::PlayerController,HostController);
        r.Put(HostController+cfghosted::disk35::ControllerPlayer,HostPlayer);r.Put(HostController+d::ActorRole,cfghosted::disk::AuthorityRole);
        struct Players{std::uintptr_t data;std::int32_t count,capacity;};r.Put(GameState+b::PlayerArray,Players{PlayerData,3,3});
        r.Put(PlayerData,RemoteState);r.Put(PlayerData+8,JasonState);r.Put(PlayerData+16,StockState);
        r.Put(GameState+b::CurrentKiller,JasonPawn);r.Put(GameState+b::CurrentKillerState,JasonState);
        StateLinks(JasonPawn,JasonController,JasonState);StateLinks(StockPawn,StockController,StockState);
        r.Put(RemoteController+cfghosted::adoption::ControllerState,RemoteState);
        r.Put(RemoteController+cfghosted::adoption::ControllerPawn,RemoteSpectator);
        r.Put(RemoteController+cfgprobe::ControllerSpectatorPawn,RemoteSpectator);
        r.Put(RemoteState+cfghosted::adoption::StateOwner,RemoteController);
        r.Put(RemoteState+cfgescape::disk::bDead,std::uint8_t{1});r.Put(RemoteState+cfgescape::disk::bEscaped,std::uint8_t{});
        r.Put(RemoteSpectator+cfghosted::adoption::PawnController,RemoteController);
        r.Put(JasonState+cfgescape::disk::bDead,std::uint8_t{});r.Put(JasonState+cfgescape::disk::bEscaped,std::uint8_t{});
        r.Put(StockState+cfgescape::disk::bDead,std::uint8_t{});r.Put(StockState+cfgescape::disk::bEscaped,std::uint8_t{});
        r.Put(JasonPawn+cfghunter::disk::Health,100.0f);r.Put(JasonPawn+cfgescape::disk::DyingFlags,std::uint8_t{});
        r.Put(StockPawn+cfgescape::disk::bHasEscaped,std::uint8_t{});r.Put(StockPawn+cfghunter::disk::Health,100.0f);
        receipt.valid=true;receipt.pawn={JasonPawn,11,112};receipt.controller={JasonController,12,113};
        receipt.controllerState=JasonState;receipt.displacedState=RemoteState;receipt.world=World;receipt.mode=Mode;
        receipt.possessReturned=true;receipt.botsBefore=0;receipt.botsAfter=1;receipt.rosterBefore=1;receipt.rosterAfter=2;
    }
    cfgh216spectator::Request RequestFor(std::uintptr_t candidate,bool jason=false){
        return {&plan,&binding,&receipt,&active,41,77,GameState,RemoteController,candidate,!jason};
    }
};
}
int main(){
    Fixture f;cfgh216spectator::Observation observed;
    auto request=f.RequestFor(Fixture::StockState);
    const bool first=cfgh216spectator::Observe(f.r,request,observed);
    if(!first)std::fprintf(stderr,"collector first=%d reads=%u step=%u lastRead=%p present=%d\n",first,observed.reads,
        observed.failureStep,reinterpret_cast<void*>(f.r.lastAddress),f.r.lastReadPresent);
    assert(first&&observed.stockCounselor&&observed.candidate.object==Fixture::StockState&&observed.proof.remoteViewer);
    auto jasonRequest=f.RequestFor(Fixture::JasonState,true);
    assert(cfgh216spectator::Observe(f.r,jasonRequest,observed)&&
        observed.candidate.object==Fixture::JasonState&&observed.candidate.serial==0&&
        observed.candidate.stronglyOwnedThisCallback&&observed.proof.registeredJason.ownerActor.serial>0);
    f.r.Put(Fixture::StockState+cfgescape::disk::bDead,std::uint8_t{1});
    assert(!cfgh216spectator::Observe(f.r,f.RequestFor(Fixture::StockState),observed));
    f.r.Put(Fixture::StockState+cfgescape::disk::bDead,std::uint8_t{});
    f.r.Put(Fixture::RemoteState+cfghosted::adoption::StateOwner,std::uintptr_t{Fixture::StockController});
    assert(!cfgh216spectator::Observe(f.r,f.RequestFor(Fixture::StockState),observed));
    std::atomic<std::uint64_t> stale{40};auto staleRequest=f.RequestFor(Fixture::StockState);staleRequest.activeGeneration=&stale;
    assert(!cfgh216spectator::Observe(f.r,staleRequest,observed));
    const auto noAuthority=[](const cfgh216spectator::Observation& o){
        return o.failureStep==17&&!o.proof.world&&!o.proof.viewer&&!o.proof.exactAdoptedJason&&!o.candidate.object&&!o.stockCounselor;
    };
    {
        Fixture transition;transition.r.MutateOnRead(Fixture::World+cfghosted::disk35::WorldGameState,2,std::uintptr_t{0x202A0000});
        assert(!cfgh216spectator::Observe(transition.r,transition.RequestFor(Fixture::StockState),observed));
        assert(transition.r.mutationApplied&&noAuthority(observed));
    }
    {
        Fixture transition;transition.r.MutateOnRead(Fixture::GameState+cfghosted::birth35::CurrentKiller,2,std::uintptr_t{0x202B0000});
        assert(!cfgh216spectator::Observe(transition.r,transition.RequestFor(Fixture::StockState),observed));
        assert(transition.r.mutationApplied&&noAuthority(observed));
    }
    {
        Fixture transition;transition.r.MutateOnRead(Fixture::GameState+cfghosted::birth35::CurrentKillerState,2,std::uintptr_t{0x202C0000});
        assert(!cfgh216spectator::Observe(transition.r,transition.RequestFor(Fixture::StockState),observed));
        assert(transition.r.mutationApplied&&noAuthority(observed));
    }
    std::printf("H216 hosted spectator read-only graph collector: valid live stock/Jason witnesses; dead-stock/owner/generation and three final-authority transition refusals; reads=%u.\n",f.r.reads);
}
