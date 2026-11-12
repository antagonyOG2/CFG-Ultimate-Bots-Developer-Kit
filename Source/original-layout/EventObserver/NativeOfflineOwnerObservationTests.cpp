#define CFG_UNVERIFIED_RUNTIME_FIXTURE
#include "NativeOfflineOwnerObservation.hpp"
#include "OwnedByteReader.hpp"
#include <cstdio>
namespace o=cfgmenu::offlineowner;
struct StageReader:OwnedByteReader {
    std::uintptr_t watch=0,mutate=0,value=0;unsigned hits=0;
    template<class T>bool Read(std::uintptr_t at,T& out){
        if(watch&&at==watch&&++hits==2)Set(mutate,value);
        return OwnedByteReader::Read(at,out);
    }
};
struct Fixture {
    static constexpr std::uintptr_t Module=0x100000,Engine=0x200000,Viewport=0x210000,World=0x220000,Instance=0x230000,
        Players=0x240000,Player=0x250000,Controller=0x260000,Mode=0x270000,PS=0x280000,Registry=0x290000,
        Level=0x300000,Pawn=0x310000,ModeType=0x320000,ControllerType=0x330000,StateType=0x340000,LevelType=0x350000,
        Counselor=0x360000,Killer=0x370000,Selected=0x380000,Package=0x390000,Names=0x400000,Chunk=0x410000;
    StageReader r;cfgmenu::freshgame::Source source{Module,Engine,Viewport,Player,Instance,0x900000,42};
    cfgmenu::promotion::Packet picked{};
    Fixture(){using namespace cfgprobe;namespace d=cfgmenu::offlinelifecycledisk;namespace i=cfgmenu::intrometadatadisk;
        r.Set(Module+EngineGlobalCandidate,Engine);r.Set(Engine+EngineGameViewport,Viewport);r.Set(Viewport+ViewportWorld,World);
        r.Set(World+WorldNetDriver,std::uintptr_t{0});r.Set(World+WorldGameMode,Mode);r.Set(World+WorldGameInstance,Instance);
        r.Set(Instance+GameInstanceLocalPlayers,Players);r.Set(Instance+GameInstanceLocalPlayers+8,int{1});r.Set(Instance+GameInstanceLocalPlayers+12,int{1});
        r.Set(Players,Player);r.Set(Player+PlayerController,Controller);r.Set(Controller+ControllerPlayer,Player);
        r.Set(Controller+0x388,PS);r.Set(PS+0x88,Controller);r.Set(PS+0x394,std::uint32_t{0});r.Set(PS+0x110,std::uint8_t{3});
        r.Set(Module+dispatch::ThreadIdRva,std::uint32_t{42});r.Set(Module+dispatch::ThreadFlagRva,std::uint8_t{1});
        r.Set(Module+0x03199662,std::uint8_t{0});r.Set(Module+0x03198110,std::uint32_t{0});
        r.Set(Module+lifetime::ObjectArrayRva+16,Registry);r.Set(Module+lifetime::ObjectArrayRva+28,int{6});
        const std::array<std::uintptr_t,6> objects{{World,Mode,Controller,PS,Level,Pawn}};
        for(unsigned n=0;n<objects.size();++n){r.Set(objects[n]+lifetime::ObjectIndexOffset,static_cast<int>(n));r.Set(Registry+24*n,objects[n]);r.Set(Registry+24*n+16,100+static_cast<int>(n));}
        const auto type=[&](std::uintptr_t object,std::uintptr_t klass,std::uintptr_t cache){r.Set(object+ObjectClass,klass);r.Set(Module+cache,klass);r.Set(Module+cache+8,klass);
            r.Set(klass+0xF8,klass+0x1000);r.Set(klass+0x1000+ObjectClass,klass);};
        type(Mode,ModeType,d::OfflineBotsClass);type(Controller,ControllerType,i::SCPlayerController_OfflineBotsClass);
        type(PS,StateType,i::SCPlayerState_OfflineBotsClass);type(Level,LevelType,cfgkill::disk::LevelReflection);
        r.Set(Mode,Module+d::OfflineBotsVtable);r.Set(Mode+0x20,Level);r.Set(Level+cfgkill::disk::OwningWorld,World);
        struct Levels {std::uintptr_t data;int count,capacity;};
        r.Set(World+0x110,Levels{0x600000,1,1});r.Set(std::uintptr_t{0x600000},Level);
        // Prebirth deliberately has no readable Pawn/AcknowledgedPawn fields.
        r.Set(Module+cfgmenu::birthdisk::SCCounselorCharacterClass,Counselor);r.Set(Module+cfgmenu::birthdisk::SCCounselorCharacterConstructor,Counselor);
        r.Set(Module+cfgmenu::birthdisk::SCKillerCharacterClass,Killer);r.Set(Module+cfgmenu::birthdisk::SCKillerCharacterConstructor,Killer);
        r.Set(Selected+ClassSuper,Counselor);r.Set(Counselor+ClassSuper,std::uintptr_t{0});r.Set(Killer+ClassSuper,std::uintptr_t{0});
        r.Set(Selected+0xF8,Selected+0x1000);r.Set(Selected+0x1000+ObjectClass,Selected);
        r.Set(Module+cfgnames::disk::Global,Names);r.Set(Names,Chunk);r.Set(Names+cfgnames::disk::ElementCount,std::array<int,2>{{4,1}});
        Name(1,"AJ_C");Name(2,"/Game/Counselors/AJ");Name(3,"/Game/Counselors/AJ.AJ_C");
        r.Set(Selected+cfgnames::disk::ObjectName,cfgnames::Words{1,0});r.Set(Selected+0x20,Package);
        r.Set(Package+cfgnames::disk::ObjectName,cfgnames::Words{2,0});r.Set(Package+0x20,std::uintptr_t{0});
        cfgnames::Words path{3,0};std::memcpy(picked.bytes.data()+16,&path,8);
    }
    void Name(unsigned n,const char* name){const auto entry=std::uintptr_t{0x500000}+n*0x1000;std::array<char,128> text{};
        std::memcpy(text.data(),name,std::strlen(name));r.Set(Chunk+8*n,entry);r.Set(entry,std::uint32_t{0});r.Set(entry+cfgnames::disk::EntryText,text);}
    void Birth(){using namespace cfgprobe;r.Set(Controller+ControllerPawn,Pawn);r.Set(Controller+ControllerAcknowledgedPawn,Pawn);
        r.Set(Pawn+0x3A0,Controller);r.Set(Pawn+0x388,PS);r.Set(Pawn+ObjectClass,Selected);
        r.Set(PS+cfgmenu::birthdisk::SpawnedCharacterClass,Selected);r.Set(PS+cfgmenu::birthdisk::ActiveCharacterClass,picked);}
    bool Observe(cfgmenu::promotion::Context& c,unsigned* reads=nullptr){return o::Observe(r,source,Mode,42,true,c,reads);}
};
#ifndef CFG_OFFLINE_OWNER_FIXTURE_ONLY
int main(){unsigned failures=0,checks=0;const auto check=[&](bool okay,const char* name){++checks;if(!okay){++failures;std::printf("FAIL %s\n",name);}};
    using namespace cfgprobe;
    {Fixture f;cfgmenu::promotion::Context c;unsigned reads=0;check(f.Observe(c,&reads),"prebirth owner needs no readable pawn");
     check(c.mode==f.Mode&&c.playerState==f.PS&&!c.selectedClass&&!c.pickedResolutionVerified,"owner output contains no invented selection");
     check(reads<o::ReadLimit,"bounded owner reads");std::printf("Owner reads=%u\n",reads);}
    for(unsigned fault=0;fault<19;++fault){Fixture f;cfgmenu::promotion::Context c;
        if(fault==0)f.source.frontendWorld=f.World;
        if(fault==1)f.r.Set(f.World+WorldNetDriver,f.Mode);
        if(fault==2)f.r.Set(f.World+WorldGameMode,f.PS);
        if(fault==3)f.r.Set(f.Instance+GameInstanceLocalPlayers+8,int{2});
        if(fault==4)f.r.Set(f.Controller+ControllerPlayer,f.Mode);
        if(fault==5)f.r.Set(f.PS+0x88,f.Player);
        if(fault==6)f.r.Set(f.Registry+24*3+16,int{0});
        if(fault==7)f.r.Set(f.Controller+ObjectClass,f.ModeType);
        if(fault==8)f.r.Set(f.PS+ObjectClass,f.ControllerType);
        if(fault==9)f.r.Set(f.ModeType+0xF8,f.Mode);
        if(fault==10)f.r.Set(f.ControllerType+0xF8,f.Controller);
        if(fault==11)f.r.Set(f.StateType+0xF8,f.PS);
        if(fault==12)f.r.Set(f.Level+cfgkill::disk::OwningWorld,f.source.frontendWorld);
        if(fault==13)f.r.Set(f.Registry+24*4+16,int{-1});
        if(fault==14)f.r.Set(f.Mode,f.Module+cfgmenu::offlinelifecycledisk::SandboxVtable);
        if(fault==15)f.r.Set(f.PS+0x110,std::uint8_t{1});
        if(fault==16)f.r.Set(f.PS+0x394,std::uint32_t{4});
        if(fault==17){f.r.changeAt=f.Level+cfgkill::disk::OwningWorld;f.r.changeSlot=f.Viewport+ViewportWorld;f.r.newPointer=f.source.frontendWorld;}
        if(fault==18)f.r.Set(f.Module+cfgmenu::intrometadatadisk::SCPlayerState_OfflineBotsClass+8,f.Killer);
        check(!f.Observe(c)&&!c.mode,"wrong or changing native owner refuses without partial output");
    }
    for(unsigned fault=0;fault<7;++fault){Fixture f;cfgmenu::promotion::Context c;
        f.r.Set(f.Registry+24*4+16,int{0});
        if(fault==1)f.r.Set(std::uintptr_t{0x600000},f.Pawn);
        if(fault==2)f.r.Set(f.Level+ObjectClass,f.ModeType);
        if(fault==3){f.r.watch=f.World+0x110;f.r.mutate=0x600000;f.r.value=f.Pawn;}
        if(fault==4){f.r.changeAt=f.Level+cfgkill::disk::OwningWorld;f.r.changeSlot=f.Registry+24*4;f.r.newPointer=f.Pawn;}
        if(fault==5){f.r.watch=f.World+0x110;f.r.mutate=f.Registry+24*4;f.r.value=f.Pawn;}
        if(fault==6){f.r.watch=f.World+0x110;f.r.mutate=f.Level+ObjectClass;f.r.value=f.ModeType;}
        check(f.Observe(c)==(fault==0),"unassigned Level requires stable strong world membership, exact type and slot");
    }
    for(unsigned fault=0;fault<11;++fault){Fixture f;cfgmenu::promotion::Context c;check(f.Observe(c),"fresh prebirth fixture");f.Birth();
        if(fault==1)f.r.Set(f.Controller+ControllerAcknowledgedPawn,f.PS);
        if(fault==2)f.r.Set(f.Pawn+0x3A0,f.Player);
        if(fault==3)f.r.Set(f.Pawn+0x388,f.Mode);
        if(fault==4)f.r.Set(f.PS+cfgmenu::birthdisk::SpawnedCharacterClass,f.Killer);
        if(fault==5)f.r.Set(f.PS+cfgmenu::birthdisk::ActiveCharacterClass,cfgmenu::promotion::Packet{});
        if(fault==6)f.r.Set(f.Registry+24*5+16,int{0});
        if(fault==7)f.r.Set(f.Selected+0xF8,f.Pawn);
        if(fault==8)f.Name(1,"Vanessa_C");
        if(fault==9){f.r.changeAt=f.PS+cfgmenu::birthdisk::ActiveCharacterClass;f.r.changeSlot=f.Controller+ControllerPawn;f.r.newPointer=f.PS;}
        if(fault==10)f.picked.bytes[24]=1;
        o::Human h;unsigned reads=0;cfgmenu::NativeHumanDiagnostic diagnostic{};
        const bool okay=o::VerifyHuman(f.r,c,f.picked,f.Selected,42,h,&reads,&diagnostic);
        const unsigned expectedStages[]{0,9,10,11,13,14,6,14,4,8,1};
        check(diagnostic.stage==expectedStages[fault],"precise existing-read human refusal stage");
        check(diagnostic.reads==reads,"diagnostic preserves exact bounded read accounting");
        check(okay==(fault==0),"strict native human selected class/active/spawned/possession proof");
        check(reads<=o::ReadLimit,"bounded human reads");
        if(!fault){check(h.pawn==f.Pawn&&h.type==f.Selected&&h.identity.HasAssignedSerial(),"strong born human identity");std::printf("Human reads=%u\n",reads);}
        else check(!h.pawn,"failed human proof publishes no pawn");
    }
    for(unsigned fault=0;fault<7;++fault){Fixture f;cfgmenu::promotion::Context c;check(f.Observe(c),"empty-cache prebirth owner");f.Birth();
        f.r.Set(f.PS+cfgmenu::birthdisk::SpawnedCharacterClass,std::uintptr_t{0});
        if(fault==1)f.r.Set(f.PS+cfgmenu::birthdisk::ActiveCharacterClass,cfgmenu::promotion::Packet{});
        if(fault==2)f.r.Set(f.Pawn+ObjectClass,f.Killer);
        if(fault==3)f.r.Set(f.Controller+ControllerAcknowledgedPawn,f.PS);
        if(fault>=4){f.r.watch=f.PS+cfgmenu::birthdisk::SpawnedCharacterClass;f.r.mutate=f.r.watch;
            f.r.value=fault==4?f.Selected:f.Killer;}
        if(fault==6){f.r.Set(f.PS+cfgmenu::birthdisk::SpawnedCharacterClass,f.Selected);f.r.value=0;}
        o::Human h;check(o::VerifyHuman(f.r,c,f.picked,f.Selected,42,h)==(fault==0),"empty cache requires exact selected active pawn and stable possession/cache");
        check((h.pawn!=0)==(fault==0),"empty or changing cache never publishes partial human");
    }
    {Fixture f;cfgmenu::promotion::Context c;check(!o::Observe(f.r,f.source,f.Mode,41,true,c),"actual caller thread required");check(!o::Observe(f.r,f.source,f.Mode,42,false,c),"exact image required");}
    {Fixture f;cfgmenu::promotion::Context c;f.r.Set(f.Module+lifetime::ObjectArrayRva+28,int{5});check(f.Observe(c),"prebirth short object array");
     f.r.Set(f.Module+lifetime::ObjectArrayRva+28,int{6});f.Birth();o::Human h;
     check(o::Current(f.r,c,42)&&o::VerifyHuman(f.r,c,f.picked,f.Selected,42,h),"growth retains owners and admits newly spawned pawn beyond old count");
     f.r.Set(f.Registry+24*3+16,int{999});check(!o::Current(f.r,c,42),"growth never admits replaced retained serial");}
    {Fixture f;cfgmenu::promotion::Context c;check(f.Observe(c),"GC fixture");f.r.Set(f.Module+0x03198110,std::uint32_t{1});check(!o::Current(f.r,c,42),"GC refuses lookup");
     f.r.Set(f.Module+0x03198110,std::uint32_t{0});f.r.Set(f.Module+0x03199662,std::uint8_t{1});check(!o::Current(f.r,c,42),"serialization refuses lookup");}
    std::printf("Native OfflineBots owner/human provider: %u checks, %u failures\n",checks,failures);return failures?1:0;
}
#endif
