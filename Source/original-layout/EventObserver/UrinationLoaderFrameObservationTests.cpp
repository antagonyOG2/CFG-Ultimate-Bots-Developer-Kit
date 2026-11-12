#define main CfgLoaderReflectionSetupMain
#include "ReflectionObservationTests.cpp"
#undef main
#include "UrinationLoaderFrameObservation.hpp"
namespace {
unsigned failures=0;
void Check(bool okay){if(!okay)++failures;}
struct LoaderFixture:Fixture {
    static constexpr std::uintptr_t Defaults=0x600000,Make=0x610000,Load=0x620000,Kind=0x630000,First=0x640000;
    void Property(std::uintptr_t at,std::uintptr_t fn,const char* name,const char* kind,int offset,int bytes,std::uint64_t flags,std::uintptr_t next){
        const auto type=at+0x1000;Name(type,kind);Name(at,name);r.Set(at+cfgprobe::ObjectClass,type);r.Set(at+0x20,fn);
        r.Set(at+cfgreflection::disk::Next,next);struct Shape{int dimension,bytes;std::uint64_t flags;};
        r.Set(at+cfgreflection::disk::PropertyDimensions,Shape{1,bytes,flags});r.Set(at+cfgreflection::disk::PropertyOffset,offset);
    }
    LoaderFixture(bool load){using namespace cfgurination;const auto fn=load?Load:Make;const auto& b=load?disk::LoadAsset:disk::MakeSoftObjectPath;
        Name(Kind,"Function");Name(fn,load?"LoadAsset":"MakeSoftObjectPath");
        r.Set(fn+cfgprobe::ObjectClass,Kind);r.Set(fn+0x20,Owner);r.Set(fn+cfgreflection::disk::Next,std::uintptr_t{});
        r.Set(fn+0x8C,static_cast<std::uint8_t>(load?4:2));r.Set(fn+0x8E,static_cast<std::uint16_t>(load?88:40));
        r.Set(fn+cfgreflection::disk::Children,First);
        r.Set(Base+b.cache,fn);r.Set(Base+b.ownerReflection,Owner);r.Set(Base+b.ownerConstructor,Owner);
        r.Set(Owner+cfgspawn::ClassDefaultObject,Defaults);r.Set(Defaults+cfgprobe::ObjectClass,Owner);r.Set(Base+b.factory,b.code);
        r.Set(fn+0xB0,Base+(load?disk::LoadAssetExecutor:disk::MakePathExecutor));
        if(load){r.Set(Base+disk::LoadAssetExecutor,disk::LoadAssetExecutorCode);
            Property(First,fn,"WorldContextObject","ObjectProperty",0,8,disk::RuntimeLoadWorldFlags,First+0x2000);
            Property(First+0x2000,fn,"Asset","SoftObjectProperty",8,40,disk::RuntimeLoadAssetFlags,First+0x4000);
            Property(First+0x4000,fn,"OnLoaded","DelegateProperty",48,16,disk::RuntimeLoadDelegateFlags,First+0x6000);
            Property(First+0x6000,fn,"LatentInfo","StructProperty",64,24,disk::RuntimeLoadLatentFlags,0);
        }else{r.Set(Base+disk::MakePathExecutor,disk::MakePathExecutorCode);
            Property(First,fn,"PathString","StrProperty",0,16,disk::RuntimePathStringFlags,First+0x2000);
            Property(First+0x2000,fn,"ReturnValue","StructProperty",16,24,disk::RuntimePathResultFlags,0);}
        r.reads=0;
    }
};
}
#ifndef CFG_URINATION_LOADER_FIXTURE_ONLY
int main(){unsigned peak=0;
    for(const bool load:{false,true}){LoaderFixture f(load);std::array<std::uintptr_t,4> roots{};const auto fn=load?f.Load:f.Make;
        Check(cfgurination::LoaderSchema(f.r,f.Base,f.Defaults,fn,load,roots));
        Check((roots==std::array<std::uintptr_t,4>{f.Defaults,fn,f.Owner,f.Defaults}));peak=(std::max)(peak,f.r.reads);
        for(unsigned fault=0;fault<11;++fault){LoaderFixture broken(load);const auto& b=load?cfgurination::disk::LoadAsset:cfgurination::disk::MakeSoftObjectPath;
            if(fault==0)broken.r.Set(broken.Base+b.cache,std::uintptr_t{});
            if(fault==1)broken.r.Set(broken.Defaults+cfgprobe::ObjectClass,broken.Kind);
            if(fault==2)broken.r.Set(fn+0x8E,std::uint16_t{128});
            if(fault==3)broken.r.Set(fn+0xB0,broken.Base+cfgurination::disk::LoadAssetNative);
            if(fault==4)broken.r.Set(broken.Base+b.factory,std::uint8_t{});
            if(fault==5)broken.r.Set(broken.First+cfgreflection::disk::PropertyFlags,std::uint64_t{0x80});
            if(fault==6)broken.r.Set(broken.First+cfgreflection::disk::PropertyOffset,8);
            if(fault==7)broken.r.Set(broken.First+cfgreflection::disk::Next,broken.First);
            if(fault==8)broken.r.Set(broken.Base+(load?cfgurination::disk::LoadAssetExecutor:cfgurination::disk::MakePathExecutor),std::uint8_t{});
            if(fault==9)broken.Name(fn,"OtherFunction");
            if(fault==10)broken.r.Set(broken.Base+b.ownerConstructor,broken.Kind);
            Check(!cfgurination::LoaderSchema(broken.r,broken.Base,broken.Defaults,fn,load,roots)&&roots[0]==0);
        }
    }
    std::printf("Exact retained urination loader schemas: %u failures, peak%u reads; named owner/CDO/cache/native bodies,40/88-byte shape and constructor-derived runtime flags; no factory/dispatch.\n",failures,peak);
    return failures?1:0;
}
#endif
