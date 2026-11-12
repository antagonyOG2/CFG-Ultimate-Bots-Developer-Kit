#pragma once
#include "NativeSpawnParameters.hpp"
#include <array>
#include <cstddef>
namespace cfgcabin {
struct Packet {std::uint8_t onlyColliding=0,padding[3]{};cfgspawn::Vec3 origin{},extent{};};
static_assert(sizeof(Packet)==28&&offsetof(Packet,origin)==4&&offsetof(Packet,extent)==16);
struct Bounds {
 cfgspawn::Vec3 origin{},extent{};
 bool Valid()const{return cfgspawn::Finite(origin)&&cfgspawn::Finite(extent)&&
  std::fabs(origin.x)<10000000&&std::fabs(origin.y)<10000000&&std::fabs(origin.z)<10000000&&
  extent.x>0&&extent.y>0&&extent.z>0&&extent.x<1000000&&extent.y<1000000&&extent.z<1000000;}
 // Conservative exterior footprint. Vertical extrusion also excludes roofs;
 // 100cm clearance prevents a capsule centre just outside a wall intersecting it.
 bool Contains(cfgspawn::Vec3 p)const{return std::fabs(p.x-origin.x)<=extent.x+100.f&&std::fabs(p.y-origin.y)<=extent.y+100.f;}
};
enum class Stage:unsigned {Dormant,Survey,Capture,Ready,Refused};
struct Cache {
 static constexpr unsigned MaximumCabins=128;
 std::array<Bounds,MaximumCabins> bounds{};
 std::uintptr_t world=0;Stage stage=Stage::Dormant;unsigned count=0,captured=0,failure=0,calls=0;
 void Refuse(unsigned why){failure=why;stage=Stage::Refused;}
 bool Add(const Packet& packet){Bounds value{packet.origin,packet.extent};if(captured>=count||!value.Valid()){Refuse(9);return false;}bounds[captured++]=value;return true;}
 bool Allows(std::uintptr_t currentWorld,cfgspawn::Vec3 point)const{
  if(stage!=Stage::Ready||!world||world!=currentWorld||!cfgspawn::Finite(point)||captured!=count)return false;
  for(unsigned i=0;i<count;++i)if(bounds[i].Contains(point))return false;
  return true;
 }
 bool Encloses(std::uintptr_t currentWorld,cfgspawn::Vec3 point)const{
  if(stage!=Stage::Ready||!world||world!=currentWorld||!cfgspawn::Finite(point)||captured!=count)return false;
  for(unsigned i=0;i<count;++i)if(bounds[i].Contains(point)&&
      std::fabs(point.z-bounds[i].origin.z)<=bounds[i].extent.z+100.f)return true;
  return false;
 }
};
}
