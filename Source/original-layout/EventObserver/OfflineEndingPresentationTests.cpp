#define main ExistingSkinMain
#include "NativeEndingSkinTests.cpp"
#undef main
#include "OfflineEndingPresentation.hpp"
#include <cassert>
int main(){assert(ExistingSkinMain()==0);
    for(unsigned fault=0;fault<10;++fault){Fixture f;constexpr std::uintptr_t world=0x30100000,pc=0x30110000,pawn=0x30120000,human=0x30130000;
        f.Object(world,6,66);f.Object(pc,7,77);f.Object(pawn,8,88);
        f.r.Set(world+cfgprobe::WorldNetDriver,std::uintptr_t{});f.r.Set(f.State+0x394,std::uint32_t{4});
        f.r.Set(f.State+0x110,std::uint8_t{3});f.r.Set(f.State+0x88,pc);f.r.Set(pc+0x388,f.State);
        f.r.Set(pawn+0x388,f.State);f.r.Set(pawn+0x3A0,pc);f.r.Set(pc+0x370,pawn);
        f.r.Set(f.State+cfgending::disk::PickedSkin,std::uintptr_t{});
        if(fault==1)f.r.Set(world+cfgprobe::WorldNetDriver,pc);
        if(fault==2)f.r.Set(f.State+0x394,std::uint32_t{});
        if(fault==3)f.r.Set(f.State+0x88,human);
        if(fault==4)f.r.Set(pawn+0x388,human);
        if(fault==5)f.r.Set(f.State+cfgending::disk::PickedSkin,f.Skin);
        if(fault==6)f.r.Set(f.State+0x110,std::uint8_t{2});
        if(fault==7)f.r.Set(f.Module+0x03199662,std::uint8_t{1});
        unsigned writes=0;
        const auto applied=s::PublishOfflinePresentation(f.r,f.Module,f.registry,world,f.State,pc,pawn,fault==8?f.State:human,f.Skin,
            [&](){return fault!=9;},[&](std::uintptr_t field,std::uintptr_t value){++writes;f.r.Set(field,value);return true;});
        assert(applied==(fault==0)&&writes==(fault==0?1u:0u));
    }
    std::puts("Offline AI outro skin metadata PASS: null-only owned offline transfer; foreign/online/GC/existing refused; native gameplay flags unchanged.");
}
