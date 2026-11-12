#define CFG_FRONTEND_BUNDLE
#define CFGMENU_BIRTH_FIXTURE_ONLY
#include "NativeCounselorBirthTests.cpp"
#include "../RuntimeProbe/FreshGameplayObservation.hpp"
int main(){unsigned failures=0;const auto check=[&](bool okay,const char* label){if(!okay){++failures;std::printf("FAIL %s\n",label);}};
    using namespace cfgmenu;using namespace cfgprobe;
    for(unsigned fault=0;fault<19;++fault){BirthFixture f;
        freshgame::Source source{f.Module,f.Engine,f.Viewport,f.Player,f.Instance,0x900000,42};
        const std::array<std::uintptr_t,3> types{{f.SandboxType,f.ControllerType,f.PawnType}};
        for(unsigned i=0;i<types.size();++i){const auto defaults=std::uintptr_t{0xA00000}+i*0x10000;
            f.reader.Set(types[i]+0xF8,defaults);f.reader.Set(defaults+ObjectClass,types[i]);}
        if(fault==1)source.frontendWorld=f.World;
        if(fault==2)source.player=f.State;
        if(fault==3)f.reader.Set(f.Controller+ControllerAcknowledgedPawn,f.State);
        if(fault==4)f.reader.Set(f.Controller+ControllerPlayer,f.State);
        if(fault==5)f.reader.Set(f.World+WorldNetDriver,f.State);
        if(fault==6)f.reader.Set(f.Instance+GameInstanceLocalPlayers+8,int{2});
        if(fault==7)f.reader.Set(f.Registry+24*3+16,int{0});
        if(fault==8)f.reader.Set(f.Pawn+ObjectClass,f.KillerType);
        if(fault==9)f.reader.Set(f.State+selectiondisk::SpawnedCharacterClass,f.KillerType);
        if(fault==10)f.reader.Set(f.SandboxType+0xF8,f.Mode);
        if(fault==11)f.reader.Set(f.ControllerType+0xF8,f.Controller);
        if(fault==12)f.reader.Set(f.PawnType+0xF8,f.Pawn);
        if(fault==13)f.reader.Set(f.Controller+ControllerPawn,std::uintptr_t{0});
        if(fault==14)f.reader.Set(f.Vtable+dispatch::EventSlotBytes,std::uintptr_t{0xDEAD0000});
        if(fault==17){f.reader.changeAt=f.State+selectiondisk::ActiveCharacterClass;f.reader.changeSlot=f.Viewport+ViewportWorld;f.reader.newPointer=source.frontendWorld;}
        if(fault==18)f.reader.Set(f.State+selectiondisk::ActiveCharacterClass,std::array<unsigned char,40>{});
        freshgame::Observation out;freshgame::Trace trace;unsigned reads=999;
        const bool okay=freshgame::Observe(f.reader,source,fault==15?41u:42u,fault!=16,out,&reads,&trace);
        if(fault==9)check(trace.step==freshgame::Step::Birth&&trace.human.step==birth::Step::SpawnedClass&&trace.human.spawned==f.KillerType,"exact spawned-class rejection reported without extra reads");
        if(fault==18)check(trace.step==freshgame::Step::Ready&&trace.human.step==birth::Step::Ready&&!trace.human.activeNonempty,"empty unused Active is not an ownership failure");
        if(fault==10||fault==11||fault==12)check(trace.step==freshgame::Step::ClassDefaults&&trace.defaultIndex==fault-9,"class-default rejection identifies owner index");
        check(okay==(fault==0||fault==18),"fresh owned offline birth only; no old HUD/controller substitution");
        check(reads<=freshgame::ReadLimit,"finite arrival reads");
        if(okay){check(out.plan.pawn==f.Pawn&&out.plan.world==f.World&&out.human.type==f.PawnType&&!out.plan.frontend.widget,
            "fresh gameplay plan never carries frontend ownership");std::printf("Fresh owned gameplay plan reads=%u\n",reads);}
        else check(!out.plan.pawn&&!out.human.type,"refusal publishes no partial plan");
    }
    std::printf("CFG fresh gameplay owner/birth provider: %s (%u failures). Fixtures only; no game access.\n",failures?"FAIL":"PASS",failures);
    return failures?1:0;
}
