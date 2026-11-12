#pragma once
#include "UrinationAssetObservation.hpp"
#include "../RuntimeProbe/ReflectionObservation.hpp"
#include <type_traits>
namespace cfgurination {
struct SharedSlot {std::uintptr_t address=0;cfgnames::Words original{},cinematic{};};
// Retained assets and the caller's active registry window are required. The
// schema comes from native registered structs, not copied file descriptors.
template<class R>bool ObserveSharedSlot(R& r,std::uintptr_t module,std::uintptr_t montage,
    std::uintptr_t skeleton,SharedSlot& out){
    out={};cfgnames::Observation<R> names(r,module);cfgreflection::Observation<R> reflection(r,module);
    const auto same=[&](std::uintptr_t at,const auto& expected){using T=std::decay_t<decltype(expected)>;T value{};return r.Read(at,value)&&value==expected;};
    const auto named=[&](std::uintptr_t at,std::u16string_view expected){cfgnames::Text text;return names.Object(at,text)&&text.View()==expected;};
    const auto schema=[&](std::uintptr_t getter,std::uintptr_t cache,const auto& code,std::u16string_view label,
        std::u16string_view member,std::u16string_view kind,int offset,int bytes){
        std::decay_t<decltype(code)> actual{};std::uintptr_t structure=0;int size=0;
        cfgreflection::Field field{};struct Shape{int dimension,bytes;std::uint64_t flags;};Shape shape{},again{};int finalOffset=0;
        if(!r.Read(module+getter,actual)||actual!=code||!r.Read(module+cache,structure)||!structure||!named(structure,label)||
            !r.Read(structure+cfgreflection::disk::StorageBytes,size)||size!=24||!reflection.Find(structure,member,field,false)||field.kind.View()!=kind||
            !r.Read(field.object+cfgreflection::disk::PropertyDimensions,shape)||shape.dimension!=1||shape.bytes!=bytes||
            !r.Read(field.object+cfgreflection::disk::PropertyOffset,finalOffset)||finalOffset!=offset||
            !r.Read(field.object+cfgreflection::disk::PropertyDimensions,again)||shape.dimension!=again.dimension||shape.bytes!=again.bytes||shape.flags!=again.flags||
            !same(module+cache,structure)||!same(structure+cfgreflection::disk::StorageBytes,size)||!same(field.object+cfgreflection::disk::PropertyOffset,offset))return false;
        return true;
    };
    const auto schemas=[&](){return schema(disk::SlotAnimationTrackGetter,disk::SlotAnimationTrackCache,disk::SlotAnimationTrackCode,
        u"SlotAnimationTrack",u"SlotName",u"NameProperty",0,8)&&
        schema(disk::AnimSlotGroupGetter,disk::AnimSlotGroupCache,disk::AnimSlotGroupCode,u"AnimSlotGroup",u"SlotNames",u"ArrayProperty",8,16);};
    const auto arrayField=[&](std::uintptr_t object,std::u16string_view member,unsigned offset,cfgspawn::PlayerArray& header){
        std::uintptr_t type=0;cfgreflection::Field field{};cfgreflection::Property property{};
        return r.Read(object+cfgprobe::ObjectClass,type)&&reflection.Find(type,member,field)&&reflection.ReadProperty(field,property)&&
            property.field.kind.View()==u"ArrayProperty"&&property.dimension==1&&property.elementBytes==16&&property.offset==static_cast<int>(offset)&&
            r.Read(object+offset,header)&&same(object+cfgprobe::ObjectClass,type)&&reflection.CurrentProperty(property);
    };
    struct Group {cfgnames::Words group{};cfgspawn::PlayerArray names{};};static_assert(sizeof(Group)==24);
    cfgspawn::PlayerArray tracks{},groups{};std::array<Group,32> group{};
    std::array<std::array<cfgnames::Words,64>,32> slots{};
    if(!montage||!skeleton||!schemas()||!arrayField(montage,u"SlotAnimTracks",static_cast<unsigned>(disk::SlotAnimTracks),tracks)||
        !arrayField(skeleton,u"SlotGroups",static_cast<unsigned>(disk::SlotGroups),groups)||
        tracks.count!=1||tracks.capacity<1||tracks.capacity>4||!cfgprobe::ObjectPointer(tracks.data)||
        groups.count<1||groups.count>32||groups.capacity<groups.count||groups.capacity>64||!cfgprobe::ObjectPointer(groups.data))return false;
    SharedSlot result;result.address=tracks.data;cfgnames::Text text;
    if(!r.Read(tracks.data,result.original)||result.original.number||!names.Decode(result.original,text)||text.View()!=u"MaleNonCinematic")return false;
    unsigned matches=0;
    for(int i=0;i<groups.count;++i){
        if(!r.Read(groups.data+24ULL*i,group[i]))return false;const auto& array=group[i].names;
        if(array.count<0||array.count>64||array.capacity<array.count||array.capacity>128||(array.count&&!cfgprobe::ObjectPointer(array.data)))return false;
        for(int j=0;j<array.count;++j){
            if(!r.Read(array.data+8ULL*j,slots[i][j])||!names.Decode(slots[i][j],text))return false;
            if(!slots[i][j].number&&text.View()==u"Cinematic"){
                if(matches&&!(result.cinematic==slots[i][j]))return false;result.cinematic=slots[i][j];++matches;
            }
        }
    }
    const auto headerSame=[&](std::uintptr_t at,const cfgspawn::PlayerArray& expected){cfgspawn::PlayerArray actual{};return
        r.Read(at,actual)&&actual.data==expected.data&&actual.count==expected.count&&actual.capacity==expected.capacity;};
    if(!matches||!schemas()||!headerSame(montage+disk::SlotAnimTracks,tracks)||!headerSame(skeleton+disk::SlotGroups,groups)||!same(tracks.data,result.original))return false;
    for(int i=0;i<groups.count;++i){
        Group fresh{};if(!r.Read(groups.data+24ULL*i,fresh)||!(fresh.group==group[i].group)||
            fresh.names.data!=group[i].names.data||fresh.names.count!=group[i].names.count||fresh.names.capacity!=group[i].names.capacity)return false;
        for(int j=0;j<fresh.names.count;++j)if(!same(fresh.names.data+8ULL*j,slots[i][j]))return false;
    }
    out=result;return true;
}
}
