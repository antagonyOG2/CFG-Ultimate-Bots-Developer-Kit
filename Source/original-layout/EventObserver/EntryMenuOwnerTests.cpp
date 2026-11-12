#define CFGMENU_NATIVE_FIXTURE_ONLY
#include "NativeMenuAdapterTests.cpp"
#undef CFGMENU_NATIVE_FIXTURE_ONLY
#include "../RuntimeProbe/OfflineEntrySettingsObservation.hpp"
static_assert(cfgmenu::pushdisk::MenuClass==cfgmenu::challengecalls::PushMenu.owner);
static_assert(cfgmenu::pushdisk::MenuConstructor==cfgmenu::challengecalls::PushMenu.ctor);
static_assert(cfgmenu::pushdisk::MenuClass!=0x304CFF0,"CFG3.4 copied cache is the wrong owner in3.5");
int main(){unsigned failures=0;constexpr std::uintptr_t NativeMenu=0x1800000,NativeHud=0x1810000;
    for(unsigned fault=0;fault<6;++fault){NativeFixture f;auto& r=f.fixture.r;
        f.fixture.Name(MenuFixture::Type,"OfflinePlayMenuWidget_C");
        f.Root(NativeMenu);f.Root(NativeHud);
        r.Set(MenuFixture::Type+cfgprobe::ClassSuper,NativeMenu);
        r.Set(MenuFixture::Base+cfgmenu::pushdisk::MenuClass,NativeMenu);
        r.Set(MenuFixture::Base+cfgmenu::pushdisk::MenuConstructor,NativeMenu);
        // Model current3.5 SCHUD cache at the old3.4 SCMenuWidget location.
        r.Set(MenuFixture::Base+0x304CFF0,NativeHud);r.Set(MenuFixture::Base+0x304CFF8,NativeHud);
        if(fault==1)r.Set(MenuFixture::Base+cfgmenu::pushdisk::MenuConstructor,NativeHud);
        if(fault==2)r.Set(MenuFixture::Type+cfgprobe::ClassSuper,NativeHud);
        if(fault==3)r.Set(MenuFixture::Base+cfgmenu::selectiondisk::SCGame_MenuConstructor,NativeHud);
        if(fault==4)r.Set(NativeFixture::Mode+cfgprobe::ObjectClass,NativeHud);
        if(fault==5)f.fixture.Name(MenuFixture::Type,"OtherMenuWidget_C");
        OptionsReader reader{r};cfgmenu::active::Context context;
        if(!cfgmenu::active::Observe(reader,MenuFixture::Base,context)){++failures;continue;}
        auto result=std::make_unique<cfgmenu::entrysettings::Resources>();unsigned stage=0,reads=0;
        const bool okay=cfgmenu::entrysettings::Observe(reader,context,context.thread,true,*result,&reads,&stage);
        // Missing transition metadata intentionally stops after successful
        // owner checks. This fixture does not invent authored graph evidence.
        const unsigned expected[]={23,102,103,104,105,101};
        if(okay||stage!=expected[fault]||result->settings){
            std::printf("FAIL owner case%u stage%u expected%u\n",fault,stage,expected[fault]);++failures;
        }
    }
    std::printf("Existing authored-entry owner binding: %u failures; current SCMenuWidget accepted, stale HUD/subclass/cache pairs rejected; owned bytes only\n",failures);
    return failures?1:0;
}
