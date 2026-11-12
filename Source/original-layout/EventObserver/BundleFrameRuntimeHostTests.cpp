#define CFGMENU_OPTIONS_FIXTURE_ONLY
#include "../RuntimeProbe/MenuOptionsScriptTests.cpp"
#undef CFGMENU_OPTIONS_FIXTURE_ONLY
#include "BundleFrameInputObservation.hpp"
int main(){unsigned failures=0;
    const auto check=[&](bool okay){if(!okay)++failures;};
    for(unsigned fault=0;fault<5;++fault){OptionsFixture fixture;auto& r=fixture.fixture.r;
        OptionsReader reader{r};cfgretainframe::RuntimeHost<OptionsReader> host(reader,MenuFixture::Base);
        if(fault==1)r.Set(MenuFixture::Items+cfgreflection::arraydisk::Inner,std::uintptr_t{0});
        if(fault==2)r.Set(MenuFixture::Inner+cfgreflection::disk::PropertyElementSize,8);
        if(fault==3)r.Set(MenuFixture::Inner+cfgreflection::disk::PropertyDimensions,2);
        if(fault==4)r.Set(MenuFixture::Inner+cfgprobe::ObjectClass,MenuFixture::ObjectProperty);
        cfgretainframe::Property property{};
        check(host.PropertyAt(MenuFixture::Items,MenuFixture::Options,property));
        check((property.kind==cfgretainframe::Kind::Value)==(fault==0));
    }
    // SERVER_OnInteract(Interactable, InteractMethod): the enum class : uint8
    // parameter is an EnumProperty. Unsupported, it refused every unlocked-door
    // Press (live PID1696 loop). One 1-byte scalar only; wider/array refuse.
    for(unsigned shape=0;shape<3;++shape){OptionsFixture fixture;auto& f=fixture.fixture;auto& r=f.r;
        OptionsReader reader{r};cfgretainframe::RuntimeHost<OptionsReader> host(reader,MenuFixture::Base);
        constexpr std::uintptr_t Method=0x1F00000,EnumKind=0x1F10000;
        f.Name(EnumKind,"EnumProperty");f.Field(Method,MenuFixture::Options,EnumKind,"InteractMethod",0);
        f.Property(Method,shape==1?4:1,8);if(shape==2)r.Set(Method+cfgreflection::disk::PropertyDimensions,2);
        cfgretainframe::Property property{};
        check(host.PropertyAt(Method,MenuFixture::Options,property));
        check((property.kind==cfgretainframe::Kind::Value)==(shape==0));
    }
    std::printf("Actual frame metadata host: %u failures; pointer-free string array and 1-byte enum only, missing/wrong inner refuses; owned bytes only\n",failures);
    return failures?1:0;
}
