#define main CfgReflectionFixtureMain
#include "ReflectionObservationTests.cpp"
#undef main
#include "SoftClassFrameObservation.hpp"
#include <cstdio>
namespace {
unsigned failures=0;
void Check(bool v){if(!v)++failures;}
struct SoftFixture:Fixture {
    static constexpr std::uintptr_t Convert=0x600000,Load=0x610000,Default=0x620000,FunctionType=0x630000,
        World=0x640000,Controller=0x650000,First=0x660000,Second=0x670000;
    cfgprimary::SoftClassBytes path{};
    void Property(std::uintptr_t p,std::uintptr_t fn,const char* name,const char* kind,int offset,int size,std::uint64_t flags,std::uintptr_t next){
        const auto type=p+0x1000;Name(type,kind);Name(p,name);
        r.Set(p+cfgprobe::ObjectClass,type);r.Set(p+0x20,fn);r.Set(p+cfgreflection::disk::Next,next);
        struct Shape{int dimension,bytes;std::uint64_t flags;};r.Set(p+cfgreflection::disk::PropertyDimensions,Shape{1,size,flags});
        r.Set(p+cfgreflection::disk::PropertyOffset,offset);
    }
    void Function(std::uintptr_t fn,const char* name,std::uint8_t count,std::uint16_t bytes,std::uintptr_t head){
        Name(fn,name);r.Set(fn+cfgprobe::ObjectClass,FunctionType);r.Set(fn+0x20,Owner);r.Set(fn+cfgreflection::disk::Next,std::uintptr_t{});
        r.Set(fn+0x8C,count);r.Set(fn+0x8E,bytes);r.Set(fn+cfgreflection::disk::Children,head);
    }
    SoftFixture(){
        Name(FunctionType,"Function");
        r.Set(Base+cfgprimary::disk::KismetReflection,Owner);r.Set(Base+cfgprimary::disk::KismetConstructor,Owner);
        r.Set(Owner+cfgspawn::ClassDefaultObject,Default);r.Set(Default+cfgprobe::ObjectClass,Owner);
        r.Set(Base+cfgprimary::disk::Conversion.cache,Convert);r.Set(Base+cfgcounselorload::disk::LoadClass.cache,Load);
        Function(Convert,"Conv_SoftClassReferenceToClass",2,48,First);
        Property(First,Convert,"SoftClass","SoftClassProperty",0,40,0x001C000008000182ULL,First+0x2000);
        Property(First+0x2000,Convert,"ReturnValue","ClassProperty",40,8,0x001C001040000780ULL,0);
        Function(Load,"LoadAssetClass",4,88,Second);
        Property(Second,Load,"WorldContextObject","ObjectProperty",0,8,0x0018001040000280ULL,Second+0x2000);
        Property(Second+0x2000,Load,"AssetClass","SoftClassProperty",8,40,0x001C000000000080ULL,Second+0x4000);
        Property(Second+0x4000,Load,"OnLoaded","DelegateProperty",48,16,0x0010001000000280ULL,Second+0x6000);
        Property(Second+0x6000,Load,"LatentInfo","StructProperty",64,24,0x0010001000000080ULL,0);
        path[16]=1; // valid nonempty native FName path, empty FString subpath
    }
    bool Observe(std::uintptr_t fn,void* packet,cfgsoftframe::Result& out,std::uintptr_t actor=Default){
        r.reads=0;return cfgsoftframe::Observe(r,Base,actor,fn,packet,Default,Convert,World,Controller,out);
    }
};
struct Reader {
    inline static SoftFixture* fixture=nullptr;unsigned cap=4096,reads=0;
    template<class T>bool Read(std::uintptr_t at,T& out){if(!fixture||reads>=cap)return false;++reads;return fixture->r.Read(at,out);}
};
struct {std::uintptr_t module=Fixture::Base,world=SoftFixture::World,controller=SoftFixture::Controller;}plan;
struct {std::uintptr_t kismetDefault=SoftFixture::Default,conversionFunction=SoftFixture::Convert;}spawnAssets;
std::array<std::uintptr_t,6> capturedSoftRoots{};unsigned rootCalls=0,completions=0,lastMaximum=0;
std::uintptr_t capturedSoftOutput=0;bool completionOkay=true;
template<std::size_t N>bool RetainPrimaryObjects(Reader&,const std::array<std::uintptr_t,N>& values){++rootCalls;capturedSoftRoots={};
    for(unsigned i=0;i<N&&i<capturedSoftRoots.size();++i)capturedSoftRoots[i]=values[i];return true;}
template<std::size_t N>bool CompleteBundleCallbackCall(Reader&,const std::array<std::uintptr_t,N>& values,unsigned maximum){
    ++completions;lastMaximum=maximum;capturedSoftOutput=N?values[0]:0;return completionOkay;}
struct ActualMembers {
    Reader r_;unsigned bundleNextOutputMaximum_=0;
    bool FinishBundleNative(){++completions;lastMaximum=0;return completionOkay;}
#include "SoftClassRetentionRuntimeMembers.hpp"
};
}
int main(){
    SoftFixture f;Reader::fixture=&f;cfgsoftframe::Result result;
    cfgprimary::ConversionParameters conversion;Check(cfgprimary::BuildConversion(f.path,conversion));
    Check(f.Observe(f.Convert,&conversion,result)&&result.kind==cfgsoftframe::Kind::Conversion&&result.outputs==1);
    Check(result.roots[0]==f.Default&&result.roots[1]==f.Convert&&result.roots[2]==f.Owner);
    const auto conversionReads=f.r.reads;
    cfgcounselorload::Packet preload;Check(cfgcounselorload::Build(f.World,f.Controller,5,f.path,preload));
    Check(f.Observe(f.Load,&preload,result)&&result.kind==cfgsoftframe::Kind::Preload&&!result.outputs&&result.roots[4]==f.World&&result.roots[5]==f.Controller);
    const auto preloadReads=f.r.reads;
    std::array<std::uintptr_t,32> oldRoots{};cfgretainframe::RuntimeHost<OwnedByteReader> generic(f.r,f.Base);
    f.r.Set(0x700000,preload);
    Check(!cfgretainframe::Observe(generic,f.Load,0x700000,oldRoots)); // old adapter refuses SoftClass/delegate/latent input
    f.r.Set(0x700000,conversion);
    Check(!cfgretainframe::Observe(generic,f.Convert,0x700000,oldRoots)); // same confirmed conversion gap
    for(unsigned fault=0;fault<11;++fault){auto bad=preload;
        if(fault==0)bad.world=0;if(fault==1)bad.latent.target=0;if(fault==2)bad.latent.linkage=0;
        if(fault==3)bad.latent.function=1;if(fault==4)bad.latent.uuid=0;if(fault==5)bad.onLoaded.index=1;
        if(fault==6)bad.onLoaded.serial=1;if(fault==7)bad.onLoaded.function=1;if(fault==8)bad.source[24]=1;
        if(fault==9)bad.source={};
        Check(!f.Observe(f.Load,&bad,result,fault==10?f.Controller:f.Default));
    }
    auto invalid=conversion;invalid.result=0x888000;Check(!f.Observe(f.Convert,&invalid,result));
    invalid=conversion;invalid.softClass[24]=1;Check(!f.Observe(f.Convert,&invalid,result));
    Check(f.Observe(0x990000,&conversion,result)&&result.kind==cfgsoftframe::Kind::None);
    Check(!f.Observe(f.Load,nullptr,result));
    for(unsigned fault=0;fault<5;++fault){SoftFixture bad;
        if(fault==0)bad.r.Set(bad.Load+0x8E,std::uint16_t{87});
        if(fault==1)bad.r.Set(bad.Second+0x6000+cfgreflection::disk::PropertyOffset,int{63});
        if(fault==2)bad.r.Set(bad.Second+0x4000+cfgreflection::disk::PropertyFlags,std::uint64_t{0x580});
        if(fault==3)bad.r.Set(bad.Base+cfgprimary::disk::KismetConstructor,std::uintptr_t{0x999000});
        if(fault==4)bad.r.Set(bad.Second+0x6000+cfgreflection::disk::Next,bad.Second);
        Check(!bad.Observe(bad.Load,&preload,result));
    }
    ActualMembers actual;bool matched=false;
    Check(actual.RetainTypedSoftClassFrame(f.Default,f.Convert,&conversion,matched)&&matched&&rootCalls==1&&actual.bundleNextOutputMaximum_==1);
    conversion.result=0x777000;
    Check(actual.CompleteTypedSoftClassFrame(&conversion,matched)&&matched&&completions==1&&capturedSoftOutput==0x777000&&lastMaximum==1);
    Check(actual.CompleteTypedSoftClassFrame(&conversion,matched)&&!matched&&completions==1); // result cannot be published twice
    Check(actual.RetainTypedSoftClassFrame(f.Default,f.Load,&preload,matched)&&matched&&rootCalls==2&&actual.bundleNextOutputMaximum_==0);
    Check(actual.CompleteTypedSoftClassFrame(&preload,matched)&&matched&&completions==2&&lastMaximum==0);
    conversion.result=0;Check(actual.RetainTypedSoftClassFrame(f.Default,f.Convert,&conversion,matched)&&matched);
    completionOkay=false;conversion.result=0x778000;
    Check(!actual.CompleteTypedSoftClassFrame(&conversion,matched)&&matched&&completions==3);
    Check(actual.CompleteTypedSoftClassFrame(&conversion,matched)&&!matched&&completions==3);
    // Preserve the precise refused metadata without changing admission.
    {SoftFixture bad;cfgsoftframe::Diagnostic d;
     bad.r.Set(bad.Load+0x8C,std::uint8_t{3});
     Check(!cfgsoftframe::Observe(bad.r,bad.Base,bad.Default,bad.Load,&preload,bad.Default,bad.Convert,bad.World,bad.Controller,result,&d));
     Check(d.stage==24&&d.count==3&&d.bytes==88);}
    {SoftFixture bad;cfgsoftframe::Diagnostic d;
     bad.r.Set(bad.Second+0x2000+cfgreflection::disk::PropertyElementSize,int{39});
     Check(!cfgsoftframe::Observe(bad.r,bad.Base,bad.Default,bad.Load,&preload,bad.Default,bad.Convert,bad.World,bad.Controller,result,&d));
     Check(d.stage==35&&d.property==bad.Second+0x2000&&d.elementBytes==39&&d.offset==8&&d.mask==1&&d.index==1&&d.name.View()==u"AssetClass"&&d.type.View()==u"SoftClassProperty");}
    {SoftFixture bad;cfgsoftframe::Diagnostic d;
     bad.r.Set(bad.Second+0x4000+cfgreflection::disk::PropertyFlags,std::uint64_t{0x580});
     Check(!cfgsoftframe::Observe(bad.r,bad.Base,bad.Default,bad.Load,&preload,bad.Default,bad.Convert,bad.World,bad.Controller,result,&d));
     Check(d.stage==35&&d.flags==0x580&&d.name.View()==u"OnLoaded"&&d.mask==3);}
    {cfgsoftframe::Diagnostic d;
     Check(cfgsoftframe::Observe(f.r,f.Base,f.Default,f.Load,&preload,f.Default,f.Convert,f.World,f.Controller,result,&d)&&d.stage==0&&d.mask==15);}
    // Raw generated descriptor flags cannot stand in for native constructed
    // UProperty flags. Check every parameter against independently verified literals.
    const std::array<std::uintptr_t,6> properties{f.First,f.First+0x2000,f.Second,f.Second+0x2000,f.Second+0x4000,f.Second+0x6000};
    const std::array<std::uint64_t,6> descriptorFlags{0x0014000008000182ULL,0x0014000000000580ULL,0x0010000000000080ULL,0x0014000000000080ULL,0x0010000000000080ULL,0x0010000000000080ULL};
    for(unsigned i=0;i<properties.size();++i){SoftFixture bad;cfgsoftframe::Diagnostic d;
        bad.r.Set(properties[i]+cfgreflection::disk::PropertyFlags,descriptorFlags[i]);
        cfgprimary::ConversionParameters input;Check(cfgprimary::BuildConversion(bad.path,input));
        Check(!cfgsoftframe::Observe(bad.r,bad.Base,bad.Default,i<2?bad.Convert:bad.Load,i<2?static_cast<void*>(&input):static_cast<void*>(&preload),bad.Default,bad.Convert,bad.World,bad.Controller,result,&d));
        Check(d.stage==35&&d.property==properties[i]&&d.flags==descriptorFlags[i]);
    }
    std::printf("Exact named soft-class frame + actual retention members:%u failures; conversion%u/preload%u reads,owned stub capturedSoftRoots/completion,no game/native calls.\n",failures,conversionReads,preloadReads);
    return failures?1:0;
}
