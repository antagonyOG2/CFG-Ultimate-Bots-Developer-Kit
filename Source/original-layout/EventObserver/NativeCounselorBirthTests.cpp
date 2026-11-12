#define main ExistingActiveMenuTests
#include "ActiveMenuObservationTests.cpp"
#undef main
#include "../RuntimeProbe/NativeCounselorBirthObservation.hpp"
struct BirthFixture:Fixture {
    static constexpr std::uintptr_t Pawn=0x400000,State=0x410000,PawnType=0x420000,StateType=0x430000,
        SandboxType=0x440000,ControllerType=0x450000,KillerType=0x460000,Vtable=0x470000;
    cfgobserver::Plan plan;
    BirthFixture(){using namespace cfgprobe;plan.bytes=sizeof(plan);plan.threadId=42;plan.module=Module;
        plan.engine=Engine;plan.viewport=Viewport;plan.world=World;plan.mode=Mode;plan.controller=Controller;
        plan.pawn=Pawn;plan.player=Player;plan.gameInstance=Instance;plan.localPlayerData=Players;plan.objectArray={Registry,6};
        plan.dispatch={1,42,Vtable,Module+dispatch::NativeActorDispatcherRva};
        const std::array<std::uintptr_t,4> objects{{World,Mode,Controller,Pawn}};
        reader.Set(Module+lifetime::ObjectArrayRva+28,int{6});
        for(unsigned i=0;i<objects.size();++i){plan.objects[i]={objects[i],static_cast<int>(i),static_cast<int>(100+i)};
            reader.Set(objects[i]+lifetime::ObjectIndexOffset,static_cast<int>(i));
            reader.Set(Registry+24*i,objects[i]);reader.Set(Registry+24*i+16,static_cast<int>(100+i));}
        reader.Set(State+lifetime::ObjectIndexOffset,int{4});reader.Set(Registry+24*4,State);
        reader.Set(Registry+24*4+8,std::uint32_t{0});reader.Set(Registry+24*4+16,int{104});
        reader.Set(Pawn,Vtable);reader.Set(Vtable+dispatch::EventSlotBytes,plan.dispatch.target);
        reader.Set(Controller+ControllerPlayer,Player);reader.Set(Controller+ControllerAcknowledgedPawn,Pawn);reader.Set(Controller+ControllerPawn,Pawn);
        const std::array<NativeClass,4> ids{{SandboxMode,Counselor,Killer,PlayerControllerClass}};
        const std::array<std::uintptr_t,4> classes{{SandboxType,PawnType,KillerType,ControllerType}};
        for(unsigned i=0;i<ids.size();++i){const auto& b=NativeClasses[ids[i]];reader.Set(Module+b.reflectionCacheRva,classes[i]);
            reader.Set(Module+b.constructorCacheRva,classes[i]);reader.Set(classes[i]+ClassSuper,std::uintptr_t{0});}
        reader.Set(Module+cfgmenu::selectiondisk::SCPlayerStateClass,StateType);reader.Set(Module+cfgmenu::selectiondisk::SCPlayerStateConstructor,StateType);
        reader.Set(StateType+ClassSuper,std::uintptr_t{0});reader.Set(Mode+ObjectClass,SandboxType);reader.Set(Pawn+ObjectClass,PawnType);
        reader.Set(Controller+ObjectClass,ControllerType);reader.Set(State+ObjectClass,StateType);
        reader.Set(Controller+0x388,State);reader.Set(Pawn+0x388,State);reader.Set(Pawn+0x3A0,Controller);reader.Set(State+cfgspawn::ActorOwner,Controller);
        reader.Set(State+0x394,std::uint32_t{0});reader.Set(State+cfgmenu::selectiondisk::SpawnedCharacterClass,PawnType);
        std::array<unsigned char,40> active{};active[16]=1;reader.Set(State+cfgmenu::selectiondisk::ActiveCharacterClass,active);
    }
};
#ifndef CFGMENU_BIRTH_FIXTURE_ONLY
int main(){unsigned failures=0;const auto check=[&](bool okay,const char* name){if(!okay){++failures;std::printf("FAIL %s\n",name);}};
    for(unsigned fault=0;fault<16;++fault){BirthFixture f;
        if(fault==1)f.reader.Set(f.Controller+cfgprobe::ControllerAcknowledgedPawn,f.State);
        if(fault==2)f.reader.Set(f.Controller+cfgprobe::ControllerPawn,f.State);
        if(fault==3)f.reader.Set(f.Pawn+cfgprobe::ObjectClass,f.KillerType);
        if(fault==4)f.reader.Set(f.State+cfgspawn::ActorOwner,f.Player);
        if(fault==5)f.reader.Set(f.Pawn+0x388,f.Controller);
        if(fault==6)f.reader.Set(f.State+0x394,std::uint32_t{4});
        if(fault==7)f.reader.Set(f.State+cfgmenu::selectiondisk::SpawnedCharacterClass,f.KillerType);
        if(fault==8)f.reader.Set(f.Registry+24*4+16,int{0});
        if(fault==9)f.reader.Set(f.State+cfgmenu::selectiondisk::ActiveCharacterClass,std::array<unsigned char,40>{});
        if(fault==10){f.reader.changeAt=f.State+cfgmenu::selectiondisk::ActiveCharacterClass;f.reader.changeSlot=f.Controller+cfgprobe::ControllerPawn;f.reader.newPointer=f.State;}
        if(fault==11)f.reader.Set(f.World+cfgprobe::WorldNetDriver,f.Controller);
        if(fault==14)f.reader.Set(f.Module+cfgprobe::NativeClasses[cfgprobe::Counselor].constructorCacheRva,f.KillerType);
        if(fault==15){f.reader.changeAt=f.State+cfgmenu::selectiondisk::ActiveCharacterClass;f.reader.changeSlot=f.Registry+24*4+16;f.reader.newPointer=std::uintptr_t{999};}
        cfgmenu::birth::Observation out;unsigned reads=0;
        const auto okay=cfgmenu::birth::Observe(f.reader,f.plan,fault==12?41:42,fault!=13,out,&reads);
        check(okay==(fault==0||fault==9),"fresh native counselor birth only; empty unused Active allowed, wrong ownership/class/serial refuse");
        check(reads<=cfgmenu::birth::ReadLimit,"bounded arrival-only reads");
        if(okay){check(out.pawn==f.Pawn&&out.type==f.PawnType&&out.playerState==f.State,"preserve actual Sandbox counselor, no default");std::printf("Native counselor birth reads=%u\n",reads);}
        else check(!out.pawn&&!out.type,"no partial/stale birth evidence");
    }
    for(unsigned fault=0;fault<8;++fault){BirthFixture f;using namespace cfgmenu;
        f.reader.Set(f.State+selectiondisk::SpawnedCharacterClass,std::uintptr_t{0});
        f.reader.Set(f.Module+birthdisk::SCPlayerController_SandboxClass,f.ControllerType);
        f.reader.Set(f.Module+birthdisk::SCPlayerController_SandboxConstructor,f.ControllerType);
        std::array<unsigned char,40> empty{};const std::int32_t weakIndex=-1;std::memcpy(empty.data(),&weakIndex,4);
        f.reader.Set(f.State+selectiondisk::ActiveCharacterClass,empty);
        if(fault==1)f.reader.Set(f.Pawn+cfgprobe::ObjectClass,f.KillerType);
        if(fault==2)f.reader.Set(f.Controller+cfgprobe::ControllerAcknowledgedPawn,std::uintptr_t{0});
        if(fault==3)f.reader.Set(f.Module+birthdisk::SCPlayerController_SandboxConstructor,f.KillerType);
        if(fault==4)f.reader.Set(f.Module+birthdisk::SCPlayerController_SandboxClass,f.KillerType);
        if(fault==5)f.reader.Set(f.State+selectiondisk::SpawnedCharacterClass,f.KillerType);
        if(fault==6){f.reader.changeAt=f.State+selectiondisk::ActiveCharacterClass;f.reader.changeSlot=f.Controller+cfgprobe::ControllerPawn;f.reader.newPointer=f.KillerType;}
        if(fault==7)f.reader.Set(f.State+0x394,std::uint32_t{4});
        birth::Observation out;birth::Trace trace;
        check(!birth::Observe(f.reader,f.plan,42,true,out,nullptr,&trace),"missing Spawned plus empty Active never approves a default/spectator fallback");
        if(!fault)check(trace.step==birth::Step::NativeSelection&&!trace.activeNonempty&&trace.spawned==0,"canonical empty weak packet is absent, not a selected class");
    }
    std::printf("CFG native counselor birth tests: %s (%u failures). Owned fixtures only; no game access.\n",failures?"FAIL":"PASS",failures);return failures?1:0;
}
#endif
