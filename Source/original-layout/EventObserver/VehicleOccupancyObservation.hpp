#pragma once
#include "BundleObservations.hpp"
#include "../RuntimeProbe/VehicleInteractionBindings.generated.hpp"
#include "../RuntimeProbe/VehicleOccupancyBindings.generated.hpp"
namespace cfgvehicleoccupancy {
enum class Result {Unknown,Empty,Exiting,Occupied};
// Caller validates offline/thread/native authority, car source/type and class
// caches. Occupied additionally requires fresh caller-owned terminal-state proof.
// This observer makes no native calls and never reads car.Driver.
struct Seat {
    cfgprobe::lifetime::SlotObservation slot{},occupantSlot{};
    std::uintptr_t occupant=0,currentSeat=0;
    std::uint8_t driver=0,leaving=0;
    bool operator==(const Seat& b)const{return slot==b.slot&&occupantSlot==b.occupantSlot&&occupant==b.occupant&&currentSeat==b.currentSeat&&driver==b.driver&&leaving==b.leaving;}
};
struct Proof {
    Result result=Result::Unknown;
    cfgprobe::lifetime::SlotObservation carSlot{};
    cfgspawn::PlayerArray seats{};
    std::array<Seat,8> entries{};
    std::uintptr_t seat=0,driver=0;
    bool operator==(const Proof& b)const{return result==b.result&&carSlot==b.carSlot&&seats.data==b.seats.data&&seats.count==b.seats.count&&seats.capacity==b.seats.capacity&&entries==b.entries&&seat==b.seat&&driver==b.driver;}
};
template<class R> bool Sample(R& r,const cfgprobe::lifetime::ArrayHeader& registry,
    std::uintptr_t car,std::uintptr_t seatType,std::uintptr_t counselorType,Proof& out){
    namespace d=cfgvehicleinteraction::disk;out={};Proof p;
    if(!seatType||!counselorType||!cfgspawn::ObjectSlot(r,registry,car,p.carSlot)||!p.carSlot.HasAssignedSerial()||
       !r.Read(car+d::Seats,p.seats)||p.seats.count<1||p.seats.count>8||p.seats.capacity<p.seats.count||p.seats.capacity>16||
       !cfgprobe::lifetime::Extent(p.seats.data,8ULL*static_cast<unsigned>(p.seats.count)))return false;
    for(int i=0;i<p.seats.count;++i){auto& s=p.entries[i];std::uintptr_t object=0,parent=0;
        if(!r.Read(p.seats.data+8ULL*i,object)||!cfgspawn::ObjectSlot(r,registry,object,s.slot)||!s.slot.HasAssignedSerial()||
           !cfgspawn::ObjectIs(r,object,seatType)||!r.Read(object+d::ParentVehicle,parent)||parent!=car||
           !r.Read(object+disk::DriverSeat,s.driver)||s.driver>1||!r.Read(object+d::CounselorInSeat,s.occupant))return false;
        for(int j=0;j<i;++j)if(p.entries[j].slot.object==object)return false;
        if(!s.driver)continue;
        if(p.seat)return false;
        p.seat=object;p.driver=s.occupant;
        if(!s.occupant){p.result=Result::Empty;continue;}
        if(!cfgspawn::ObjectSlot(r,registry,s.occupant,s.occupantSlot)||!s.occupantSlot.HasAssignedSerial()||
           !cfgspawn::ObjectIs(r,s.occupant,counselorType)||!r.Read(s.occupant+disk::CurrentSeat,s.currentSeat)||
           !r.Read(s.occupant+disk::LeavingSeat,s.leaving)||s.leaving>1)return false;
        p.result=s.leaving?Result::Exiting:(s.currentSeat==object?Result::Occupied:Result::Empty);
    }
    if(!p.seat)return false;
    out=p;return true;
}
template<class R> Result Observe(R& r,const cfgprobe::lifetime::ArrayHeader& registry,
    std::uintptr_t car,std::uintptr_t seatType,std::uintptr_t counselorType,Proof& out){
    out={};Proof first,second;
    if(!Sample(r,registry,car,seatType,counselorType,first)||!Sample(r,registry,car,seatType,counselorType,second)||!(first==second))return Result::Unknown;
    out=second;return out.result;
}
// Fresh whole-proof comparison immediately before effects; includes slot
// serials, array membership, class/owner and reciprocal occupancy edges.
template<class R> bool Current(R& r,const cfgprobe::lifetime::ArrayHeader& registry,
    std::uintptr_t seatType,std::uintptr_t counselorType,const Proof& proof){
    if(proof.result==Result::Unknown)return false;Proof fresh;
    return Sample(r,registry,proof.carSlot.object,seatType,counselorType,fresh)&&fresh==proof;
}
}
