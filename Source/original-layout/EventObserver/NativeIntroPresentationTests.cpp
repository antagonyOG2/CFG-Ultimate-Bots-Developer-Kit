#define main ExistingSelectedSkinTests
#include "NativeEndingSkinTests.cpp"
#undef main
#include "NativeIntroPresentation.hpp"
int main(){unsigned checks=0,failures=0;const auto check=[&](bool x){++checks;if(!x){++failures;std::printf("FAIL %u\n",checks);}};
    constexpr std::uintptr_t World=0x40000000,GameState=0x41000000;
    for(unsigned fault=0;fault<9;++fault){Fixture f;cfgending::skin::IntroPresentation lease;
        auto profile=f.selected;profile[16]=9;f.r.Set(f.State+cfgending::disk::PickedClass,profile);
        f.r.Set(f.State+cfgmenu::intrometadatadisk::ActiveCharacterClass,f.selected);
        f.r.Set(GameState+cfgmenu::intrometadatadisk::KillerClass,f.selected);
        f.r.Set(GameState+cfgmenu::intrometadatadisk::CurrentKillerPlayerState,f.State);
        std::uintptr_t out=0;
        // Old current-profile source correctly refuses: that path was never
        // assigned by the intro borrow. The real borrowed pair proves selection.
        check(!f.Source(out));
        if(fault==1)f.r.Set(GameState+cfgmenu::intrometadatadisk::KillerClass,profile);
        if(fault==2)f.r.Set(f.State+cfgmenu::intrometadatadisk::ActiveCharacterClass,profile);
        if(fault==3)f.r.Set(GameState+cfgmenu::intrometadatadisk::CurrentKillerPlayerState,std::uintptr_t{});
        if(fault==4){f.r.Set(f.State+cfgending::disk::PickedSkin,std::uintptr_t{});
            f.r.Set(f.Module+cfgending::disk::SCJasonSkinConstructor,std::uintptr_t{});}
        if(fault==5)f.r.Set(f.Data+24ULL*4+16,int{});
        if(fault==6)f.r.Set(f.State+0x88,std::uintptr_t{});
        if(fault==7)f.r.Set(f.Module+0x03199662,std::uint8_t{1});
        const auto capture=[&](){return lease.Capture(f.r,f.Module,f.registry,World,GameState,f.Controller,f.State,f.selected,[&]{return fault!=8;});};
        if(fault){check(!capture()&&!lease.Ready());continue;}
        check(capture()&&lease.Ready()&&!lease.stage);
        f.r.Set(f.State+cfgmenu::intrometadatadisk::ActiveCharacterClass,profile);
        f.r.Set(GameState+cfgmenu::intrometadatadisk::KillerClass,profile);
        f.r.Set(GameState+cfgmenu::intrometadatadisk::CurrentKillerPlayerState,std::uintptr_t{});
        check(lease.Current(f.r,f.registry,World,f.Controller,f.selected,out)&&out==f.Skin);
        check(!lease.Current(f.r,f.registry,World+8,f.Controller,f.selected,out)&&!out);
        check(!lease.Current(f.r,f.registry,World,f.Controller+8,f.selected,out));
        check(!lease.Current(f.r,f.registry,World,f.Controller,profile,out));
        f.r.Set(f.State+cfgending::disk::PickedClass,f.selected);
        check(!lease.Current(f.r,f.registry,World,f.Controller,f.selected,out));
        f.r.Set(f.State+cfgending::disk::PickedClass,profile);f.r.Set(f.State+cfgending::disk::PickedSkin,f.Base);
        check(!lease.Current(f.r,f.registry,World,f.Controller,f.selected,out));
    }
    {Fixture f;cfgending::skin::IntroPresentation lease;std::uintptr_t out=0;
     f.r.Set(f.State+cfgending::disk::PickedSkin,std::uintptr_t{});
     f.r.Set(f.State+cfgmenu::intrometadatadisk::ActiveCharacterClass,f.selected);
     f.r.Set(GameState+cfgmenu::intrometadatadisk::KillerClass,f.selected);
     f.r.Set(GameState+cfgmenu::intrometadatadisk::CurrentKillerPlayerState,f.State);
     check(lease.Capture(f.r,f.Module,f.registry,World,GameState,f.Controller,f.State,f.selected,[]{return true;}));
     check(lease.Ready()&&lease.observedSkin==f.Base);
     check(lease.Current(f.r,f.registry,World,f.Controller,f.selected,out)&&out==f.Base);
     f.r.Set(f.State+cfgending::disk::PickedSkin,f.Skin);
     check(!lease.Current(f.r,f.registry,World,f.Controller,f.selected,out));
     f.r.Set(f.State+cfgending::disk::PickedSkin,std::uintptr_t{});
     f.r.Set(f.Module+cfgending::disk::SCJasonSkinConstructor,f.Skin);
     check(!lease.Current(f.r,f.registry,World,f.Controller,f.selected,out));
     f.r.Set(f.Module+cfgending::disk::SCJasonSkinConstructor,f.Base);
     f.r.Set(f.BaseCdo+cfgprobe::ObjectClass,f.Skin);
     check(!lease.Current(f.r,f.registry,World,f.Controller,f.selected,out));}
    std::printf("Intro presentation selected-owner witness: %u checks,%u failures; no native writes/calls.\n",checks,failures);return failures?1:0;
}
