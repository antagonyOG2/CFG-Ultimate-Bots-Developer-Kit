#pragma once
#include "SnapshotBindings.hpp"
#include "DispatchObservation.hpp"
#include "MenuExecution.generated.hpp"
namespace cfgmenu::active {
struct StackArray {
    std::uintptr_t data=0;std::int32_t count=0,capacity=0;
    bool Sane()const noexcept{return cfgprobe::ObjectPointer(data)&&count>0&&count<=64&&capacity>=count&&capacity<=64&&
        cfgprobe::lifetime::Extent(data,static_cast<std::uintptr_t>(capacity)*execdisk::RecordBytes);}
    bool operator==(const StackArray& b)const noexcept{return data==b.data&&count==b.count&&capacity==b.capacity;}
};
struct Context {
    std::uintptr_t module=0,engine=0,viewport=0,world=0,mode=0,gameInstance=0,players=0,player=0,controller=0,hud=0,stack=0,widget=0,type=0;
    std::uint32_t thread=0;StackArray array{};
    cfgprobe::lifetime::ArrayHeader registry{};
    std::array<cfgprobe::lifetime::SlotObservation,4> identities{}; // World/controller/ACTIVE widget/HUD, not metadata/CDOs.
    bool operator==(const Context& b)const noexcept{return module==b.module&&engine==b.engine&&viewport==b.viewport&&world==b.world&&
        mode==b.mode&&gameInstance==b.gameInstance&&players==b.players&&player==b.player&&controller==b.controller&&hud==b.hud&&stack==b.stack&&
        widget==b.widget&&type==b.type&&thread==b.thread&&array==b.array&&identities==b.identities;}
};
// Exact CFG GetCurrentMenu leaf and native fields independently proved on disk.
// Supplied current chain only, not registry discovery, native call or retained
// pointer authority. Caller must re-observe on its native owner-thread callback.
template<class Reader> bool Sample(Reader& r,std::uintptr_t module,Context& out){
    out={};Context c;c.module=module;std::uint8_t flag=0;std::uintptr_t net=1;
    std::int32_t playerCount=0,playerCapacity=0;
    const auto pointer=[&](std::uintptr_t at,std::uintptr_t& value){return r.Read(at,value)&&cfgprobe::ObjectPointer(value);};
    if(!cfgprobe::ObjectPointer(module)||!r.Read(module+cfgprobe::dispatch::ThreadFlagRva,flag)||flag!=1||
        !r.Read(module+cfgprobe::dispatch::ThreadIdRva,c.thread)||!c.thread||
        !pointer(module+cfgprobe::EngineGlobalCandidate,c.engine)||!pointer(c.engine+cfgprobe::EngineGameViewport,c.viewport)||
        !pointer(c.viewport+cfgprobe::ViewportWorld,c.world)||!r.Read(c.world+cfgprobe::WorldNetDriver,net)||net||
        !pointer(c.world+cfgprobe::WorldGameMode,c.mode)||
        !pointer(c.world+cfgprobe::WorldGameInstance,c.gameInstance)||!pointer(c.gameInstance+cfgprobe::GameInstanceLocalPlayers,c.players)||
        !r.Read(c.gameInstance+cfgprobe::GameInstanceLocalPlayers+8,playerCount)||playerCount!=1||
        !r.Read(c.gameInstance+cfgprobe::GameInstanceLocalPlayers+12,playerCapacity)||playerCapacity<1||playerCapacity>4||
        !pointer(c.players,c.player)||!pointer(c.player+cfgprobe::PlayerController,c.controller)||
        !pointer(c.controller+execdisk::ControllerHUD,c.hud)||!pointer(c.hud+execdisk::HUDStack,c.stack)||
        !r.Read(c.stack+execdisk::StackArray,c.array)||!c.array.Sane()||
        !pointer(c.array.data+static_cast<std::uintptr_t>(c.array.count-1)*execdisk::RecordBytes+execdisk::RecordWidget,c.widget)||
        !pointer(c.widget+cfgprobe::ObjectClass,c.type)||
        !cfgprobe::lifetime::ReadArrayHeader(r,module+cfgprobe::lifetime::ObjectArrayRva,c.registry))return false;
    const std::array<std::uintptr_t,4> objects{{c.world,c.controller,c.widget,c.hud}};
    for(unsigned i=0;i<objects.size();++i){
        if(!cfgprobe::lifetime::ReadSlot(r,c.registry,objects[i],c.identities[i])||!c.identities[i].HasAssignedSerial())return false;
    }
    out=c;return true;
}
template<class Reader> bool Observe(Reader& r,std::uintptr_t module,Context& out){
    out={};Context a,b;if(!Sample(r,module,a)||!Sample(r,module,b)||!(a==b))return false;
    // Registry growth does not invalidate known slots; their exact identities
    // were sampled twice. No saved zero-serial widget ever becomes an owner.
    out=b;return true;
}
template<class Reader> bool Current(Reader& r,const Context& expected,std::uint32_t callerThread){
    Context fresh;return callerThread==expected.thread&&Observe(r,expected.module,fresh)&&fresh==expected;
}
}
