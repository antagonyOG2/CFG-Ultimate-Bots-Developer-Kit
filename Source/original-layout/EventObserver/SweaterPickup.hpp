#pragma once
#include "../RuntimeProbe/SweaterPickupBindings.generated.hpp"
#include "../RuntimeProbe/ObjectLifetime.hpp"
namespace cfgsweater {
using PickupTable=std::array<std::uintptr_t,pickupdisk::Entries+1>;
inline unsigned PickupCloneAllocations=0;
constexpr unsigned MaximumPickupClones=16;
struct PickupList {
 std::uintptr_t data=0;std::int32_t count=0,capacity=0;
 bool operator==(const PickupList& b)const{return data==b.data&&count==b.count&&capacity==b.capacity;}
};
// WorldItems is mutable as loot is collected. The installation index is not
// an identity. Reacquire only this exact retained item; do not adopt a new one.
template<class R>bool LocatePickup(R& r,std::uintptr_t field,std::uintptr_t wanted,
 PickupList& list,unsigned& index){
 list={};index=~0u;PickupList again{};
 if(!cfgprobe::ObjectPointer(wanted)||!r.Read(field,list)||list.count<=0||list.count>256||
    list.capacity<list.count||list.capacity>65536||!cfgprobe::ObjectPointer(list.data)||
    list.data>UINTPTR_MAX-8ULL*static_cast<unsigned>(list.count))return false;
 for(unsigned i=0;i<static_cast<unsigned>(list.count);++i){std::uintptr_t item=0;
  if(!r.Read(list.data+8ULL*i,item))return false;
  if(item==wanted){if(index!=~0u)return false;index=i;}}
 std::uintptr_t fresh=0;
 return index!=~0u&&r.Read(field,again)&&again==list&&
  r.Read(list.data+8ULL*index,fresh)&&fresh==wanted;
}
struct PickupState {
 cfgprobe::lifetime::SlotObservation source{},level{};std::uintptr_t world=0,module=0,clone=0;
 unsigned cursor=0,index=0,stage=0,scanned=0,installed=0,accepted=0,refused=0,lastFailure=0,basePassed=0,baseDenied=0;
 std::uintptr_t lastInteractor=0;int lastResult=0;
 std::uint64_t deadline=0;bool busy=false;
};
template<class R>bool BuildPickupTable(R& r,std::uintptr_t module,std::uintptr_t replacement,PickupTable& out){
 if(!pickupdisk::RuntimeVerified||!cfgprobe::ObjectPointer(module)||!replacement||module>UINTPTR_MAX-pickupdisk::NextVtable)return false;
 PickupTable table{};if(!r.Read(module+pickupdisk::Vtable-8,table)||table[0]!=module+pickupdisk::Prefix)return false;
 for(unsigned i=0;i<pickupdisk::Entries;++i)if(table[i+1]!=module+pickupdisk::VtableRvas[i])return false;
 table[1+pickupdisk::Slot/8]=replacement;out=table;return true;
}
inline bool PickupScope(const PickupState& s,std::uintptr_t module,std::uintptr_t world,std::uintptr_t item){
 return s.installed==1&&s.clone&&s.module==module&&s.world==world&&s.source.object==item;
}
}
