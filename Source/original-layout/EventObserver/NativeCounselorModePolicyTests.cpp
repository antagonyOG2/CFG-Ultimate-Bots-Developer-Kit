#include "NativeCounselorModePolicy.hpp"
#include <cstdio>
int main(){using namespace cfgmenu;using namespace launch;unsigned checks=0,failures=0;
    const auto check=[&](bool value){++checks;if(!value)++failures;};
    const auto label=[](cfgnames::Text& text,std::u16string_view value){text={};text.count=text.baseCount=static_cast<unsigned>(value.size());std::copy(value.begin(),value.end(),text.data.begin());};
    const auto alias=[](catalogues::StringValue& text,std::u16string_view value){text={};text.header.count=static_cast<int>(value.size()+1);text.header.capacity=text.header.count;std::copy(value.begin(),value.end(),text.bytes.begin());};
    catalogues::Modes base;base.count=2;base.selectedMap=0x100000;
    base.entries[0].type=0x200000;label(base.entries[0].name,u"ModeDef_Sandbox_C");alias(base.entries[0].alias,u"snbx");base.entries[0].supportsSelectedMap=true;
    base.entries[1].type=0x300000;label(base.entries[1].name,u"ModeDef_OfflineBots_C");alias(base.entries[1].alias,u"oflb");base.entries[1].supportsSelectedMap=true;
    check(ModesForOrigin(Origin::NativeSandbox,base,0x200000));
    check(!ModesForOrigin(Origin::NativeSandbox,base,0x300000));
    check(ModesForOrigin(Origin::AcceptedCounselorPicker,base,0x300000));
    check(ModesForOrigin(Origin::AcceptedCounselorPicker,base,0x200000));
    for(unsigned fault=0;fault<10;++fault){auto changed=base;
        if(fault==0)changed.selectedMap=0;if(fault==1)changed.entries[0].supportsSelectedMap=false;
        if(fault==2)changed.entries[1].supportsSelectedMap=false;if(fault==3)alias(changed.entries[1].alias,u"snbx");
        if(fault==4)label(changed.entries[1].name,u"ModeDef_Online_C");
        if(fault==5){changed.count=3;changed.entries[2]=changed.entries[0];}
        if(fault==6){changed.count=3;changed.entries[2]=changed.entries[1];}
        if(fault==7)changed.count=catalogues::MaximumModes+1;
        if(fault==8)changed.entries[1].type=0x400000;if(fault==9)changed.count=0;
        check(!ModesForOrigin(Origin::AcceptedCounselorPicker,changed,0x300000));}
    check(!ModesForOrigin(static_cast<Origin>(99),base,0x300000));check(!ModesForOrigin(Origin::AcceptedCounselorPicker,base,0));
    std::printf("Dedicated accepted-counselor mode admission: %s (%u checks; default Jason entry refused; no writes/calls).\n",failures?"FAIL":"PASS",checks);return failures?1:0;
}
