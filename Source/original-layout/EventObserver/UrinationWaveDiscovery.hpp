#pragma once
#include "UrinationAssetObservation.hpp"
namespace cfgurination {
// Bounded native equipped/default catalogue only. No global object scanning,
// soft lookup or class creation. Caller must retain the result in THIS window.
template<class R>bool DiscoverWave(R& r,const cfgprobe::lifetime::ArrayHeader& objects,std::uintptr_t module,
    std::uintptr_t pawn,std::uintptr_t counselorClass,std::uintptr_t catalogueOffset,std::array<std::uintptr_t,3>& output){
    output={};cfgprobe::lifetime::SlotObservation source{},fresh{};cfgspawn::PlayerArray entries{},again{};
    std::array<std::uintptr_t,64> classes{};cfgnames::Observation<R> names(r,module);
    if(!cfgspawn::ObjectSlot(r,objects,pawn,source)||!source.HasAssignedSerial()||!cfgspawn::ObjectIs(r,pawn,counselorClass)||
        !r.Read(pawn+catalogueOffset,entries)||entries.count<1||entries.count>64||
        entries.capacity<entries.count||entries.capacity>128||!cfgprobe::ObjectPointer(entries.data))return false;
    std::array<std::uintptr_t,3> found{};unsigned matches=0;
    for(int i=0;i<entries.count;++i){
        if(!r.Read(entries.data+8ULL*i,classes[i]))return false;if(!classes[i])continue;
        cfgprobe::lifetime::SlotObservation slot{};cfgnames::Text name;
        if(!cfgspawn::ObjectSlot(r,objects,classes[i],slot)||!names.Object(classes[i],name))return false;
        if(name.View()!=u"Emote_Wave_C")continue;
        std::uintptr_t defaults=0,male=0,female=0;AssetProof proof{};
        if(!r.Read(classes[i]+cfgspawn::ClassDefaultObject,defaults)||!r.Read(defaults+disk::MaleMontage,male)||
            !r.Read(defaults+disk::FemaleMontage,female))return false;
        if(!ObserveAssets(r,objects,module,{classes[i],male,female},0,proof)){
            // Donor preserves ordinary female Wave when its optional shared
            // skeleton/layout proof is unavailable; the proven male path works.
            female=0;if(!ObserveAssets(r,objects,module,{classes[i],male,0},0,proof))return false;
        }
        if(matches&&found!=std::array<std::uintptr_t,3>{classes[i],male,female})return false;
        found={classes[i],male,female};++matches;
    }
    if(!matches||!r.Read(pawn+catalogueOffset,again)||again.data!=entries.data||again.count!=entries.count||
        again.capacity!=entries.capacity||!cfgspawn::ObjectSlot(r,objects,pawn,fresh)||!(fresh==source)||!cfgspawn::ObjectIs(r,pawn,counselorClass))return false;
    for(int i=0;i<entries.count;++i){std::uintptr_t current=0;if(!r.Read(entries.data+8ULL*i,current)||current!=classes[i])return false;}
    output=found;return true;
}
}
