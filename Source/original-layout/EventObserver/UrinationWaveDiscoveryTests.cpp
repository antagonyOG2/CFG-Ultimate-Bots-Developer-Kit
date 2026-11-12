#define CFG_URINATION_ASSET_FIXTURE_ONLY
#include "UrinationAssetObservationTests.cpp"
#include "UrinationWaveDiscovery.hpp"
#include "../RuntimeProbe/EmoteMirrorBindings.generated.hpp"
namespace {
struct DiscoveryFixture:Fixture {
    static constexpr std::uintptr_t Pawn=0x700000,Counselor=0x710000,Entries=0x720000;
    DiscoveryFixture(){objects.count=11;Slot(Pawn,9,Counselor);Slot(Counselor,10,0);
        r.Set(Counselor+cfgprobe::ClassSuper,std::uintptr_t{});r.Set(Registry+9*24+16,std::int32_t{12});
        r.Set(Pawn+cfgemotemirror::disk::DefaultEmotes,cfgspawn::PlayerArray{Entries,1,1});r.Set(Entries,Wave);r.reads=0;r.cap=4096;}
    bool Discover(std::array<std::uintptr_t,3>& result){return cfgurination::DiscoverWave(r,objects,Module,Pawn,Counselor,cfgemotemirror::disk::DefaultEmotes,result);}
};
}
int main(){using namespace cfgurination;unsigned peak=0;
    {DiscoveryFixture f;std::array<std::uintptr_t,3> result{};CHECK(f.Discover(result));CHECK((result==std::array<std::uintptr_t,3>{f.Wave,f.Male,f.Female}));peak=f.r.reads;}
    {DiscoveryFixture f;std::array<std::uintptr_t,3> result{};f.r.Set(f.Female+disk::Skeleton,f.Emote);
        CHECK(f.Discover(result)&&result[0]==f.Wave&&result[1]==f.Male&&!result[2]);}
    for(unsigned fault=0;fault<8;++fault){DiscoveryFixture f;std::array<std::uintptr_t,3> result{};
        if(fault==0)f.r.Set(f.Pawn+cfgemotemirror::disk::DefaultEmotes,cfgspawn::PlayerArray{f.Entries,65,65});
        if(fault==1)f.r.Set(f.Registry+9*24+16,std::int32_t{});
        if(fault==2)f.r.Set(f.Entries,f.Male);
        if(fault==3)f.r.Set(f.Wave+cfgprobe::ClassSuper,std::uintptr_t{});
        if(fault==4)f.Name(f.Wave,"OtherEmote_C");
        if(fault==5){f.r.changeAt=f.Male+disk::Skeleton;f.r.changeSlot=f.Entries;f.r.newPointer=f.Male;}
        if(fault==6)f.r.Set(f.Pawn+cfgprobe::ObjectClass,f.Emote);
        if(fault==7)f.r.cap=16;
        CHECK(!f.Discover(result)&&!result[0]);peak=(std::max)(peak,f.r.reads);
    }
    std::printf("Actual native Wave catalogue discovery: %u failures, peak%u/4096; positive live counselor and bounded native catalogue, exact named current Wave dependency proof/resampling, no object scans or native calls.\n",failures,peak);
    return failures?1:0;
}
