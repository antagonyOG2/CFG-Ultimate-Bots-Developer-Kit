#include "UrinationAssetObservation.hpp"
#include "OwnedByteReader.hpp"
#include <cstdio>
namespace {
unsigned failures=0;
#define CHECK(x) do{if(!(x)){++failures;std::printf("FAIL %u %s\n",__LINE__,#x);}}while(false)
struct Reader:OwnedByteReader {unsigned cap=1536;
    template<class T>bool Read(std::uintptr_t at,T& out){return reads<cap&&OwnedByteReader::Read(at,out);}
};
struct Fixture {
    static constexpr std::uintptr_t Module=0x100000,Registry=0x200000,Names=0x300000,Chunk=0x310000,
        Emote=0x400000,Montage=0x410000,SkeletonType=0x420000,Wave=0x430000,Defaults=0x440000,
        Male=0x450000,Female=0x460000,Skeleton=0x470000,Loaded=0x480000;
    Reader r;cfgprobe::lifetime::ArrayHeader objects{Registry,9};unsigned names=0;
    void Slot(std::uintptr_t object,unsigned index,std::uintptr_t type){r.Set(object+12,static_cast<int>(index));
        r.Set(Registry+24ULL*index,object);r.Set(Registry+24ULL*index+8,std::uint32_t{});
        r.Set(Registry+24ULL*index+16,std::int32_t{});r.Set(object+cfgprobe::ObjectClass,type);}
    void Name(std::uintptr_t object,const char* name){const auto index=++names;const auto entry=0x500000+index*256;
        r.Set(Chunk+8ULL*index,static_cast<std::uintptr_t>(entry));r.Set(entry,std::uint32_t{});
        std::array<char,128> text{};for(unsigned i=0;name[i]&&i<127;++i)text[i]=name[i];
        r.Set(entry+cfgnames::disk::EntryText,text);r.Set(object+cfgnames::disk::ObjectName,cfgnames::Words{static_cast<int>(index),0});}
    Fixture(){using namespace cfgurination;
        r.Set(Module+cfgnames::disk::Global,Names);r.Set(Names,Chunk);
        r.Set(Names+cfgnames::disk::ElementCount,std::array<std::int32_t,2>{32,1});
        unsigned index=0;for(const auto object:{Emote,Montage,SkeletonType,Wave,Defaults,Male,Female,Skeleton,Loaded})Slot(object,index++,0);
        for(const auto type:{Emote,Montage,SkeletonType})r.Set(type+cfgprobe::ClassSuper,std::uintptr_t{});
        r.Set(Wave+cfgprobe::ClassSuper,Emote);r.Set(Defaults+cfgprobe::ObjectClass,Wave);
        for(const auto object:{Male,Female,Loaded})r.Set(object+cfgprobe::ObjectClass,Montage);
        r.Set(Skeleton+cfgprobe::ObjectClass,SkeletonType);
        r.Set(Module+disk::SCEmoteDataReflection,Emote);r.Set(Module+disk::SCEmoteDataConstructor,Emote);
        r.Set(Module+disk::AnimMontageReflection,Montage);r.Set(Module+disk::AnimMontageConstructor,Montage);
        r.Set(Module+disk::SkeletonReflection,SkeletonType);r.Set(Module+disk::SkeletonConstructor,SkeletonType);
        Name(Wave,"Emote_Wave_C");Name(Defaults,"Default__Emote_Wave_C");Name(Male,"AM_m_normal_stand_emote_wave");
        Name(Female,"AM_f_normal_stand_emote_wave");Name(Loaded,"m_normal_stand_piss_Montage");
        r.Set(Wave+cfgspawn::ClassDefaultObject,Defaults);r.Set(Defaults+disk::MaleMontage,Male);r.Set(Defaults+disk::FemaleMontage,Female);
        for(const auto montage:{Male,Female,Loaded})r.Set(montage+disk::Skeleton,Skeleton);r.reads=0;
    }
    bool Observe(cfgurination::AssetProof& proof,std::uintptr_t female=Female,std::uintptr_t loaded=Loaded){return
        cfgurination::ObserveAssets(r,objects,Module,{Wave,Male,female},loaded,proof);}
};
}
#ifndef CFG_URINATION_ASSET_FIXTURE_ONLY
int main(){using namespace cfgurination;unsigned peak=0;
    {Fixture f;AssetProof proof;CHECK(f.Observe(proof)&&proof.wave.object==f.Wave&&proof.loaded.object==f.Loaded&&proof.skeleton.object==f.Skeleton);
        CHECK(!proof.wave.HasAssignedSerial());peak=f.r.reads;CHECK(peak<1536);}
    {Fixture f;AssetProof proof;CHECK(f.Observe(proof,0)&&!proof.female.object);}
    {Fixture f;AssetProof proof;CHECK(f.Observe(proof,f.Female,0)&&!proof.loaded.object);}
    {Fixture f;AssetProof proof;f.r.Set(f.Defaults+disk::MaleMontage,f.Loaded);f.r.Set(f.Defaults+disk::FemaleMontage,f.Loaded);
        CHECK(ObserveAssets(f.r,f.objects,f.Module,{f.Wave,f.Male,f.Female},f.Loaded,proof,3));
        CHECK(!f.Observe(proof)&&!proof.wave.object);}
    {Fixture f;AssetProof proof;f.r.Set(f.Defaults+disk::MaleMontage,f.Loaded);
        CHECK(ObserveAssets(f.r,f.objects,f.Module,{f.Wave,f.Male,f.Female},f.Loaded,proof,1));
        CHECK(!ObserveAssets(f.r,f.objects,f.Module,{f.Wave,f.Male,f.Female},f.Loaded,proof,3));}
    for(unsigned fault=0;fault<15;++fault){Fixture f;AssetProof proof;
        if(fault==0)f.Name(f.Wave,"OtherWave_C");
        if(fault==1)f.Name(f.Loaded,"DifferentMontage");
        if(fault==2)f.r.Set(f.Defaults+disk::MaleMontage,f.Female);
        if(fault==3)f.r.Set(f.Loaded+disk::Skeleton,f.Emote);
        if(fault==4)f.r.Set(f.Female+disk::Skeleton,f.Emote);
        if(fault==5)f.r.Set(f.Loaded+cfgprobe::ObjectClass,f.Emote);
        if(fault==6)f.r.Set(f.Wave+cfgspawn::ClassDefaultObject,f.Male);
        if(fault==7)f.r.Set(f.Registry+8ULL*24+8,std::uint32_t{0x20000000});
        if(fault==8)f.r.Set(f.Module+disk::AnimMontageConstructor,f.Emote);
        if(fault==9)f.r.cap=32;
        if(fault==10){f.r.changeAt=f.Male+disk::Skeleton;f.r.changeSlot=f.Defaults+disk::MaleMontage;f.r.newPointer=f.Female;}
        if(fault==11){f.r.changeAt=f.Loaded+disk::Skeleton;f.r.changeSlot=f.Module+disk::AnimMontageConstructor;f.r.newPointer=f.Emote;}
        if(fault==12){f.r.changeAt=f.Loaded+disk::Skeleton;f.r.changeSlot=f.Male+cfgprobe::ObjectClass;f.r.newPointer=f.Emote;}
        if(fault==13)f.r.Set(f.Skeleton+cfgprobe::ObjectClass,f.Montage);
        if(fault==14)f.r.Set(f.Wave+cfgprobe::ClassSuper,std::uintptr_t{});
        CHECK(!f.Observe(proof)&&!proof.wave.object&&!proof.loaded.object);peak=(std::max)(peak,f.r.reads);
    }
    std::printf("Actual retained urination asset proof: %u failures, peak%u/1536; named Wave CDO/original male+optional female, exact native montage class and same skeleton, fresh slot/link/class checks; no native/memory writes.\n",failures,peak);
    return failures?1:0;
}
#endif
