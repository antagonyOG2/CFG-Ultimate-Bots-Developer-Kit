#pragma once
#include "KnifeRuntime.hpp"
namespace cfgknife {
struct NativeFlightObservation {FlightSample flight{};cfgspawn::Vec3 velocity{};};
// Diagnostic reads of the SAME identities/fields used by existing KnifeFlight.
// No getters, writes, GC exemption or launch/retention authority.
template<class R>bool ObserveNativeFlight(R& r,const cfgprobe::lifetime::ArrayHeader& objects,
    Identity expected,std::uintptr_t jason,std::uintptr_t movementTable,NativeFlightObservation& out){
    out={};NativeFlightObservation candidate;cfgprobe::lifetime::SlotObservation first{},last{};
    std::uintptr_t root=0,movement=0,owner=0,table=0,thrownBy=0,again=0;cfgspawn::Vec3 point{};std::uint8_t active=0;
    if(!expected.object||expected.serial<=0||!jason||!movementTable||
       !cfgspawn::ObjectSlot(r,objects,expected.object,first)||first.serial!=expected.serial||
       !r.Read(expected.object+disk::ThrownBy,thrownBy)||thrownBy!=jason||
       !r.Read(expected.object+0x160,root)||!root||!r.Read(root+0x20,owner)||owner!=expected.object||
       !r.Read(root+0x190,point)||!cfgspawn::Finite(point)||
       !r.Read(expected.object+0x378,movement)||!movement||!r.Read(movement,table)||table!=movementTable||
       !r.Read(movement+0x20,owner)||owner!=expected.object||
       !r.Read(movement+0x104,candidate.velocity)||!cfgspawn::Finite(candidate.velocity)||
       !r.Read(movement+0xF0,candidate.flight.updated)||(candidate.flight.updated&&candidate.flight.updated!=root)||
       !r.Read(movement+0xAA,active)||
       !r.Read(expected.object+0x160,again)||again!=root||!r.Read(root+0x20,owner)||owner!=expected.object||
       !r.Read(expected.object+0x378,again)||again!=movement||
       !cfgspawn::ObjectSlot(r,objects,expected.object,last)||!(first==last))return false;
    candidate.flight.x=point.x;candidate.flight.y=point.y;candidate.flight.z=point.z;
    candidate.flight.active=(active&1)!=0;
    const auto& velocity=candidate.velocity;
    candidate.flight.speed=std::sqrt(velocity.x*velocity.x+velocity.y*velocity.y+velocity.z*velocity.z);
    if(!std::isfinite(candidate.flight.speed))return false;out=candidate;return true;
}
}
