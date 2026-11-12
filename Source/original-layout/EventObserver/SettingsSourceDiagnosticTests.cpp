#define CFGMENU_START_BINDING_FIXTURE_ONLY
#include "SettingsStartBindingTests.cpp"
#undef CFGMENU_START_BINDING_FIXTURE_ONLY
#include "../RuntimeProbe/SettingsSourceDiagnostic.hpp"
int main(){using namespace cfgmenu;unsigned failures=0;
    const auto check=[&](bool okay,const char* label){if(!okay){++failures;std::printf("FAIL %s\n",label);}};
    for(unsigned fault=0;fault<7;++fault){auto fixture=std::make_unique<StartBindingFixture>();OptionsReader reader{fixture->fixture.r};
        active::Context actual;check(active::Observe(reader,MenuFixture::Base,actual),"actual positive owner");
        auto& r=fixture->fixture.r;
        if(fault==1)r.Set(StartBindingFixture::Start+cfgprobe::ObjectClass,MenuFixture::ObjectProperty);
        if(fault==2)r.Set(StartBindingFixture::Construct+scriptdisk::Script,capture::ScriptHeader{StartBindingFixture::ConstructCode,65537,65537});
        if(fault==3)r.Set(StartBindingFixture::Graph+cfgreflection::disk::StorageBytes,938);
        if(fault==4)r.Set(StartBindingFixture::Graph+cfgreflection::disk::FunctionFlags,std::uint32_t{1});
        if(fault==5)r.Set(StartBindingFixture::Graph+startdisk::FastGraph,std::uintptr_t{1});
        if(fault==6)r.Set(NativeFixture::Registry+24*2+16,999);
        auto snapshot=std::make_unique<startdiagnostic::Snapshot>();
        const bool captured=startdiagnostic::ObserveClass(reader,actual,actual.type,*snapshot);
        check(captured==(fault==0||fault==3||fault==4),"diagnostic captures changed shape, refuses invalid reads/ownership");
        if(!fault){check(snapshot->completed==3&&!snapshot->stage&&snapshot->functions[2].script.count==startbinding::shape::GraphBytes,
            "all three function bodies and locals retained in one bounded snapshot");
            startbinding::Binding binding;auto strict=std::make_unique<startbinding::Workspace>();
            check(startbinding::Observe(reader,actual,true,*strict,binding),"unchanged recorded shape remains independently valid");}
        if(fault==1)check(snapshot->observed==6&&snapshot->completed==2,"failed Start metadata still captures Construct and graph in same pass");
        if(fault==2)check(snapshot->observed==5&&snapshot->completed==2,"oversized Construct still captures Start and graph in same pass");
        if(fault==3||fault==4){startbinding::Binding binding;auto strict=std::make_unique<startbinding::Workspace>();
            check(!startbinding::Observe(reader,actual,true,*strict,binding)&&!binding.context.widget,
                "successful diagnostic is never approval of an incompatible binding");}
    }
    std::printf("Read-only named settings source snapshot: %u failures; owned memory only, no game calls.\n",failures);return failures?1:0;
}
