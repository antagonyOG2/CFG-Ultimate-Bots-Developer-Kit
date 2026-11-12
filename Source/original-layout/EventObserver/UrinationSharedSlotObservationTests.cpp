#define CFG_URINATION_ASSET_FIXTURE_ONLY
#include "UrinationAssetObservationTests.cpp"
#include "UrinationSharedSlotObservation.hpp"
namespace {
struct SharedFixture:Fixture {
    static constexpr std::uintptr_t Track=0x600000,Group=0x610000,TrackName=0x620000,GroupNames=0x630000,
        MontageArray=0x640000,SkeletonArray=0x650000,NameType=0x660000,ArrayType=0x670000,
        Tracks=0x680000,Groups=0x690000,SlotNames=0x6A0000,NameObject=0x6B0000;
    cfgnames::Words original{},cinematic{};
    void Property(std::uintptr_t object,std::uintptr_t owner,std::uintptr_t type,const char* label,int offset,int size){
        struct Header{std::uintptr_t type;cfgnames::Words name;std::uintptr_t owner,next;};
        Name(object,label);cfgnames::Words word{};CHECK(r.Read(object+cfgnames::disk::ObjectName,word));
        r.Set(object+cfgprobe::ObjectClass,Header{type,word,owner,0});
        r.Set(object+cfgreflection::disk::PropertyDimensions,std::array<int,4>{1,size,0,0});
        r.Set(object+cfgreflection::disk::PropertyOffset,offset);r.Set(owner+cfgreflection::disk::Children,object);
    }
    SharedFixture(){using namespace cfgurination;
        Name(Track,"SlotAnimationTrack");Name(Group,"AnimSlotGroup");Name(NameType,"NameProperty");Name(ArrayType,"ArrayProperty");
        r.Set(Track+cfgreflection::disk::StorageBytes,int{24});r.Set(Group+cfgreflection::disk::StorageBytes,int{24});
        r.Set(Module+disk::SlotAnimationTrackGetter,disk::SlotAnimationTrackCode);r.Set(Module+disk::SlotAnimationTrackCache,Track);
        r.Set(Module+disk::AnimSlotGroupGetter,disk::AnimSlotGroupCode);r.Set(Module+disk::AnimSlotGroupCache,Group);
        Property(TrackName,Track,NameType,"SlotName",0,8);Property(GroupNames,Group,ArrayType,"SlotNames",8,16);
        Property(MontageArray,Montage,ArrayType,"SlotAnimTracks",static_cast<int>(disk::SlotAnimTracks),16);
        Property(SkeletonArray,SkeletonType,ArrayType,"SlotGroups",static_cast<int>(disk::SlotGroups),16);
        r.Set(Loaded+disk::SlotAnimTracks,cfgspawn::PlayerArray{Tracks,1,1});r.Set(Skeleton+disk::SlotGroups,cfgspawn::PlayerArray{Groups,1,1});
        Name(NameObject,"MaleNonCinematic");CHECK(r.Read(NameObject+cfgnames::disk::ObjectName,original));r.Set(Tracks,original);
        Name(NameObject,"Cinematic");CHECK(r.Read(NameObject+cfgnames::disk::ObjectName,cinematic));
        struct GroupValue{cfgnames::Words group;cfgspawn::PlayerArray slots;};r.Set(Groups,GroupValue{{}, {SlotNames,1,1}});r.Set(SlotNames,cinematic);
        r.Set(Names+cfgnames::disk::ElementCount,std::array<std::int32_t,2>{64,1});r.cap=8192;r.reads=0;
    }
    bool Observe(cfgurination::SharedSlot& proof){return cfgurination::ObserveSharedSlot(r,Module,Loaded,Skeleton,proof);}
};
}
#ifndef CFG_URINATION_SHARED_FIXTURE_ONLY
int main(){using namespace cfgurination;unsigned peak=0;
    {SharedFixture f;SharedSlot proof;CHECK(f.Observe(proof)&&proof.address==f.Tracks&&proof.original==f.original&&proof.cinematic==f.cinematic);peak=f.r.reads;}
    for(unsigned fault=0;fault<13;++fault){SharedFixture f;SharedSlot proof;
        if(fault==0)f.r.Set(f.Track+cfgreflection::disk::StorageBytes,int{32});
        if(fault==1)f.r.Set(f.TrackName+cfgreflection::disk::PropertyOffset,int{8});
        if(fault==2)f.r.Set(f.GroupNames+cfgreflection::disk::PropertyDimensions,std::array<int,4>{2,16,0,0});
        if(fault==3)f.r.Set(f.Loaded+disk::SlotAnimTracks,cfgspawn::PlayerArray{f.Tracks,2,2});
        if(fault==4)f.r.Set(f.Skeleton+disk::SlotGroups,cfgspawn::PlayerArray{f.Groups,33,33});
        if(fault==5)f.r.Set(f.Groups+8,cfgspawn::PlayerArray{f.SlotNames,65,65});
        if(fault==6)f.r.Set(f.Tracks,f.cinematic);
        if(fault==7)f.r.Set(f.SlotNames,f.original);
        if(fault==8)f.r.Set(f.Module+disk::SlotAnimationTrackCache,f.Group);
        if(fault==9)f.r.Set(f.Module+disk::AnimSlotGroupGetter,std::array<unsigned char,47>{});
        if(fault==10){f.r.changeAt=f.SlotNames;f.r.changeSlot=f.Skeleton+disk::SlotGroups;f.r.newPointer=f.Tracks;}
        if(fault==11){f.r.changeAt=f.SlotNames;f.r.changeSlot=f.Tracks;f.r.newPointer=0;}
        if(fault==12)f.r.cap=32;
        CHECK(!f.Observe(proof)&&!proof.address);peak=(std::max)(peak,f.r.reads);
    }
    std::printf("Actual native shared-gender slot observation: %u failures, peak%u/8192; registered script structs, bounded arrays, same native original slot, existing Cinematic FName and resampled storage. No writes/native authority.\n",failures,peak);
    return failures?1:0;
}
#endif
