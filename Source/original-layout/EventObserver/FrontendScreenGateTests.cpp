#define CFGMENU_START_BINDING_FIXTURE_ONLY
#include "SettingsStartBindingTests.cpp"
#include "FrontendScreenGate.hpp"
int main(){unsigned failures=0;const auto check=[&](bool okay,const char* label){if(!okay){++failures;std::printf("FAIL %s\n",label);}};
    for(unsigned fault=0;fault<4;++fault){auto f=std::make_unique<StartBindingFixture>();OptionsReader r{f->fixture.r};cfgmenu::active::Context c;
        check(cfgmenu::active::Observe(r,MenuFixture::Base,c),"current HUD on any menu is possible");
        f->fixture.Name(c.type,fault==1?"SandboxMapMenuWidget_C":fault==2?"OfflineBotsMapMenuWidget_C":fault==3?"OfflinePlayMenuWidget_C":"OfflineBotsSettingsMenuWidget_C");
        check(cfgmenu::screengate::Settings(r,c)==(fault==0),"only real settings type accepted; map list never becomes settings proof");
        check(cfgmenu::screengate::Entry(r,c)==(fault==3),"first Offline Play entry accepted; stock settings and map list refused");
    }
    std::printf("CFG frontend screen preflight: %s (%u failures). Owned fixtures only; no game access.\n",failures?"FAIL":"PASS",failures);return failures?1:0;
}
