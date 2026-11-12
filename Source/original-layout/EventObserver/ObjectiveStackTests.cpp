#define main CfgActiveMenuFixtureMain
#include "ActiveMenuObservationTests.cpp"
#undef main
#include "ObjectiveStackObservation.hpp"
int main(){
    unsigned failures=0;unsigned peak=0;
    for(unsigned fault=0;fault<9;++fault){Fixture f;
        using namespace cfgmenu::execdisk;
        f.reader.Set(Fixture::Registry+24*4,Fixture::Stack);f.reader.Set(Fixture::Registry+24*4+16,104);
        f.reader.Set(Fixture::Stack+cfgprobe::lifetime::ObjectIndexOffset,4);
        f.reader.Set(Fixture::Module+cfgprobe::lifetime::ObjectArrayRva+28,5);
        for(unsigned i=0;i<5;++i)f.reader.Set(Fixture::Registry+24*i+8,std::uint32_t{0});
        f.reader.Set(Fixture::OtherWidget+cfgprobe::ObjectClass,std::uintptr_t{0x400000});
        f.reader.Set(Fixture::Stack+0x20,Fixture::HUD);f.reader.Set(Fixture::Widget+0x20,Fixture::Stack);
        if(fault==1)f.reader.Set(Fixture::Stack+0x20,Fixture::Controller);
        if(fault==2)f.reader.Set(Fixture::Widget+0x20,Fixture::HUD);
        if(fault==3)f.reader.Set(Fixture::HUD+HUDStack,Fixture::Widget);
        if(fault==4)f.reader.Set(Fixture::Registry+24*2+16,0);
        if(fault==5)f.reader.Set(Fixture::OtherWidget+cfgprobe::ObjectClass,Fixture::Type);
        if(fault==6){f.reader.changeAt=Fixture::Widget+cfgprobe::ObjectClass;f.reader.changeSlot=Fixture::Records+RecordBytes+RecordWidget;f.reader.newPointer=Fixture::OtherWidget;}
        if(fault==7)f.reader.Set(Fixture::Stack+StackArray,cfgmenu::active::StackArray{Fixture::Records,65,65});
        if(fault==8)f.reader.Set(Fixture::Controller+ControllerHUD,Fixture::OtherWidget);
        cfgprobe::lifetime::ArrayHeader registry;cfgobjective::stack::Observation out;
        const bool header=cfgprobe::lifetime::ReadArrayHeader(f.reader,Fixture::Module+cfgprobe::lifetime::ObjectArrayRva,registry);
        if(!fault&&header){
            auto stale=registry;stale.count=2;
            if(cfgobjective::stack::Observe(f.reader,stale,Fixture::Controller,Fixture::HUD,Fixture::Stack,Fixture::Type,out))++failures;
            // Newly born HUD/stack/widget slots must be admitted using the
            // fresh registry header, never the original Plan's count limit.
            f.reader.reads=0;
        }
        const bool ok=header&&cfgobjective::stack::Observe(f.reader,registry,Fixture::Controller,Fixture::HUD,Fixture::Stack,Fixture::Type,out);
        if(ok!=(fault==0)||(!fault&&(out.widget.object!=Fixture::Widget||out.index!=1)))++failures;
        peak=(std::max)(peak,f.reader.reads);
    }
    std::printf("ObjectiveStackTests: %u failures, peak%u reads; live local chain only, repeated serial/outer/header/slot proof, no census/writes\n",failures,peak);
    return failures?1:0;
}
