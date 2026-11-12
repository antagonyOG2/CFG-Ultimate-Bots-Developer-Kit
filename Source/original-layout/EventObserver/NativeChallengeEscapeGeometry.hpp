#pragma once
#include "../RuntimeProbe/ChallengeGeometryInputs.generated.hpp"
#include "../Core/ChallengeEscapeBoundaryPolicy.hpp"
#include <cstring>
namespace cfgchallengeescape::geometry {
static_assert(geometrydisk::Scale==geometrydisk::Translation+16,"native geometry inputs must remain contiguous");
// Cache only numeric getter results. No pointer, metadata or action authority is
// borrowed from this record. Every reuse freshly verifies the sources and ALL
// memory inputs read by the exact native location/radius getters.
struct Inputs {
 std::array<unsigned char,32> transform{};float radius=0;
 std::array<std::uint32_t,4> mask{};
 bool operator==(const Inputs& b)const{return transform==b.transform&&radius==b.radius&&mask==b.mask;}
};
template<class R>bool ReadInputs(R& r,std::uintptr_t module,std::uintptr_t sphere,Inputs& out){
 std::array<unsigned char,36> body{};
 Inputs value;
 if(!r.ReadBytes(module+geometrydisk::LocationBody,body.data(),body.size())||body!=geometrydisk::LocationCode||
  !r.Read(module+geometrydisk::AbsMask,value.mask))return false;
 if(!r.ReadBytes(sphere+geometrydisk::Translation,value.transform.data(),value.transform.size())||
  !r.Read(sphere+geometrydisk::Radius,value.radius)||!std::isfinite(value.radius)||value.radius<=0)return false;
 float components[8]{};std::memcpy(components,value.transform.data(),sizeof(components));
 for(unsigned i=0;i<8;++i)if(i!=3&&i!=7&&!std::isfinite(components[i]))return false;
 out=value;return true;
}
struct Entry {
 bool valid=false;boundary::Scope scope{};boundary::Identity volume{},sphere{};
 Inputs inputs{};boundary::Sphere value{};
};
}
