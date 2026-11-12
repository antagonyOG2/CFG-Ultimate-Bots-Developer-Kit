#pragma once
#include "BundleObservations.hpp"
#include "CounselorTerminalObservation.hpp"
#include "ShackTrapSource.hpp"
#include "../RuntimeProbe/VehicleBindings.generated.hpp"
#include "../RuntimeProbe/VehicleInteractionBindings.generated.hpp"
namespace cfgvehicle {
// CFG named descriptors: SCGameMode.SpawnedVehicles 0x02DAE0A0 ->
// 0x7C0 (not the donor's 0x720); SCDriveableVehicle.VehicleType -> 0x3D8.
constexpr std::uintptr_t SpawnedVehicles=0x7C0,VehicleType=0x3D8;
struct RosterProof {
    bool valid=false,cached=false;
    std::uintptr_t module=0,world=0,mode=0,gameState=0,source=0,type=0,level=0;
    cfgprobe::lifetime::SlotObservation slot{};
    cfgprobe::Ancestry ancestry{};
    cfgshacktrap::List roster{},seats{};
    std::array<std::uintptr_t,16> entries{};
};
template<class R> bool RosterCurrent(R& r,const cfgprobe::lifetime::ArrayHeader& registry,const RosterProof& p){
    if(!p.valid||!p.slot.HasAssignedSerial()||!p.seats.Sane(16)||p.seats.count<1||
       ((p.seats.count>2)!=(p.source==disk::Car4)))return false;
    cfgprobe::lifetime::SlotObservation now{};cfgshacktrap::List list{};
    if(!cfgobserver::Same(r,p.module+disk::VehicleReflection,p.type)||!cfgobserver::Same(r,p.module+disk::VehicleConstructor,p.type)||
       !cfgspawn::ObjectSlot(r,registry,p.slot.object,now)||!(now==p.slot)||
       !cfgprobe::StableAncestry(r,p.slot.object,p.ancestry)||!p.ancestry.Contains(p.type)||
       !cfgobserver::Same(r,p.slot.object+0x20,p.level)||!cfgobserver::Same(r,p.level+cfgkill::disk::OwningWorld,p.world)||
       !cfgescape::Same(r,p.slot.object+VehicleType,std::uint8_t{1})||
       !r.Read(p.slot.object+cfgvehicleinteraction::disk::Seats,list)||!(list==p.seats)||
       !cfgobserver::Same(r,p.gameState+p.source,p.cached?p.slot.object:std::uintptr_t{}))return false;
    if(!p.cached){
        if(!r.Read(p.mode+SpawnedVehicles,list)||!(list==p.roster))return false;
        for(int i=0;i<p.roster.count;++i)
            if(!cfgobserver::Same(r,p.roster.data+8*static_cast<unsigned>(i),p.entries[i]))return false;
        if(!r.Read(p.mode+SpawnedVehicles,list)||!(list==p.roster))return false;
    }
    return cfgspawn::ObjectSlot(r,registry,p.slot.object,now)&&now==p.slot;
}
template<class R> bool RosterCar(R& r,const cfgprobe::lifetime::ArrayHeader& registry,
    std::uintptr_t module,std::uintptr_t world,std::uintptr_t mode,std::uintptr_t gameState,
    std::uintptr_t source,std::uintptr_t& out,RosterProof* proof=nullptr){
    if(proof)*proof={};
    out=0;if(source!=disk::Car2&&source!=disk::Car4)return false;
    std::uintptr_t type=0,ctor=0;
    if(!r.Read(module+disk::VehicleReflection,type)||!r.Read(module+disk::VehicleConstructor,ctor)||
       !(type=cfgprobe::MatchingClassCache(type,ctor)))return false;
    const auto classify=[&](std::uintptr_t car){
        cfgprobe::lifetime::SlotObservation slot{},again{};cfgshacktrap::List seats{};
        std::uintptr_t level=0,owner=0;unsigned char kind=0;
        return cfgspawn::ObjectSlot(r,registry,car,slot)&&slot.HasAssignedSerial()&&cfgspawn::ObjectIs(r,car,type)&&
            r.Read(car+0x20,level)&&level&&r.Read(level+cfgkill::disk::OwningWorld,owner)&&owner==world&&
            r.Read(car+VehicleType,kind)&&kind==1&&r.Read(car+cfgvehicleinteraction::disk::Seats,seats)&&seats.Sane(16)&&seats.count>0&&
            ((seats.count>2)==(source==disk::Car4))&&cfgspawn::ObjectSlot(r,registry,car,again)&&again==slot;
    };
    RosterProof observation;observation.module=module;observation.world=world;observation.mode=mode;
    observation.gameState=gameState;observation.source=source;observation.type=type;
    const auto publish=[&](std::uintptr_t car){
        if(!proof){out=car;return true;}
        if(!car)return true;
        if(!cfgspawn::ObjectSlot(r,registry,car,observation.slot)||
           !cfgprobe::CaptureAncestry(r,car,observation.ancestry)||
           !r.Read(car+0x20,observation.level)||!r.Read(car+cfgvehicleinteraction::disk::Seats,observation.seats))return false;
        observation.valid=true;if(!RosterCurrent(r,registry,observation))return false;
        *proof=observation;out=car;return true;
    };
    std::uintptr_t cached=0;
    if(!r.Read(gameState+source,cached))return false;
    if(cached){if(!classify(cached)||!cfgobserver::Same(r,gameState+source,cached))return false;observation.cached=true;return publish(cached);}
    // OfflineBots can leave both inherited GameState caches empty. Resolve
    // the authoritative mode roster without publishing or changing caches.
    cfgshacktrap::List list{},again{};
    if(!r.Read(mode+SpawnedVehicles,list)||!list.Sane(16))return false;
    std::uintptr_t found=0,cell=0;
    for(int i=0;i<list.count;++i){std::uintptr_t car=0;
        if(!r.Read(list.data+8*static_cast<unsigned>(i),car))return false;
        observation.entries[i]=car;
        if(!car||!classify(car))continue;
        if(found&&found!=car)return false;
        found=car;cell=list.data+8*static_cast<unsigned>(i);
    }
    if(!r.Read(mode+SpawnedVehicles,again)||!(again==list)||!cfgobserver::Same(r,gameState+source,std::uintptr_t{}))return false;
    if(found&&(!cfgobserver::Same(r,cell,found)||!classify(found)))return false;
    observation.roster=list;return publish(found);
}
// Same consumer as the donor: the stock HUD getters read GameState caches,
// not SCGameMode.SpawnedVehicles. Only publish the exact native-owned car
// already proved above, into an empty cache. Never replace a foreign entry.
template<class R,class Host> bool PublishRosterCache(R& r,const cfgprobe::lifetime::ArrayHeader& registry,
    const RosterProof& p,Host& host){
    if(!host.Current()||!RosterCurrent(r,registry,p))return false;
    if(p.cached)return true;
    if(!host.Exchange(p.gameState+p.source,std::uintptr_t{},p.slot.object))return false;
    auto published=p;published.cached=true;
    return host.Current()&&RosterCurrent(r,registry,published);
}
}
