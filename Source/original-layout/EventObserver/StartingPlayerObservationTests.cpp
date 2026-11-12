#define main ExistingStartingPlayerOwnerTests
#include "ActiveMenuObservationTests.cpp"
#undef main
#include "StartingPlayerObservation.hpp"
#include <functional>
namespace sp=cfgmenu::starting;
struct StartingFixture:Fixture {
    static constexpr std::uintptr_t State=0x400000,ModeClass=0x410000,PCClass=0x420000,PSClass=0x430000,
        Counselor=0x440000,Killer=0x450000,Selected=0x460000,ModeDefault=0x470000,PCDefault=0x480000,PSDefault=0x490000,
        BaseMode=0x4A0000,BasePC=0x4B0000,Function=0x600000,Parameter=0x610000,Picked=0x620000,Packet=0x630000,
        FunctionKind=0x700000,ObjectKind=0x710000,SoftKind=0x720000,NameTable=0xA10000,NameChunk=0xA20000;
    unsigned nextName=0;sp::Event event{Module,Mode,Function,Packet,42,true};cfgmenu::promotion::Packet pick{};
    void Name(std::uintptr_t object,const char* label){
        const auto index=nextName++;const auto entry=std::uintptr_t{0xB00000}+index*0x100;
        reader.Set(NameChunk+8*index,entry);reader.Set(entry,std::uint32_t{0});std::array<unsigned char,128> text{};
        for(unsigned i=0;label[i]&&i<127;++i)text[i]=static_cast<unsigned char>(label[i]);
        reader.Set(entry+cfgnames::disk::EntryText,text);reader.Set(object+cfgnames::disk::ObjectName,cfgnames::Words{static_cast<int>(index),0});
        struct Counts{int entries,chunks;};reader.Set(NameTable+cfgnames::disk::ElementCount,Counts{static_cast<int>(nextName),1});
    }
    void Field(std::uintptr_t field,std::uintptr_t owner,std::uintptr_t type,const char* name){
        reader.Set(field+0x20,owner);reader.Set(field+cfgprobe::ObjectClass,type);reader.Set(field+0x28,std::uintptr_t{0});Name(field,name);
    }
    void Pair(std::uintptr_t cache,std::uintptr_t constructor,std::uintptr_t type){reader.Set(Module+cache,type);reader.Set(Module+constructor,type);}
    StartingFixture(){
        using namespace cfgprobe;using namespace cfgmenu;
        const std::array<std::uintptr_t,4> actors{{World,Mode,Controller,State}};
        for(unsigned i=0;i<actors.size();++i){reader.Set(actors[i]+lifetime::ObjectIndexOffset,static_cast<int>(i));
            reader.Set(Registry+24*i,actors[i]);reader.Set(Registry+24*i+16,static_cast<int>(100+i));}
        reader.Set(Controller+ControllerPlayer,Player);reader.Set(Controller+ControllerPawn,std::uintptr_t{0});reader.Set(Controller+ControllerAcknowledgedPawn,std::uintptr_t{0});
        reader.Set(Controller+0x388,State);reader.Set(State+0x88,Controller);reader.Set(State+birthdisk::SpawnedCharacterClass,std::uintptr_t{0});
        reader.Set(State+0x394,std::uint32_t{0});reader.Set(State+0x110,static_cast<unsigned char>(3));
        Pair(birthdisk::SCGameMode_SandboxClass,birthdisk::SCGameMode_SandboxConstructor,ModeClass);
        Pair(birthdisk::SCPlayerController_SandboxClass,birthdisk::SCPlayerController_SandboxConstructor,PCClass);
        Pair(birthdisk::SCPlayerStateClass,birthdisk::SCPlayerStateConstructor,PSClass);
        Pair(birthdisk::SCCounselorCharacterClass,birthdisk::SCCounselorCharacterConstructor,Counselor);
        Pair(birthdisk::SCKillerCharacterClass,birthdisk::SCKillerCharacterConstructor,Killer);
        Pair(startingdisk::GameModeBaseClass,startingdisk::GameModeBaseConstructor,BaseMode);
        Pair(startingdisk::PlayerControllerClass,startingdisk::PlayerControllerConstructor,BasePC);
        const std::array<std::uintptr_t,3> types{{ModeClass,PCClass,PSClass}},defaults{{ModeDefault,PCDefault,PSDefault}},objects{{Mode,Controller,State}};
        for(unsigned i=0;i<3;++i){reader.Set(objects[i]+ObjectClass,types[i]);reader.Set(types[i]+0xF8,defaults[i]);reader.Set(defaults[i]+ObjectClass,types[i]);}
        reader.Set(ModeClass+ClassSuper,BaseMode);reader.Set(BaseMode+ClassSuper,std::uintptr_t{0});
        reader.Set(PCClass+ClassSuper,BasePC);reader.Set(BasePC+ClassSuper,std::uintptr_t{0});reader.Set(PSClass+ClassSuper,std::uintptr_t{0});
        reader.Set(Selected+ClassSuper,Counselor);reader.Set(Counselor+ClassSuper,std::uintptr_t{0});reader.Set(Killer+ClassSuper,std::uintptr_t{0});
        reader.Set(Mode,Module+gatedisk::Table);reader.Set(Module+gatedisk::Table+0x630,Module+startingdisk::Implementation);
        reader.Set(Module+gatedisk::Table+cfgprobe::dispatch::EventSlotBytes,Module+cfgprobe::dispatch::NativeActorDispatcherRva);
        reader.Set(Module+startingdisk::FunctionCache,Function);reader.Set(Packet,Controller);
        reader.Set(Module+startingdisk::Wrapper,startingdisk::WrapperCode);reader.Set(Module+startingdisk::Executor,startingdisk::ExecutorCode);
        reader.Set(Module+startingdisk::Implementation,startingdisk::ImplementationCode);
        reader.Set(Module+cfgnames::disk::Global,NameTable);reader.Set(NameTable,NameChunk);
        Name(FunctionKind,"Function");Name(ObjectKind,"ObjectProperty");Name(SoftKind,"SoftClassProperty");
        Field(Function,BaseMode,FunctionKind,"HandleStartingNewPlayer");Field(Parameter,Function,ObjectKind,"NewPlayer");
        Field(Picked,PSClass,SoftKind,"PickedCounselorClass");reader.Set(PSClass+0x38,Picked);
        reader.Set(Function+0x88,std::uint32_t{startingdisk::FunctionFlags});reader.Set(Function+0x8C,static_cast<unsigned char>(1));
        reader.Set(Function+0x8E,std::uint16_t{8});reader.Set(Function+0x40,int{8});
        struct Script{std::uintptr_t data;int count,capacity;};reader.Set(Function+0x48,Script{});
        reader.Set(Function+0x68,std::uintptr_t{0});reader.Set(Function+0xB0,Module+startingdisk::Executor);reader.Set(Function+0x38,Parameter);
        reader.Set(Parameter,Module+startingdisk::ObjectPropertyVtable);reader.Set(Parameter+0x70,BasePC);
        reader.Set(Parameter+0x30,int{1});reader.Set(Parameter+0x34,int{8});reader.Set(Parameter+0x44,int{0});reader.Set(Parameter+0x38,startingdisk::ParameterFlags);
        reader.Set(Picked,Module+softdisk::PropertyVtable);reader.Set(Picked+0x30,int{1});reader.Set(Picked+0x34,int{40});
        reader.Set(Picked+0x44,int{0x748});reader.Set(Picked+0x38,startingdisk::PickedFlags);
        const std::uint64_t name=0x1234;std::memcpy(pick.bytes.data()+16,&name,8);reader.Set(State+birthdisk::PickedCounselorClass,pick);
    }
};
struct Resolver {
    StartingFixture& f;unsigned calls=0;bool success=true;std::uintptr_t selected=StartingFixture::Selected;std::function<void()> during;
    explicit Resolver(StartingFixture& fixture):f(fixture){}
    bool ResolveLoaded(std::uintptr_t source,const cfgmenu::promotion::Packet& packet,std::uintptr_t& result){
        ++calls;if(source!=f.State+cfgmenu::birthdisk::PickedCounselorClass||packet.bytes!=f.pick.bytes)return false;
        if(during)during();result=selected;return success;
    }
};
#ifndef CFG_STARTING_FIXTURE_ONLY
int main(){unsigned checks=0,failures=0;const auto check=[&](bool ok,const char* what){++checks;if(!ok){++failures;std::printf("FAIL %s\n",what);}};
    for(unsigned fault=0;fault<31;++fault){StartingFixture f;sp::Observation observed;
        if(fault==1)f.event.receiver=f.State;
        if(fault==2)f.event.function=f.Parameter;
        if(fault==3)f.event.thread=43;
        if(fault==4)f.event.exactImageVerified=false;
        if(fault==5)f.reader.Set(f.Packet,f.State);
        if(fault==6)f.reader.Set(f.Registry+24+16,int{0});
        if(fault==7)f.reader.Set(f.Registry+24*3+16,int{0});
        if(fault==8)f.reader.Set(f.World+cfgprobe::WorldNetDriver,f.State);
        if(fault==9)f.reader.Set(f.Controller+cfgprobe::ControllerPawn,f.State);
        if(fault==10)f.reader.Set(f.Controller+cfgprobe::ControllerAcknowledgedPawn,f.State);
        if(fault==11)f.reader.Set(f.State+cfgmenu::birthdisk::SpawnedCharacterClass,f.Selected);
        if(fault==12)f.reader.Set(f.Instance+cfgprobe::GameInstanceLocalPlayers+8,int{2});
        if(fault==13)f.reader.Set(f.ModeClass+0xF8,f.Mode);
        if(fault==14)f.reader.Set(f.Controller+cfgprobe::ObjectClass,f.PSClass);
        if(fault==15)f.reader.Set(f.Function+0x88,std::uint32_t{0});
        if(fault==16)f.reader.Set(f.Function+0x8E,std::uint16_t{16});
        if(fault==17)f.reader.Set(f.Function+0xB0,f.Module+cfgmenu::startingdisk::Wrapper);
        if(fault==18)f.reader.Set(f.Parameter+0x44,int{8});
        if(fault==19)f.reader.Set(f.Parameter+0x38,cfgmenu::startingdisk::ParameterDescriptorFlags);
        if(fault==20)f.reader.Set(f.Parameter+0x70,f.Counselor);
        if(fault==21)f.reader.Set(f.Parameter+0x28,f.Picked);
        if(fault==22)f.reader.Set(f.Module+cfgmenu::startingdisk::Wrapper,static_cast<unsigned char>(0));
        if(fault==23)f.reader.Set(f.Module+cfgmenu::gatedisk::Table+0x630,std::uintptr_t{0});
        if(fault==24)f.Name(f.Function,"HandleStartingNewPlayer_0");
        if(fault==25)f.Name(f.Parameter,"DifferentPlayer");
        if(fault==26)f.reader.Set(f.State+0x110,static_cast<unsigned char>(2));
        if(fault==27)f.reader.Set(f.State+0x394,std::uint32_t{2});
        if(fault==28)f.reader.Set(f.PSClass+0xF8,f.State);
        if(fault==29)f.reader.Set(f.Function+0x68,f.Picked);
        if(fault==30)f.reader.Set(f.Function+0x50,int{1});
        const bool ok=sp::Observe(f.reader,f.event,observed);check(ok==(fault==0),"event/schema/owner guards");
        check(fault?!observed.owner.mode:observed.owner.mode==f.Mode,"no partial output on refusal");
        check(f.reader.reads<sp::MaximumReads,"finite exact observation reads");
        if(!fault){check(!observed.owner.pickedResolutionVerified&&!observed.owner.selectedClass&&!observed.owner.softProperty,"owner observation does not fabricate selection");
            check(observed.owner.identities[1].serial==101&&observed.parameterProperty==f.Parameter,"fresh positive mode serial and exact parameter");
            Resolver resolver(f);cfgmenu::promotion::Context selected;
            const bool produced=sp::ResolvePicked(f.reader,observed.owner,42,resolver,selected);
            check(cfgmenu::startingdisk::PickedFlagsVerified?produced&&resolver.calls==1&&selected.pickedResolutionVerified:
                    !produced&&resolver.calls==0&&!selected.mode,
                "3.5 picked-property resolver runs only with proved runtime flags");
            std::printf("Starting-player owner reads=%u\n",observed.reads);}
    }
    for(unsigned scenario=0;scenario<15;++scenario){StartingFixture f;sp::Observation observed;check(sp::Observe(f.reader,f.event,observed),"selection fixture observed");
        Resolver resolver(f);cfgmenu::promotion::Context selected;
        if(scenario==0)f.reader.Set(f.State+cfgmenu::birthdisk::PickedCounselorClass,cfgmenu::promotion::Packet{});
        if(scenario==1)resolver.selected=f.Killer;
        if(scenario==2)resolver.success=false;
        if(scenario==3)resolver.during=[&](){f.reader.Set(f.State+cfgmenu::birthdisk::PickedCounselorClass,cfgmenu::promotion::Packet{});};
        if(scenario==4)resolver.during=[&](){f.reader.Set(f.Registry+24+16,int{999});};
        if(scenario==5)resolver.during=[&](){f.reader.Set(f.Controller+cfgprobe::ControllerPawn,f.State);};
        if(scenario==6)f.Name(f.Picked,"DifferentPick");
        if(scenario==7)f.reader.Set(f.Picked+0x38,std::uint64_t{0});
        if(scenario==8)f.reader.Set(f.Picked+0x44,int{0x848});
        if(scenario==9)f.reader.Set(f.Selected+cfgprobe::ClassSuper,f.Selected);
        if(scenario==10)f.reader.Set(f.Registry+24+16,int{0});
        if(scenario==11)f.reader.Set(f.Picked+0x28,f.Picked);
        if(scenario==12)resolver.during=[&](){f.reader.Set(f.PSClass+0x38,std::uintptr_t{0});};
        if(scenario==13)resolver.selected=0;
        const bool ok=sp::ResolvePicked(f.reader,observed.owner,scenario==14?43:42,resolver,selected);
        check(!ok&&!selected.pickedResolutionVerified&&!selected.mode,"invalid/changed/unloaded/noncounselor selection refuses with empty output");
        if(scenario==0||scenario==6||scenario==7||scenario==8||scenario==10||scenario==11||scenario==14)check(!resolver.calls,"invalid prerequisites never invoke resolver");
        check(resolver.calls<=1,"no resolver retries/default selection");
    }
    {StartingFixture f;sp::Observation observed;f.reader.changeAt=f.Function+0xB0;f.reader.changeSlot=f.Registry+24;f.reader.newPointer=f.State;
        check(!sp::Observe(f.reader,f.event,observed),"owner replacement during metadata read refuses");}
    std::printf("CFG starting-player producer: %s (%u checks, %u failures). Read-only owned fixtures; no game/native calls.\n",failures?"FAIL":"PASS",checks,failures);return failures?1:0;
}
#endif
