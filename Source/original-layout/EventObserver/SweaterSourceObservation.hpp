#pragma once
#include "../RuntimeProbe/FeatureSnapshotBindings.generated.hpp"
#include "../RuntimeProbe/RestrictedLoadoutBindings.generated.hpp"
#include "../RuntimeProbe/SnapshotBindings.hpp"
#include "../RuntimeProbe/ObjectLifetime.hpp"
namespace cfgsweater {
// Retains an unassigned-serial source ONLY through its CURRENT strong world
// ItemList cell. The list may relocate/grow; exact current header/cell are
// sampled twice. The saved index may never silently adopt another source.
template<class Reader> bool StrongSource(Reader& r,const cfgprobe::lifetime::ArrayHeader& objects,
    std::uintptr_t world,std::uintptr_t state,std::uintptr_t human,
    const cfgprobe::lifetime::SlotObservation& source,const cfgprobe::lifetime::SlotObservation& ability,unsigned index){
    struct List {std::uintptr_t data=0;std::int32_t count=0,capacity=0;
        bool operator==(const List& b)const{return data==b.data&&count==b.count&&capacity==b.capacity;}};
    List list{},fresh{};std::uintptr_t value=0,owner=0;cfgprobe::lifetime::SlotObservation sourceAgain{},abilityAgain{};
    if(!cfgprobe::ObjectPointer(source.object)||!cfgprobe::ObjectPointer(ability.object)||
       !r.Read(world+cfgprobe::WorldGameState,value)||value!=state||
       !r.Read(state+cfgfeature::disk::WorldItems,list)||list.count<0||list.count>256||list.capacity<list.count||list.capacity>65536||
       index>=static_cast<unsigned>(list.count)||!cfgprobe::ObjectPointer(list.data)||
       !r.Read(list.data+8*index,value)||value!=source.object||
       !r.Read(source.object+cfgfeature::disk::SweaterAbility,value)||value!=ability.object||
       !r.Read(source.object+cfgloadout::restricteddisk::OwningCharacter,owner)||(owner&&owner!=human)||
       !cfgprobe::lifetime::ReadSlot(r,objects,source.object,sourceAgain)||!(sourceAgain==source)||
       !cfgprobe::lifetime::ReadSlot(r,objects,ability.object,abilityAgain)||!(abilityAgain==ability)||
       !r.Read(state+cfgfeature::disk::WorldItems,fresh)||!(fresh==list)||
       !r.Read(list.data+8*index,value)||value!=source.object||
       !r.Read(source.object+cfgfeature::disk::SweaterAbility,value)||value!=ability.object||
       !r.Read(source.object+cfgloadout::restricteddisk::OwningCharacter,value)||value!=owner||
       !r.Read(world+cfgprobe::WorldGameState,value)||value!=state)return false;
    return true;
}
}
