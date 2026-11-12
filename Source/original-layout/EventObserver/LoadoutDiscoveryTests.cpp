#define main RestrictedLoadoutFixtureMain
#include "RestrictedLoadoutTests.cpp"
#undef main
#include "ExpandedLoadoutRuntime.hpp"
#include <algorithm>
#include <cstring>
struct SurveyReader {
    Fixture& f;unsigned reads=0,cap=1536,peak=0,count=20000,target=19000;
    bool registry=false,missing=false;
    static constexpr std::uintptr_t Data=0x70000000,Actors=0x80000000,OtherType=0x500000,Level=0x600000,Levels=0x610000;
    std::uintptr_t Object(unsigned index)const{
        if(!registry)return Actors+0x100*index;
        switch(index){case 0:return f.World;case 1:return f.Pawn;case 2:return f.Mode;
            case 4:return f.Defaults;case 5:return f.Special;case 6:return f.Small;case 8:return f.Item;default:break;}
        return index==target&&!missing?f.Loaded:Actors+0x100*index;
    }
    bool Charge(unsigned n){if(n>cap-reads)return false;reads+=n;return true;}
    void Phase(){peak=(std::max)(peak,reads);reads=320;} // outer guards192 + deeper-ancestry reserve128
    template<class T> bool Read(std::uintptr_t at,T& out){
        if(!Charge(1))return false;
        const unsigned stride=registry?24u:8u;
        if(at>=Data&&at+sizeof(T)<=Data+static_cast<std::uintptr_t>(stride)*count){
            auto* bytes=reinterpret_cast<unsigned char*>(&out);
            for(unsigned i=0;i<sizeof(T);++i){const auto offset=static_cast<unsigned>(at-Data)+i;
                const auto index=offset/stride,within=offset%stride;
                const std::uintptr_t word=within<8?Object(index):within>=16?100u+index:0u;
                bytes[i]=static_cast<unsigned char>((word>>(8*(within%8)))&255u);}
            return true;
        }
        if(at>=Actors&&at<Actors+0x100ull*count&&(at-Actors)%0x100==cfgprobe::ObjectClass&&sizeof(T)==8){
            const auto index=static_cast<unsigned>((at-Actors)/0x100);
            const std::uintptr_t type=!registry&&!missing&&index==target?f.Loaded:OtherType;
            std::memcpy(&out,&type,8);return true;
        }
        return f.memory.Read(at,out);
    }
    void Initialize(){
        f.Name(0,"CounselorTwoHandedAxe_C");f.Name(3,"StaticMeshActor");
        struct Counts{int entries,chunks;};f.memory.Set(f.Names+cfgnames::disk::ElementCount,Counts{4,1});
        f.memory.Set(OtherType+cfgnames::disk::ObjectName,cfgnames::Words{3,0});
        f.memory.Set(OtherType+cfgprobe::ClassSuper,std::uintptr_t{});
        f.memory.Set(f.Item+cfgprobe::ObjectClass,std::uintptr_t{0x390000});
        f.memory.Set(f.Loaded+cfgprobe::ObjectClass,std::uintptr_t{0x390000});
        f.memory.Set(0x390000+cfgprobe::ClassSuper,std::uintptr_t{});
        f.Slot(8,f.Item);f.memory.Set(f.Module+cfgprobe::lifetime::ObjectArrayRva+0x1C,int{9});
        f.memory.Set(f.Pawn+cfgloadout::restricteddisk::EquippedItem,std::uintptr_t{});
        f.memory.Set(f.Defaults+cfgloadout::restricteddisk::IsLarge,std::uint8_t{1});
        f.memory.Set(f.World+cfgprobe::ObjectClass,f.Counselor);
        if(registry){f.memory.Set(f.World+cfgradarworld::disk::WorldLevels,cfgspawn::PlayerArray{});
            f.memory.Set(f.Module+cfgprobe::lifetime::ObjectArrayRva+0x10,Data);
            f.memory.Set(f.Module+cfgprobe::lifetime::ObjectArrayRva+0x1C,static_cast<int>(count));
            f.memory.Set(f.Loaded+cfgprobe::lifetime::ObjectIndexOffset,static_cast<int>(target));}
        else {f.memory.Set(f.World+cfgradarworld::disk::WorldLevels,cfgspawn::PlayerArray{Levels,1,1});
            f.memory.Set(Levels,Level);f.memory.Set(Level+cfgkill::disk::OwningWorld,f.World);
            f.memory.Set(Level+cfgradarworld::disk::LevelActors,cfgspawn::PlayerArray{Data,static_cast<int>(count),static_cast<int>(count)});}
    }
};
struct SurveyHost {SurveyReader& r;unsigned calls=0;
    bool Current(const cfgofflinesettings::Request&){return r.Charge(128);}
    bool InvokeGrant(std::uintptr_t,std::uintptr_t,std::uintptr_t){++calls;return true;}
};
int main(){unsigned failures=0;const auto check=[&](bool okay,const char* label){if(!okay){++failures;std::printf("FAIL %s\n",label);}};
    using namespace cfgloadout;
    for(bool registry:{false,true}){Fixture f;SurveyReader reader{f};reader.registry=registry;
        if(registry){reader.count=175377;reader.target=150000;}reader.Initialize();SurveyHost host{reader};expanded::Runtime runtime;
        check(runtime.Begin(f.request.owner,{3,0},0),"Axe request begins");unsigned foundAt=0;
        // Two eligible idle lanes/sec for the actor route; four/sec for fallback.
        const unsigned gap=registry?250u:500u;
        for(unsigned now=3000;now<90000&&runtime.Active()&&!host.calls;now+=gap){reader.Phase();runtime.Step(reader,host,f.Module,now);
            if(!foundAt&&runtime.report.type)foundAt=now;}
        reader.Phase();check(host.calls==1,"late Axe class reaches native grant under deadline");
        check(reader.peak<1536,"survey and grant share production cap");
        check(runtime.report.nativeStage==0&&runtime.report.grantAttempts==1,"grant diagnostic identifies real attempt");
        if(!registry)check(foundAt<30000,"world items avoid global class catalogue tail");
        std::printf("Loadout survey %s: class=%u foundAt=%u slices=%u visited=%u peak=%u/1536 calls=%u\n",
            registry?"registry":"world actors",reader.target,foundAt,runtime.report.slices,runtime.report.visited,reader.peak,host.calls);
    }
    {Fixture f;SurveyReader reader{f};reader.count=300;reader.target=299;reader.missing=true;reader.Initialize();
        expanded::ClassSurvey survey;for(unsigned n=0;n<10&&survey.phase==0;++n){reader.Phase();survey.Step(reader,f.Module,f.World,Slot::Weapon,3);}
        check(survey.phase==1,"missing world item advances to global fallback");}
    std::printf("LoadoutDiscoveryTests: %u failures\n",failures);return failures?1:0;
}
