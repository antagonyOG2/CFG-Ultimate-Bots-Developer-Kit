#pragma once
#include "HostedListenAuthorityFixture.hpp"
#include "HostedTakeoverObservation.hpp"
namespace t=cfghosted::takeover35;
namespace b=cfghosted::birth35;
struct HostedFixture:Fixture {
    static constexpr std::uintptr_t localPawn=0x600000,human=0x610000,killer=0x620000,humanState=0x630000,hostState=0x640000,connection=0x650000,roster=0x700000;
    static constexpr std::uintptr_t stateType=0x480000,counselorType=0x490000,killerType=0x4A0000,connectionType=0x4B0000;
    HostedFixture(){
        Pair(cfgescape::disk::SCPlayerStateReflection,cfgescape::disk::SCPlayerStateConstructor,stateType);
        Pair(b::ConnectionReflection,b::ConnectionConstructor,connectionType);
        const auto& counselor=cfgprobe::NativeClasses[cfgprobe::Counselor];Pair(counselor.reflectionCacheRva,counselor.constructorCacheRva,counselorType);
        const auto& killerClass=cfgprobe::NativeClasses[cfgprobe::Killer];Pair(killerClass.reflectionCacheRva,killerClass.constructorCacheRva,killerType);
        Object(localPawn,7,counselorType);Object(human,8,types[6]);Object(killer,9,killerType);Object(humanState,10,stateType);Object(hostState,11,stateType);Object(connection,12,connectionType);
        r.Put(a.module+cfgprobe::lifetime::ObjectArrayRva+28,std::int32_t(13));
        for(auto object:{localPawn,human,killer,humanState,hostState})r.Put(object+0x20,a.world);
        r.Put(controller+cfgprobe::ControllerPawn,localPawn);r.Put(localPawn+cfgescape::disk::Controller,controller);
        r.Put(controller+cfgescape::disk::PlayerState,hostState);
        r.Put(human+cfgprobe::ControllerPawn,killer);r.Put(killer+cfgescape::disk::Controller,human);r.Put(human+cfgescape::disk::PlayerState,humanState);
        r.Put(human+d::ControllerPlayer,connection);r.Put(human+b::ControllerConnection,connection);r.Put(connection+d::PlayerController,human);
        r.Put(connection+b::ConnectionDriver,driver);r.Put(connection+b::ConnectionActor,human);
        r.Put(humanState+b::BotFlags,std::uint32_t{});r.Put(hostState+b::BotFlags,std::uint32_t{});
        r.Put(state+b::PlayerArray,h::LocalPlayers{roster,2,8});r.Put(roster,hostState);r.Put(roster+8,humanState);
        r.Put(state+b::CurrentKiller,killer);r.Put(state+b::CurrentKillerState,humanState);
        r.Put(mode+b::NumBots,std::int32_t{});r.Put(mode+b::BotFill,std::uint8_t{});
        r.Put(mode+b::JasonSelectPlayer,humanState);r.Put(mode+b::CounselorBehaviorTree,std::uintptr_t{});
    }
};
