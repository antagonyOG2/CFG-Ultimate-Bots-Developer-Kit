#include "../RuntimeProbe/ActiveMenuObservation.hpp"
#include "OwnedByteReader.hpp"
#include <cstdio>
struct Fixture {
    static constexpr std::uintptr_t Module=0x100000,Engine=0x200000,Viewport=0x210000,World=0x220000,Instance=0x230000,
        Players=0x240000,Player=0x250000,Controller=0x260000,HUD=0x270000,Stack=0x280000,Records=0x290000,
        Widget=0x300000,Type=0x310000,Registry=0x320000,OtherWidget=0x330000,Mode=0x340000;
    OwnedByteReader reader;
    Fixture(){
        using namespace cfgprobe;using namespace cfgmenu::execdisk;
        reader.Set(Module+dispatch::ThreadFlagRva,std::uint8_t{1});reader.Set(Module+dispatch::ThreadIdRva,std::uint32_t{42});
        reader.Set(Module+EngineGlobalCandidate,Engine);reader.Set(Engine+EngineGameViewport,Viewport);reader.Set(Viewport+ViewportWorld,World);
        reader.Set(World+WorldNetDriver,std::uintptr_t{0});reader.Set(World+WorldGameInstance,Instance);reader.Set(World+WorldGameMode,Mode);
        reader.Set(Instance+GameInstanceLocalPlayers,Players);reader.Set(Instance+GameInstanceLocalPlayers+8,std::int32_t{1});reader.Set(Instance+GameInstanceLocalPlayers+12,std::int32_t{1});
        reader.Set(Players,Player);reader.Set(Player+cfgprobe::PlayerController,Controller);reader.Set(Controller+ControllerHUD,HUD);reader.Set(HUD+HUDStack,Stack);
        reader.Set(Stack+StackArray,cfgmenu::active::StackArray{Records,2,4});reader.Set(Records+RecordWidget,OtherWidget);
        reader.Set(Records+RecordBytes+RecordWidget,Widget);reader.Set(Widget+ObjectClass,Type);
        reader.Set(Module+lifetime::ObjectArrayRva+16,Registry);reader.Set(Module+lifetime::ObjectArrayRva+28,std::int32_t{4});
        const std::array<std::uintptr_t,4> objects{{World,Controller,Widget,HUD}};
        for(unsigned i=0;i<objects.size();++i){reader.Set(objects[i]+lifetime::ObjectIndexOffset,static_cast<int>(i));
            reader.Set(Registry+24*i,objects[i]);reader.Set(Registry+24*i+16,static_cast<int>(100+i));}
    }
};
int main(){unsigned failures=0;const auto check=[&](bool value,const char* name){if(!value){++failures;std::printf("FAIL %s\n",name);}};
    Fixture f;cfgmenu::active::Context current;
    check(cfgmenu::active::Observe(f.reader,Fixture::Module,current)&&current.widget==Fixture::Widget&&current.widget!=Fixture::OtherWidget,
        "top menu only, loaded/bottom widget not current");
    check(f.reader.reads<=100,"bounded current-chain reads, no discovery census");std::printf("Active menu observation reads=%u\n",f.reader.reads);
    check(cfgmenu::active::Current(f.reader,current,42)&&!cfgmenu::active::Current(f.reader,current,41),"native owner thread only");
    for(unsigned fault=0;fault<11;++fault){Fixture changed;
        if(fault==0)changed.reader.Set(Fixture::World+cfgprobe::WorldNetDriver,std::uintptr_t{0x500000});
        if(fault==1)changed.reader.Set(Fixture::Instance+cfgprobe::GameInstanceLocalPlayers+8,std::int32_t{2});
        if(fault==2)changed.reader.Set(Fixture::Stack+cfgmenu::execdisk::StackArray,cfgmenu::active::StackArray{Fixture::Records,0,4});
        if(fault==3)changed.reader.Set(Fixture::Stack+cfgmenu::execdisk::StackArray,cfgmenu::active::StackArray{Fixture::Records,65,65});
        if(fault==4)changed.reader.Set(Fixture::Registry+24*2+16,std::int32_t{0});
        if(fault==5)changed.reader.Set(Fixture::Registry+24*2,Fixture::OtherWidget);
        if(fault==6)changed.reader.Set(Fixture::Widget+cfgprobe::lifetime::ObjectIndexOffset,std::int32_t{4});
        if(fault==7){changed.reader.changeAt=Fixture::Widget+cfgprobe::ObjectClass;changed.reader.changeSlot=Fixture::HUD+cfgmenu::execdisk::HUDStack;changed.reader.newPointer=Fixture::OtherWidget;}
        if(fault==8)changed.reader.Set(Fixture::Module+cfgprobe::dispatch::ThreadFlagRva,std::uint8_t{0});
        if(fault==9)changed.reader.Set(Fixture::Registry+24*3+16,std::int32_t{0});
        if(fault==10)changed.reader.Set(Fixture::Registry+24*3,Fixture::OtherWidget);
        cfgmenu::active::Context out;check(!cfgmenu::active::Observe(changed.reader,Fixture::Module,out)&&!out.widget,"bad/current-changing chain refuses");
    }
    f.reader.Set(Fixture::Registry+24*2+16,std::int32_t{103});
    check(!cfgmenu::active::Current(f.reader,current,42),"same address with replaced serial is not the old menu");
    Fixture hudChanged;cfgmenu::active::Context hudContext;check(cfgmenu::active::Observe(hudChanged.reader,Fixture::Module,hudContext),"HUD identity observed");
    hudChanged.reader.Set(Fixture::Registry+24*3+16,std::int32_t{105});
    check(!cfgmenu::active::Current(hudChanged.reader,hudContext,42),"same address with replaced HUD serial refuses");
    std::printf("CFG active menu owner-chain/replacement/thread tests: %s (%u failures). No game access.\n",failures?"FAIL":"PASS",failures);return failures?1:0;
}
