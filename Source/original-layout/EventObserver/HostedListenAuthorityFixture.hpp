#pragma once
#include "HostedListenAuthorityObservation.hpp"
#include <map>
#include <cstdio>
#include <cstring>
namespace h=cfghosted::listen35;
namespace n=cfghosted::native;
namespace d=cfghosted::disk35;
struct Memory {
    std::map<std::uintptr_t,unsigned char> bytes;unsigned reads=0,failAt=0,mutateAt=0;
    std::uintptr_t mutateAddress=0;std::uint8_t mutateValue=0;
    template<class T> void Put(std::uintptr_t at,const T& value){
        const auto* p=reinterpret_cast<const unsigned char*>(&value);for(std::size_t i=0;i<sizeof(T);++i)bytes[at+i]=p[i];
    }
    template<class T> bool Read(std::uintptr_t at,T& value){
        if(++reads==mutateAt)Put(mutateAddress,mutateValue);if(reads==failAt)return false;
        T temp{};auto* p=reinterpret_cast<unsigned char*>(&temp);
        for(std::size_t i=0;i<sizeof(T);++i){const auto found=bytes.find(at+i);if(found==bytes.end())return false;p[i]=found->second;}
        value=temp;return true;
    }
};
struct Fixture {
    Memory r;n::Admission a{0x140000000,0x200000,0x210000,0x220000,99,true,true};
    static constexpr std::uintptr_t driver=0x230000,mode=0x240000,state=0x250000,instance=0x260000,player=0x270000,controller=0x280000;
    static constexpr std::uintptr_t table=0x300000,players=0x310000;
    std::array<std::uintptr_t,7> types{{0x400000,0x410000,0x420000,0x430000,0x440000,0x450000,0x460000}};
    void Pair(std::uintptr_t first,std::uintptr_t second,std::uintptr_t type){r.Put(a.module+first,type);r.Put(a.module+second,type);r.Put(type+cfgprobe::ClassSuper,std::uintptr_t{});}
    void Object(std::uintptr_t object,unsigned index,std::uintptr_t type){
        r.Put(object+cfgprobe::ObjectClass,type);r.Put(object+0xC,static_cast<std::int32_t>(index));r.Put(table+24*index,object);r.Put(table+24*index+16,std::int32_t(20+index));
    }
    Fixture(){
        Pair(d::WorldReflection,d::WorldConstructor,types[0]);Pair(d::NetDriverReflection,d::NetDriverConstructor,types[1]);
        Pair(d::HuntReflection,d::HuntConstructor,types[2]);Pair(cfgescape::disk::SCGameStateReflection,cfgescape::disk::SCGameStateConstructor,types[3]);
        Pair(d::SCGameInstanceReflection,d::SCGameInstanceConstructor,types[4]);Pair(d::ILLLocalPlayerReflection,d::ILLLocalPlayerConstructor,types[5]);
        const auto& pc=cfgprobe::NativeClasses[cfgprobe::PlayerControllerClass];Pair(pc.reflectionCacheRva,pc.constructorCacheRva,types[6]);
        Pair(d::PlayerControllerReflection,d::PlayerControllerConstructor,types[6]);
        const std::array<std::uintptr_t,7> objects{{a.world,driver,mode,state,instance,player,controller}};
        for(unsigned i=0;i<objects.size();++i)Object(objects[i],i,types[i]);
        r.Put(a.module+cfgprobe::lifetime::ObjectArrayRva+16,table);r.Put(a.module+cfgprobe::lifetime::ObjectArrayRva+28,std::int32_t(7));
        r.Put(a.module+cfgprobe::dispatch::ThreadFlagRva,std::uint8_t(1));r.Put(a.module+cfgprobe::dispatch::ThreadIdRva,a.ownerThread);
        r.Put(a.module+cfgprobe::EngineGlobalCandidate,a.engine);r.Put(a.engine+cfgprobe::EngineGameViewport,a.viewport);r.Put(a.viewport+cfgprobe::ViewportWorld,a.world);
        r.Put(a.world+d::WorldNetDriver,driver);r.Put(a.world+d::WorldGameMode,mode);r.Put(a.world+d::WorldGameState,state);
        r.Put(mode+d::ActorRole,cfghosted::disk::AuthorityRole);r.Put(mode+cfgescape::disk::MatchState,cfgnames::Words{});
        r.Put(a.module+cfgnames::disk::Global,std::uintptr_t(0x500000));r.Put(0x500400,std::array<std::int32_t,2>{{1,1}});
        r.Put(0x500000,std::uintptr_t(0x510000));r.Put(0x510000,std::uintptr_t(0x520000));r.Put(0x520000,std::uint32_t{});
        std::array<char,16> text{};std::memcpy(text.data(),"InProgress",10);r.Put(0x520010,text);
        r.Put(driver+d::ServerConnection,std::uintptr_t{});r.Put(driver+d::DriverWorld,a.world);
        r.Put(a.world+d::WorldGameInstance,instance);r.Put(instance+d::GameInstanceLocalPlayers,h::LocalPlayers{players,1,1});
        r.Put(players,player);r.Put(player+d::PlayerController,controller);r.Put(controller+d::ControllerPlayer,player);
        r.Put(controller+d::ActorRole,cfghosted::disk::AuthorityRole);r.Put(controller+0x20,a.world);
        r.Put(a.module+d::WorldClassConstructor,d::WorldClassConstructorCode);r.Put(a.module+d::NetDriverClassConstructor,d::NetDriverClassConstructorCode);
        r.Put(a.module+d::HuntClassConstructor,d::HuntClassConstructorCode);r.Put(a.module+d::SCGameInstanceClassConstructor,d::SCGameInstanceClassConstructorCode);
        r.Put(a.module+d::ILLLocalPlayerClassConstructor,d::ILLLocalPlayerClassConstructorCode);
        r.Put(a.module+d::PlayerControllerClassConstructor,d::PlayerControllerClassConstructorCode);
    }
};
