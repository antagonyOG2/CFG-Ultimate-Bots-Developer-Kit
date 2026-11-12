#define CFGMENU_NATIVE_FIXTURE_ONLY
#include "NativeMenuAdapterTests.cpp"
#undef CFGMENU_NATIVE_FIXTURE_ONLY
#include "../RuntimeProbe/SettingsStartObservation.hpp"
#include <memory>
struct StartBindingFixture : NativeFixture {
    static constexpr std::uintptr_t Start=0x1900000,Construct=0x1910000,Graph=0x1920000,Entry=0x1930000,
        StartCode=0x1940000,ConstructCode=0x1950000,GraphCode=0x1960000;
    StartBindingFixture(){auto& f=fixture;auto& r=f.r;using namespace cfgmenu;
        f.Name(MenuFixture::Type,"OfflineBotsSettingsMenuWidget_C");
        r.Set(MenuFixture::Type+cfgreflection::disk::Children,Start);
        const auto wrapper=[&](std::uintptr_t fn,const char* name,std::uintptr_t next,std::uintptr_t code,std::uint32_t flags,std::int32_t entry){
            f.Field(fn,MenuFixture::Type,MenuFixture::FunctionClass,name,next);Register(fn);
            r.Set(fn+cfgreflection::disk::Children,std::uintptr_t{0});r.Set(fn+cfgreflection::disk::FunctionFlags,flags);
            r.Set(fn+cfgreflection::disk::StorageBytes,0);r.Set(fn+cfgreflection::arraydisk::FunctionParameterBytes,std::uint16_t{0});
            r.Set(fn+0x68,std::uintptr_t{0});r.Set(fn+startdisk::Executor,MenuFixture::Base+startdisk::ScriptExecutor);
            r.Set(fn+startdisk::FastGraph,Graph);r.Set(fn+startdisk::FastEntry,entry);
            r.Set(fn+scriptdisk::Script,capture::ScriptHeader{code,22,22});
            std::array<unsigned char,22> bytes{};bytes[0]=0x90;for(unsigned i=1;i<=12;++i)bytes[i]=static_cast<unsigned char>(i);
            bytes[13]=0x5A;std::memcpy(bytes.data()+14,&entry,4);bytes[18]=0x97;bytes[19]=0xA5;bytes[20]=0x39;bytes[21]=0xE5;r.Set(code,bytes);
        };
        wrapper(Start,"OnClicked_Start",Construct,StartCode,0x0C000000,startdisk::StartEntry);
        wrapper(Construct,"Construct",Graph,ConstructCode,0x08020808,startdisk::ConstructEntry);
        f.Field(Graph,MenuFixture::Type,MenuFixture::FunctionClass,"ExecuteUbergraph_OfflineBotsSettingsMenuWidget",0);Register(Graph);
        r.Set(Graph+cfgreflection::disk::Children,Entry);r.Set(Graph+cfgreflection::disk::FunctionFlags,std::uint32_t{0});
        r.Set(Graph+cfgreflection::disk::StorageBytes,startbinding::shape::StorageBytes);r.Set(Graph+cfgreflection::arraydisk::FunctionParameterBytes,std::uint16_t{4});
        r.Set(Graph+0x68,Entry);r.Set(Graph+startdisk::Executor,MenuFixture::Base+startdisk::ScriptExecutor);
        r.Set(Graph+startdisk::FastGraph,std::uintptr_t{0});r.Set(Graph+startdisk::FastEntry,std::int32_t{123}); // unused when A0 null
        r.Set(Graph+scriptdisk::Script,capture::ScriptHeader{GraphCode,startbinding::shape::GraphBytes,startbinding::shape::GraphBytes});
        std::array<unsigned char,startbinding::shape::GraphBytes> graph{};
        for(const auto entry:{startdisk::StartEntry,startdisk::ConstructEntry}){graph[entry]=0x0A;
            const std::uint32_t destination=entry==startdisk::StartEntry?startbinding::shape::StartDestination:startbinding::shape::ConstructDestination;std::memcpy(graph.data()+entry+1,&destination,4);}
        r.Set(GraphCode,graph);
        f.Field(Entry,Graph,MenuFixture::IntProperty,"EntryPoint",0);f.Property(Entry,4,0);
        r.Set(Entry+cfgreflection::disk::PropertyFlags,std::uint64_t{0x0008001040000294ULL});
    }
};
#ifndef CFGMENU_START_BINDING_FIXTURE_ONLY
int main(){unsigned failures=0;const auto check=[&](bool okay,const char* label){if(!okay){++failures;std::printf("FAIL %s\n",label);}};
    using namespace cfgmenu;using F=StartBindingFixture;
    for(unsigned fault=0;fault<13;++fault){auto fixture=std::make_unique<F>();OptionsReader reader{fixture->fixture.r};active::Context context;
        check(active::Observe(reader,MenuFixture::Base,context),"active owner");
        auto scratch=std::make_unique<startbinding::Workspace>();startbinding::Binding binding;unsigned reads=0;
        check(startbinding::Observe(reader,context,true,*scratch,binding,&reads),"cold concrete owned Start binding");
        if(!fault)std::printf("Start preparation reads=%u\n",reads);
        auto& r=fixture->fixture.r;
        if(fault==1)r.Set(F::Start+startdisk::FastEntry,std::int32_t{2506});
        if(fault==2)r.Set(F::GraphCode+500,std::uint8_t{1});
        if(fault==3)r.Set(F::Graph+startdisk::Executor,MenuFixture::Base+startdisk::ScriptExecutor+8);
        if(fault==4)r.Set(F::Entry+cfgreflection::disk::PropertyOffset,4);
        if(fault==5)r.Set(F::Start+0x68,F::Entry);
        if(fault==6)r.Set(F::Construct+startdisk::FastGraph,F::Start);
        if(fault==7)r.Set(MenuFixture::Widget+cfgprobe::ObjectClass,F::Graph);
        if(fault==8)r.Set(F::Graph+cfgreflection::disk::StorageBytes,938);
        if(fault==9)r.Set(F::Registry+24*2+16,999);
        if(fault==10)r.Set(F::Start+scriptdisk::Script,capture::ScriptHeader{F::StartCode,22,23});
        if(fault==12)r.Set(F::GraphCode+startbinding::shape::GraphBytes-1,std::uint8_t{1}); // Added 3.5 tail must also be revalidated.
        std::uintptr_t wrapperExecutor=MenuFixture::Base+startdisk::ScriptExecutor,graphExecutor=wrapperExecutor;
        if(fault==11){wrapperExecutor=0x1970000;graphExecutor=0x1980000;
            r.Set(F::Start+startdisk::Executor,wrapperExecutor);r.Set(F::Graph+startdisk::Executor,graphExecutor);}
        check(startbinding::Current(reader,binding,42,wrapperExecutor,graphExecutor,&reads)==(fault==0||fault==11),
            "exact metadata/script/alias/owner revalidation; only supplied owned executor replacements accepted");
        if(!fault)std::printf("Start current reads=%u (only on Start, not each HUD draw)\n",reads);
    }
    {F fixture;OptionsReader reader{fixture.fixture.r};active::Context context;active::Observe(reader,MenuFixture::Base,context);
        auto scratch=std::make_unique<startbinding::Workspace>();startbinding::Binding out;unsigned reads=99;
        check(!startbinding::Observe(reader,context,false,*scratch,out,&reads)&&!reads&&!out.context.widget,"unsupported build performs no cold reads");}
    std::printf("CFG concrete settings Start binding/metadata-revalidation tests: %s (%u failures). No game access.\n",failures?"FAIL":"PASS",failures);
    return failures?1:0;
}
#endif
