#define CFG_OFFLINE_OWNER_FIXTURE_ONLY
#include "NativeOfflineOwnerObservationTests.cpp"
#include "TommyEndRaceObservation.hpp"
int main(){unsigned failures=0,checks=0;const auto check=[&](bool x){++checks;if(!x){++failures;std::printf("FAIL %u\n",checks);}};
    constexpr std::uintptr_t State=0x610000,KillerPawn=0x620000;
    for(unsigned fault=0;fault<21;++fault){Fixture f;cfgmenu::promotion::Context owner;
        f.r.Set(f.Module+cfgprobe::lifetime::ObjectArrayRva+28,8);
        for(unsigned i=0;i<2;++i){const auto object=i?KillerPawn:State;f.r.Set(object+0xC,static_cast<int>(6+i));
            f.r.Set(f.Registry+24ULL*(6+i),object);f.r.Set(f.Registry+24ULL*(6+i)+16,static_cast<int>(106+i));}
        for(unsigned i=0;i<8;++i)f.r.Set(f.Registry+24ULL*i+8,std::uint32_t{});
        f.r.Set(f.World+cfgprobe::WorldGameState,State);f.r.Set(State+0x420,KillerPawn);
        f.r.Set(State+cfgprobe::ObjectClass,std::uintptr_t{0x630000});f.r.Set(std::uintptr_t{0x630000}+cfgprobe::ClassSuper,std::uintptr_t{});
        f.r.Set(f.Module+cfgescape::disk::SCGameStateReflection,std::uintptr_t{0x630000});f.r.Set(f.Module+cfgescape::disk::SCGameStateConstructor,std::uintptr_t{0x630000});
        f.r.Set(KillerPawn+cfgprobe::ObjectClass,f.Killer);
        f.r.Set(KillerPawn+cfghunter::disk::Health,100.f);f.r.Set(KillerPawn+cfgescape::disk::DyingFlags,std::uint8_t{});
        f.Name(0,"InProgress");f.r.Set(f.Mode+cfgescape::disk::MatchState,cfgnames::Words{});
        f.r.Set(f.Mode+cfgescape::disk::HunterToSpawn,f.Controller);f.r.Set(State+cfgescape::disk::HunterTimeRemaining,0);
        f.r.Set(f.Controller+cfgprobe::ControllerPawn,std::uintptr_t{});f.r.Set(f.Controller+0x110,std::uint8_t{3});
        f.r.Set(f.ControllerType+cfgprobe::ClassSuper,std::uintptr_t{});
        f.r.Set(f.Module+cfgescape::disk::ControllerReflection,f.ControllerType);f.r.Set(f.Module+cfgescape::disk::ControllerConstructor,f.ControllerType);
        f.r.Set(f.Module+cfgtommyend::disk::Timer,cfgtommyend::disk::TimerCode);
        check(f.Observe(owner));
        auto receiver=f.Mode,caller=f.Module+cfgtommyend::disk::TimerReturn;unsigned thread=42,reason=99;
        if(fault==1)++caller;if(fault==2)++receiver;if(fault==3)++thread;
        if(fault==4)f.r.Set(f.Mode+cfgescape::disk::HunterToSpawn,std::uintptr_t{});
        if(fault==5)f.r.Set(f.World+cfgprobe::WorldNetDriver,f.Mode);
        if(fault==6)f.r.Set(KillerPawn+cfghunter::disk::Health,0.f);
        if(fault==7)f.r.Set(KillerPawn+cfgescape::disk::DyingFlags,std::uint8_t{1});
        if(fault==8)f.r.Set(f.Module+cfgtommyend::disk::Timer,std::uint8_t{});
        if(fault==9)f.r.Set(f.Module+0x03198110,std::uint32_t{1});
        if(fault==10)f.r.Set(f.Registry+24*2+16,0);
        if(fault==11)f.Name(0,"WaitingPostMatchOutro");
        if(fault==12){f.r.Set(f.Mode+cfgescape::disk::HunterToSpawn,std::uintptr_t{});f.r.Set(State+cfgescape::disk::HunterTimeRemaining,3);}
        if(fault==13){f.r.Set(f.Mode+cfgescape::disk::HunterToSpawn,std::uintptr_t{});f.r.Set(State+cfgescape::disk::HunterTimeRemaining,601);}
        if(fault==14)f.r.Set(KillerPawn+cfgprobe::ObjectClass,f.Counselor);
        if(fault==15)f.r.Set(State+cfgprobe::ObjectClass,f.Counselor);
        if(fault==16)f.r.Set(f.Module+cfgescape::disk::SCGameStateConstructor,f.Counselor);
        if(fault==17)f.r.Set(f.Controller+0x110,std::uint8_t{1});
        if(fault==18)f.r.Set(f.Registry+24*6+16,0);
        if(fault==19)f.r.Set(f.Registry+24*7+8,std::uint32_t{0x20000000});
        if(fault==20)caller=f.Module+0x3CF22E; // legitimate base countdown/timeout return must never veto
        f.r.reads=0;const bool protectedEnd=cfgtommyend::Protect(f.r,owner,State,receiver,caller,thread,reason);
        check(protectedEnd==(fault==0||fault==12));check(protectedEnd?reason==(fault==12?2u:1u):reason==0);
        check(f.r.reads<=4096);
    }
    for(unsigned fault=0;fault<10;++fault){Fixture f;cfgmenu::promotion::Context owner;
        f.r.Set(f.Module+cfgprobe::lifetime::ObjectArrayRva+28,8);
        for(unsigned i=0;i<2;++i){const auto object=i?KillerPawn:State;f.r.Set(object+0xC,static_cast<int>(6+i));
            f.r.Set(f.Registry+24ULL*(6+i),object);f.r.Set(f.Registry+24ULL*(6+i)+16,static_cast<int>(106+i));}
        for(unsigned i=0;i<8;++i)f.r.Set(f.Registry+24ULL*i+8,std::uint32_t{});
        f.r.Set(f.World+cfgprobe::WorldGameState,State);f.r.Set(State+0x420,KillerPawn);
        f.r.Set(State+cfgprobe::ObjectClass,std::uintptr_t{0x630000});f.r.Set(std::uintptr_t{0x630000}+cfgprobe::ClassSuper,std::uintptr_t{});
        f.r.Set(f.Module+cfgescape::disk::SCGameStateReflection,std::uintptr_t{0x630000});f.r.Set(f.Module+cfgescape::disk::SCGameStateConstructor,std::uintptr_t{0x630000});
        f.r.Set(KillerPawn+cfgprobe::ObjectClass,f.Killer);
        f.r.Set(KillerPawn+cfghunter::disk::Health,100.f);f.r.Set(KillerPawn+cfgescape::disk::DyingFlags,std::uint8_t{});
        f.Name(0,"InProgress");f.r.Set(f.Mode+cfgescape::disk::MatchState,cfgnames::Words{});
        f.r.Set(f.Mode+cfgescape::disk::HunterToSpawn,std::uintptr_t{});f.r.Set(State+cfgescape::disk::HunterTimeRemaining,0);
        f.r.Set(f.Module+cfgtommyend::disk::Timer,cfgtommyend::disk::TimerCode);
        f.Birth();f.r.Set(f.Module+cfgescape::disk::SCCounselorCharacterReflection,f.Counselor);
        f.r.Set(f.Module+cfgescape::disk::SCCounselorCharacterConstructor,f.Counselor);
        f.r.Set(f.Module+cfgescape::disk::SCPlayerStateReflection,f.StateType);
        f.r.Set(f.Module+cfgescape::disk::SCPlayerStateConstructor,f.StateType);
        f.r.Set(f.StateType+cfgprobe::ClassSuper,std::uintptr_t{});
        f.r.Set(f.Pawn+cfgescape::disk::bHasEscaped,std::uint8_t{});f.r.Set(f.Pawn+cfghunter::disk::Health,100.f);
        f.r.Set(f.PS+cfgescape::disk::bDead,std::uint8_t{});f.r.Set(f.PS+cfgescape::disk::bEscaped,std::uint8_t{});
        check(f.Observe(owner));
        if(fault==1)f.r.Set(f.Pawn+cfghunter::disk::Health,0.f);
        if(fault==2)f.r.Set(f.PS+cfgescape::disk::bDead,std::uint8_t{1});
        if(fault==3)f.r.Set(f.Pawn+cfgescape::disk::bHasEscaped,std::uint8_t{1});
        if(fault==4)f.r.Set(f.Registry+24*5+16,0);
        if(fault==5)f.r.Set(f.Pawn+cfgescape::disk::Controller,f.Mode);
        if(fault==6)f.r.Set(f.Pawn+cfgescape::disk::PlayerState,f.Mode);
        if(fault==7){f.r.watch=f.World+cfgprobe::WorldGameState;f.r.mutate=f.r.watch;f.r.value=f.Mode;}
        if(fault==8)f.r.Set(f.Registry+24*5+8,std::uint32_t{0x20000000});
        if(fault==9)f.r.Set(f.Pawn+cfgprobe::ObjectClass,f.Killer);
        unsigned reason=99;f.r.reads=0;
        const bool protectedEnd=cfgtommyend::Protect(f.r,owner,State,f.Mode,f.Module+cfgtommyend::disk::TimerReturn,42,reason);
        check(protectedEnd==(fault==0));check(reason==(fault==0?3u:0u));check(f.r.reads<=4096);
    }
    {Fixture f;f.r.Set(f.Module+cfgescape::disk::EndMatchNative,cfgescape::disk::EndMatchNativeCode);
        check(cfgtommyend::EndBodyCurrent(f.r,f.Module));
        auto owned=cfgescape::disk::EndMatchNativeCode;owned[0]=0xE9;
        f.r.Set(f.Module+cfgescape::disk::EndMatchNative,owned);
        check(!cfgtommyend::EndBodyCurrent(f.r,f.Module));
        cfgtommyend::guardedModule=f.Module;cfgtommyend::guardedCode=owned;
        check(cfgtommyend::EndBodyCurrent(f.r,f.Module));
        f.r.Set(f.Module+cfgescape::disk::EndMatchNative+50,std::uint8_t{0xFF});
        check(!cfgtommyend::EndBodyCurrent(f.r,f.Module));cfgtommyend::guardedModule=0;
    }
    std::printf("Tommy timer end-race proof: %u checks,%u failures; no native calls/writes.\n",checks,failures);return failures?1:0;
}
