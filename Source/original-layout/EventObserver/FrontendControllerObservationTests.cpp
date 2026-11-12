#include "../RuntimeProbe/FrontendControllerObservation.hpp"
#include "OwnedByteReader.hpp"
#include <cstdio>
struct MenuControllerFixture {
    static constexpr std::uintptr_t Module=0x100000,Object=0x200000,Blueprint=0x210000,
        Menu=0x220000,Common=0x230000,Gameplay=0x240000;
    OwnedByteReader r;
    MenuControllerFixture(){
        r.Set(Module+cfgmenu::controllergate::MenuClass,Menu);
        r.Set(Module+cfgmenu::controllergate::MenuConstructor,Menu);
        r.Set(Object+cfgprobe::ObjectClass,Blueprint);
        r.Set(Blueprint+cfgprobe::ClassSuper,Menu);
        r.Set(Menu+cfgprobe::ClassSuper,Common);
        r.Set(Gameplay+cfgprobe::ClassSuper,Common);
        r.Set(Common+cfgprobe::ClassSuper,std::uintptr_t{0});
    }
};
int main(){unsigned failures=0;using F=MenuControllerFixture;
    for(unsigned fault=0;fault<9;++fault){F f;
        if(fault==1)f.r.Set(F::Module+cfgmenu::controllergate::MenuClass,std::uintptr_t{0});
        if(fault==2)f.r.Set(F::Module+cfgmenu::controllergate::MenuConstructor,F::Gameplay);
        if(fault==3)f.r.Set(F::Blueprint+cfgprobe::ClassSuper,F::Gameplay);
        if(fault==4)f.r.Set(F::Object+cfgprobe::ObjectClass,F::Common);
        if(fault==5)f.r.Set(F::Menu+cfgprobe::ClassSuper,F::Blueprint);
        if(fault==6)f.r.Set(F::Object+cfgprobe::ObjectClass,std::uintptr_t{3});
        if(fault==7){f.r.changeAt=F::Object+cfgprobe::ObjectClass;
            f.r.changeSlot=F::Module+cfgmenu::controllergate::MenuConstructor;f.r.newPointer=F::Gameplay;}
        if(fault==8)f.r.Set(F::Object+cfgprobe::ObjectClass,F::Menu); // native instance also valid
        const bool accepted=cfgmenu::controllergate::Menu(f.r,F::Module,F::Object);
        if(accepted!=(fault==0||fault==8)){++failures;std::printf("FAIL controller case %u\n",fault);}
        if(f.r.reads>80){++failures;std::puts("FAIL controller read bound");}
    }
    std::printf("CFG menu-vs-gameplay controller siblings: %s (9 cases, %u failures). Owned memory only.\n",failures?"FAIL":"PASS",failures);
    return failures?1:0;
}
