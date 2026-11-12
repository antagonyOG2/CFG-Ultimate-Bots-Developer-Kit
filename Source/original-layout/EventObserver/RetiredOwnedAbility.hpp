#pragma once
#include "../RuntimeProbe/ObjectLifetime.hpp"
namespace cfgsweater {
struct RegistryItem{std::uintptr_t object=0;std::uint32_t flags=0;int cluster=0,serial=0,padding=0;
 bool operator==(const RegistryItem& b)const{return object==b.object&&flags==b.flags&&cluster==b.cluster&&serial==b.serial;}};
static_assert(sizeof(RegistryItem)==24);
// Explicit retirement proof from the CAPTURED registry index; never dereference
// a freed instance and never interpret a failed/budgeted read as retirement.
template<class R>bool Retired(R& r,std::uintptr_t module,const cfgprobe::lifetime::SlotObservation& known){
 if(!known.object||known.index<0)return false;
 cfgprobe::lifetime::ArrayHeader a{},b{};RegistryItem first{},again{};
 if(!cfgprobe::lifetime::ReadArrayHeader(r,module+cfgprobe::lifetime::ObjectArrayRva,a))return false;
 if(known.index>=a.count)return cfgprobe::lifetime::ReadArrayHeader(r,module+cfgprobe::lifetime::ObjectArrayRva,b)&&a==b;
 const auto at=a.data+24ULL*static_cast<unsigned>(known.index);
 if(!r.Read(at,first)||!r.Read(at,again)||!(first==again)||
    !cfgprobe::lifetime::ReadArrayHeader(r,module+cfgprobe::lifetime::ObjectArrayRva,b)||!(a==b))return false;
 return first.object!=known.object||(first.flags&0x30000000u)!=0||
  (known.serial>0&&first.serial>0&&first.serial!=known.serial);
}
// A pawn may still hold the retired address. Replacing that field requires
// stronger proof than "the OLD serial is gone": a new live object at the same
// address is a native replacement and must win. Unknown reads also refuse.
template<class R>bool RetiredInPlace(R& r,std::uintptr_t module,const cfgprobe::lifetime::SlotObservation& known){
 if(!known.object||known.index<0)return false;
 int index=-1;cfgprobe::lifetime::ArrayHeader a{},b{};RegistryItem first{},again{};
 if(!r.Read(known.object+cfgprobe::lifetime::ObjectIndexOffset,index)||index!=known.index||
    !cfgprobe::lifetime::ReadArrayHeader(r,module+cfgprobe::lifetime::ObjectArrayRva,a)||index>=a.count)return false;
 const auto at=a.data+24ULL*static_cast<unsigned>(index);
 if(!r.Read(at,first)||!r.Read(at,again)||!(first==again)||
    !cfgprobe::lifetime::ReadArrayHeader(r,module+cfgprobe::lifetime::ObjectArrayRva,b)||!(a==b)||
    !r.Read(known.object+cfgprobe::lifetime::ObjectIndexOffset,index)||index!=known.index)return false;
 return first.object!=known.object||(first.flags&0x30000000u)!=0;
}
}
