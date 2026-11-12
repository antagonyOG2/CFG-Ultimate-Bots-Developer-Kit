#define CFG_FRONTEND_BUNDLE
#define CFGMENU_MAP_FIXTURE_ONLY
#include "FreshWorldMapObservationTests.cpp"
#include "ObservedMenuArrival.hpp"
#include "NicoteroCatalogueFixture.hpp"
struct ArrivalFixture:MapFixture {
    static constexpr std::uintptr_t GameState=0x600000,GameStateType=0x610000,NativeMode=0x620000,
        JasonType=0x630000,JasonPackage=0x640000,Tree=0x650000,TreeType=0x660000,Catalogue=0x670000,PathName=0x680000;
    cfgmenu::Session session;cfgmenu::GameplayHandoff handoff{session};
    cfgmenu::freshgame::Source source{Module,Engine,Viewport,Player,Instance,0x900000,42};
    cfgmenu::MenuCatalogueEvidence menu;cfgprimary::SoftClassBytes freshJason{};
    ArrivalFixture(){using namespace cfgprobe;
        plan.objectArray.count=10;reader.Set(Module+lifetime::ObjectArrayRva+28,int{10});
        for(unsigned i=0;i<4;++i)reader.Set(Registry+24*i+8,std::uint32_t{0});
        const auto object=[&](std::uintptr_t at,int index,std::uintptr_t type){
            reader.Set(at+ObjectClass,type);reader.Set(at+lifetime::ObjectIndexOffset,index);
            reader.Set(Registry+24*index,at);reader.Set(Registry+24*index+8,std::uint32_t{0});reader.Set(Registry+24*index+16,100+index);
        };
        object(GameState,8,GameStateType);object(Tree,9,TreeType);
        reader.Set(World+WorldGameState,GameState);reader.Set(Module+cfgprimary::ModeReflection,NativeMode);
        reader.Set(Module+cfgprimary::ModeConstructor,NativeMode);reader.Set(SandboxType+ClassSuper,NativeMode);
        reader.Set(NativeMode+ClassSuper,std::uintptr_t{0});reader.Set(GameStateType+ClassSuper,std::uintptr_t{0});
        reader.Set(Module+cfgprimary::StateReflection,GameStateType);reader.Set(Module+cfgprimary::StateConstructor,GameStateType);
        const auto& playerState=cfgspawn::Classes[cfgspawn::PlayerState];const auto& tree=cfgspawn::Classes[cfgspawn::Tree];
        reader.Set(Module+playerState.reflection,StateType);reader.Set(Module+playerState.constructor,StateType);
        reader.Set(Module+tree.reflection,TreeType);reader.Set(Module+tree.constructor,TreeType);reader.Set(TreeType+ClassSuper,std::uintptr_t{0});
        reader.Set(Module+cfgprimary::SerializationFlag,std::uint8_t{0});reader.Set(Module+cfgprimary::GarbageCollectionCounter,std::uint32_t{0});
        reader.Set(GameState+cfgprimary::CurrentKiller,std::uintptr_t{0});reader.Set(GameState+cfgprimary::CurrentKillerState,std::uintptr_t{0});
        reader.Set(GameState+cfgprimary::SelectedKiller,cfgprimary::SoftClassBytes{});reader.Set(Mode+cfgprimary::CounselorTree,Tree);
        const std::array<std::uintptr_t,3> types{{SandboxType,ControllerType,PawnType}};
        for(unsigned i=0;i<types.size();++i){const auto defaults=std::uintptr_t{0xD00000}+i*0x10000;
            reader.Set(types[i]+0xF8,defaults);reader.Set(defaults+ObjectClass,types[i]);}
        reader.Set(JasonType+ClassSuper,KillerType);reader.Set(JasonType+0x20,JasonPackage);reader.Set(JasonPackage+0x20,std::uintptr_t{0});
        Name(JasonType,"Jason_C");Name(JasonPackage,"/Game/Jasons/Jason");Name(PathName,"/Game/Jasons/Jason.Jason_C");
        cfgnames::Words path;reader.Read(PathName+cfgnames::disk::ObjectName,path);std::memcpy(freshJason.data()+16,&path,8);
        freshJason[0]=37; // Native fresh weak cache differs from the armed menu value.
        CatalogueCount(1);menu.frontend=Widget;menu.frontendWorld=source.frontendWorld;menu.catalogueGeneration=9;
        menu.map=Definition;menu.jason=JasonType;menu.maximumBots=8;menu.selectedJason=freshJason;menu.selectedJason[0]=12;
        menu.cfgMetadata=menu.ownedSelections=menu.preserveNativeCounselor=true;
    }
    void CatalogueCount(unsigned count){reader.Set(Mode+cfgprimary::KillerClasses,cfgspawn::PlayerArray{Catalogue,static_cast<int>(count),static_cast<int>(count)});
        for(unsigned i=0;i<count;++i)reader.Set(Catalogue+40*i,i+1==count?freshJason:cfgprimary::SoftClassBytes{});}
    bool Arm(){cfgmenu::Capabilities caps;caps.verified.fill(true);caps.automaticJason=true;
        if(!session.Open(Widget,cfgmenu::Mode::Counselor,caps,9))return false;menu.generation=session.Generation();
        return session.PickNativeCustomization(Widget,menu.generation,Definition,JasonType)&&session.Commit(Widget,menu.generation)&&handoff.Arm(menu);}
};
int main(){unsigned checks=0,failures=0;const auto check=[&](bool ok,const char* label){++checks;if(!ok){++failures;std::printf("FAIL %s\n",label);}};
    {ArrivalFixture gate;const auto before=gate.reader.reads;
     check(!cfgprimary::RuntimeLookupEligibilityVerified&&!cfgprimary::LookupSafe(gate.reader,gate.Module)&&
        gate.reader.reads==before,"read-only arrival tests run while general native execution remains disabled");}
    for(unsigned veto=0;veto<2;++veto){ArrivalFixture blocked;check(blocked.Arm(),"veto arrival armed");
        if(veto)blocked.reader.Set(blocked.Module+cfgprimary::GarbageCollectionCounter,std::uint32_t{1});
        else blocked.reader.Set(blocked.Module+cfgprimary::SerializationFlag,std::uint8_t{1});
        cfgmenu::observedarrival::Observation out;
        check(!cfgmenu::observedarrival::Consume(blocked.reader,blocked.handoff,blocked.source,42,true,out)&&
            !out.configuration.submission,"observation still refuses serialization and GC, without publishing configuration");
    }
    using namespace cfgmenu;namespace oa=observedarrival;
    for(unsigned variant=0;variant<4;++variant){ArrivalFixture f;
        if(variant==1)f.CatalogueCount(64);
        if(variant==2){f.Name(f.Package,"/Game/Maps/Lake_Snow");f.Name(f.World,"Lake_Snow");}
        if(variant==3)f.reader.Set(f.GameState+cfgprimary::SelectedKiller,f.freshJason);
        check(f.Arm(),"native menu submission armed");oa::Observation out;unsigned reads=0;oa::Failure failure;
        const auto accepted=oa::Consume(f.reader,f.handoff,f.source,42,true,out,&reads,&failure);
        if(!accepted)std::printf("Arrival variant=%u failure=%u reads=%u\n",variant,static_cast<unsigned>(failure),reads);
        check(accepted,"concrete birth/map/roster/native-path chain produces one configuration");
        check(out.plan.pawn==f.Pawn&&!out.plan.frontend.widget&&out.configuration.submission==1&&out.configuration.counselorBots==6,
            "owned counselor retained; total seven means six helpers, no HUD/pawn substitution");
        check(out.configuration.jasonCatalogueIndex==(variant==1?63u:0u)&&out.configuration.selectedJason==f.freshJason&&
            out.mapVariant==(variant==2?worldmap::Variant::Snow:worldmap::Variant::Normal),"fresh actual catalogue packet and normal/snow world, not cached menu bytes");
        check(reads<=oa::ReadLimit&&failure==oa::Failure::None&&f.handoff.State()==HandoffState::Consumed,"one bounded accepted handoff");
        if(!variant)std::printf("Concrete observed native arrival reads=%u\n",reads);
        const auto before=f.reader.reads;
        check(!oa::Consume(f.reader,f.handoff,f.source,42,true,out,&reads)&&!reads&&f.reader.reads==before&&!out.configuration.submission,
            "duplicate has zero reads and no stale configuration");
    }
    // Production used a selected counselor, while earlier arrival fixtures all
    // used the deferred/unset choice. Cover explicit choice and native variant.
    for(unsigned variant=0;variant<4;++variant){ArrivalFixture f;
        constexpr std::uintptr_t chosen=0xE50000;
        f.menu.preserveNativeCounselor=false;f.menu.counselor=variant==0?f.PawnType:chosen;
        if(variant){f.reader.Set(chosen+cfgprobe::ClassSuper,f.PawnType);}
        if(variant==1){f.reader.Set(f.PawnType+cfgprobe::ClassSuper,chosen);f.reader.Set(chosen+cfgprobe::ClassSuper,std::uintptr_t{0});}
        if(variant==3)f.reader.Set(f.State+cfgspawn::ActorOwner,f.Player);
        Capabilities caps;caps.verified.fill(true);caps.automaticJason=true;
        check(f.session.Open(f.Widget,Mode::Counselor,caps,9),"explicit counselor session");f.menu.generation=f.session.Generation();
        check(f.session.Pick(f.Widget,f.menu.generation,f.Definition,f.JasonType,f.menu.counselor)&&f.session.Commit(f.Widget,f.menu.generation)&&f.handoff.Arm(f.menu),"explicit native choice armed");
        oa::Observation out;arrival::Trace trace;const bool okay=oa::Consume(f.reader,f.handoff,f.source,42,true,out,nullptr,nullptr,nullptr,&trace);
        check(okay==(variant<2),"exact or native-derived chosen counselor accepted, unrelated/invalid ownership refused");
        if(variant<2)check(trace.actual==f.PawnType&&trace.expected==f.menu.counselor&&trace.relationship==(variant?3u:2u)&&out.configuration.counselorBots==6,"chosen native counselor stays owned and reaches AI configuration");
    }
    for(unsigned fault=0;fault<4;++fault){ArrivalFixture f;
        constexpr std::uintptr_t package=0xE00000,pathObject=0xE10000;
        f.Name(f.PawnType,"AJ_C");f.Name(package,"/Game/Counselors/AJ");f.Name(pathObject,"/Game/Counselors/AJ.AJ_C");
        f.reader.Set(f.PawnType+0x20,package);f.reader.Set(package+0x20,std::uintptr_t{0});
        cfgnames::Words path;f.reader.Read(pathObject+cfgnames::disk::ObjectName,path);
        std::memcpy(f.menu.selectedCounselor.data()+16,&path,8);f.menu.preserveNativeCounselor=false;f.menu.counselorPathSelection=true;
        f.menu.counselor=softpath::Token(f.menu.selectedCounselor);Capabilities caps;caps.verified.fill(true);caps.automaticJason=true;
        check(f.session.Open(f.Widget,Mode::Counselor,caps,9),"accepted native path session");f.menu.generation=f.session.Generation();
        check(f.session.Pick(f.Widget,f.menu.generation,f.Definition,f.JasonType,f.menu.counselor)&&f.session.Commit(f.Widget,f.menu.generation)&&f.handoff.Arm(f.menu),"accepted row token arms without stale Customize class");
        if(fault==1)f.Name(f.PawnType,"Vanessa_C");
        if(fault==2)f.Name(package,"/Game/Counselors/Vanessa");
        if(fault==3)f.reader.Set(f.Controller+cfgprobe::ControllerAcknowledgedPawn,f.State);
        oa::Observation out;arrival::Trace trace;
        check(oa::Consume(f.reader,f.handoff,f.source,42,true,out,nullptr,nullptr,nullptr,&trace)==(fault==0),"fresh owned born class must match accepted full path, not profile or guessed counselor");
        if(!fault)check(trace.relationship==5&&out.configuration.counselorBots==6,"accepted AJ path reaches actual helper spawn configuration");
    }
    // Accepted row names the counselor asset, while a native wardrobe variant
    // may be the actual born class. This preserves the old pointer branch's
    // derived-class semantics using ONLY fresh stable ancestry + full paths.
    for(unsigned fault=0;fault<9;++fault){ArrivalFixture f;
        constexpr std::uintptr_t chosen=0xE50000,base=0xE60000,package=0xE70000,skinPackage=0xE80000,pathObject=0xE90000;
        constexpr std::uintptr_t middle=0xEA0000,middlePackage=0xEB0000;
        const auto& native=cfgprobe::NativeClasses[cfgprobe::Counselor];
        f.reader.Set(f.Module+native.reflectionCacheRva,base);f.reader.Set(f.Module+native.constructorCacheRva,base);
        f.reader.Set(base+cfgprobe::ClassSuper,std::uintptr_t{0});f.reader.Set(chosen+cfgprobe::ClassSuper,base);
        f.reader.Set(f.PawnType+cfgprobe::ClassSuper,chosen);
        f.Name(chosen,"Rocker_Counselor_C");f.Name(package,"/Game/Characters/Counselors/Rocker/Rocker_Counselor");
        f.Name(f.PawnType,"Rocker_Wardrobe_C");f.Name(skinPackage,"/Game/Characters/Counselors/Rocker/Rocker_Wardrobe");
        f.reader.Set(chosen+0x20,package);f.reader.Set(package+0x20,std::uintptr_t{0});
        f.reader.Set(f.PawnType+0x20,skinPackage);f.reader.Set(skinPackage+0x20,std::uintptr_t{0});
        f.Name(pathObject,"/Game/Characters/Counselors/Rocker/Rocker_Counselor.Rocker_Counselor_C");
        cfgnames::Words path;f.reader.Read(pathObject+cfgnames::disk::ObjectName,path);
        std::memcpy(f.menu.selectedCounselor.data()+16,&path,8);f.menu.preserveNativeCounselor=false;f.menu.counselorPathSelection=true;
        f.menu.counselor=softpath::Token(f.menu.selectedCounselor);Capabilities caps;caps.verified.fill(true);caps.automaticJason=true;
        check(f.session.Open(f.Widget,Mode::Counselor,caps,9),"accepted native derived path session");f.menu.generation=f.session.Generation();
        check(f.session.Pick(f.Widget,f.menu.generation,f.Definition,f.JasonType,f.menu.counselor)&&f.session.Commit(f.Widget,f.menu.generation)&&f.handoff.Arm(f.menu),"derived counselor path armed without menu class pointer");
        if(fault==1)f.Name(chosen,"Other_Counselor_C");
        if(fault==2)f.Name(package,"/Game/Other/Rocker_Counselor");
        if(fault==3)f.reader.Set(f.Controller+cfgprobe::ControllerAcknowledgedPawn,f.State);
        if(fault==4)f.reader.Set(chosen+cfgprobe::ClassSuper,f.PawnType);
        if(fault==5){f.reader.changeAt=chosen+cfgnames::disk::ObjectName;f.reader.changeSlot=f.PawnType+cfgprobe::ClassSuper;f.reader.newPointer=base;}
        if(fault==6){f.Name(f.PawnType,"Rocker_Counselor_C");f.reader.Set(f.PawnType+0x20,package);}
        if(fault==7)f.reader.Set(f.PawnType+cfgprobe::ClassSuper,base); // Selected sibling is not an ancestor.
        if(fault==8){f.reader.Set(f.PawnType+cfgprobe::ClassSuper,middle);f.reader.Set(middle+cfgprobe::ClassSuper,chosen);
            f.Name(middle,"Rocker_Variant_C");f.Name(middlePackage,"/Game/Characters/Counselors/Rocker/Rocker_Variant");
            f.reader.Set(middle+0x20,middlePackage);f.reader.Set(middlePackage+0x20,std::uintptr_t{0});}
        if(fault==0||fault==8){unsigned depth=99;
            check(!classchoice::MatchesPath(f.reader,f.Module,f.PawnType,base,f.menu.selectedCounselor)&&
                classchoice::MatchesPathOrAncestor(f.reader,f.Module,f.PawnType,base,f.menu.selectedCounselor,&depth)&&depth==(fault==8?2u:1u),
                "exact-only regression reproduced; exact full accepted path exists on verified native ancestor");}
        oa::Observation out;arrival::Trace trace;unsigned reads=0;
        const bool okay=oa::Consume(f.reader,f.handoff,f.source,42,true,out,&reads,nullptr,nullptr,&trace);
        check(okay==(fault==0||fault==8),"native derived accepted path only; wrong name/package, owner, cycle, changing ancestry, duplicate or sibling refuse");
        check(reads<=oa::ReadLimit,"derived-path arrival stays within original aggregate read budget");
        if(okay)check(trace.relationship==5&&out.plan.pawn==f.Pawn&&out.configuration.counselorBots==6,
            "fresh owned native variant reaches helper configuration without pawn/class repair");
    }
    for(unsigned fault=0;fault<8;++fault){
        ArrivalFixture f;f.reader.Set(f.State+selectiondisk::SpawnedCharacterClass,std::uintptr_t{0});
        f.reader.Set(f.Module+birthdisk::SCPlayerController_SandboxClass,f.ControllerType);
        f.reader.Set(f.Module+birthdisk::SCPlayerController_SandboxConstructor,f.ControllerType);
        constexpr std::uintptr_t counselorPackage=0xE00000,activePath=0xE10000;
        f.Name(f.PawnType,"Counselor_C");f.Name(counselorPackage,"/Game/Counselors/Counselor");f.Name(activePath,"/Game/Counselors/Counselor.Counselor_C");
        f.reader.Set(f.PawnType+0x20,counselorPackage);f.reader.Set(counselorPackage+0x20,std::uintptr_t{0});
        std::array<unsigned char,40> active{};const std::int32_t weakIndex=-1;std::memcpy(active.data(),&weakIndex,4);
        cfgnames::Words path;f.reader.Read(activePath+cfgnames::disk::ObjectName,path);std::memcpy(active.data()+16,&path,8);
        if(fault==1)f.Name(f.PawnType,"Different_C");
        if(fault==2)active[24]=1;
        if(fault==3)f.reader.Set(f.Module+birthdisk::SCPlayerController_SandboxConstructor,f.KillerType);
        if(fault==4)f.reader.Set(f.State+selectiondisk::SpawnedCharacterClass,f.KillerType);
        if(fault==5)f.reader.Set(f.State+cfgspawn::ActorOwner,f.Player);
        f.reader.Set(f.State+selectiondisk::ActiveCharacterClass,active);
        cfgprimary::SoftClassBytes nativeEmpty{};nativeEmpty.fill(0);nativeEmpty[0]=nativeEmpty[1]=nativeEmpty[2]=nativeEmpty[3]=0xFF;
        f.reader.Set(f.GameState+cfgprimary::SelectedKiller,nativeEmpty);
        if(fault==6){f.reader.changeAt=f.PawnType+cfgnames::disk::ObjectName;f.reader.changeSlot=f.Controller+cfgprobe::ControllerPawn;f.reader.newPointer=f.State;}
        if(fault==7){f.reader.changeAt=f.State+selectiondisk::ActiveCharacterClass;f.reader.changeSlot=f.State+selectiondisk::SpawnedCharacterClass;f.reader.newPointer=f.PawnType;}
        check(f.Arm(),"first native Sandbox route armed");oa::Observation out;freshgame::Trace trace;
        check(oa::Consume(f.reader,f.handoff,f.source,42,true,out,nullptr,nullptr,&trace)==(fault==0),"missing Spawned needs exact live state-owned full counselor path/controller proof");
        if(!fault){check(out.configuration.counselorBots==6&&out.plan.pawn==f.Pawn&&trace.human.spawned==0&&trace.human.activeNonempty&&trace.human.nativeActiveVerified,"native selected counselor reaches helper configuration without default or possession repair");
            check(trace.human.reads<birth::ReadLimit,"full active-path proof remains within nested birth cap");std::printf("Native first-birth full active-path proof reads=%u\n",trace.human.reads);}
    }
    for(unsigned fault=0;fault<22;++fault){ArrivalFixture f;check(f.Arm(),"fault submission armed");
        if(fault==0)f.source.frontendWorld=f.World;
        if(fault==1)f.reader.Set(f.Controller+cfgprobe::ControllerPawn,f.State);
        if(fault==2)f.reader.Set(f.World+cfgprobe::WorldNetDriver,f.State);
        if(fault==3)f.reader.Set(f.State+selectiondisk::SpawnedCharacterClass,f.KillerType);
        if(fault==4)f.Name(f.Package,"/Game/Maps/Other");
        if(fault==5)f.reader.Set(f.Mode+cfgprimary::KillerClasses,cfgspawn::PlayerArray{f.Catalogue,65,65});
        if(fault==6)f.CatalogueCount(0);
        if(fault==7){auto packet=f.freshJason;packet[24]=1;f.reader.Set(f.Catalogue,packet);}
        if(fault==8){f.CatalogueCount(2);f.reader.Set(f.Catalogue,f.freshJason);}
        if(fault==9)f.Name(f.JasonType,"DifferentJason_C");
        if(fault==10)f.reader.Set(f.GameState+cfgprimary::CurrentKiller,f.Pawn);
        if(fault==11)f.reader.Set(f.Mode+cfgprimary::CounselorTree,std::uintptr_t{0});
        if(fault==12)f.reader.Set(f.Module+cfgprimary::SerializationFlag,std::uint8_t{1});
        if(fault==13)f.reader.Set(f.Module+cfgprimary::GarbageCollectionCounter,std::uint32_t{1});
        if(fault==14){auto packet=f.freshJason;packet[16]=255;f.reader.Set(f.GameState+cfgprimary::SelectedKiller,packet);}
        if(fault==15)f.reader.Set(f.World+cfgprobe::WorldGameState,f.State);
        if(fault==16)f.reader.Set(f.Registry+24*8+16,int{0});
        if(fault==17)f.reader.Set(f.Module+cfgspawn::Classes[cfgspawn::Tree].constructor,f.StateType);
        if(fault==18){f.reader.changeAt=f.JasonType+cfgnames::disk::ObjectName;f.reader.changeSlot=f.Mode+cfgprimary::KillerClasses;f.reader.newPointer=f.Catalogue+40;}
        if(fault==19){f.reader.changeAt=f.JasonType+cfgnames::disk::ObjectName;f.reader.changeSlot=f.Controller+cfgprobe::ControllerPawn;f.reader.newPointer=f.State;}
        oa::Observation out;unsigned reads=0;oa::Failure failure;
        check(!oa::Consume(f.reader,f.handoff,f.source,fault==20?43:42,fault!=21,out,&reads,&failure),"bad/changed actual birth/map/roster refuses without fallback");
        check(!out.configuration.submission&&!out.plan.pawn&&failure!=oa::Failure::None&&reads<=oa::ReadLimit&&f.handoff.State()==HandoffState::Refused,
            "failure consumes arrival and publishes no partial result");
        const auto before=f.reader.reads;
        check(!oa::Consume(f.reader,f.handoff,f.source,42,true,out,&reads)&&!reads&&f.reader.reads==before,"failed route cannot silently retry into later world");
    }
    for(unsigned fault=0;fault<15;++fault){ArrivalFixture f;using namespace cfgprobe;namespace d=cfgmenu::intrometadatadisk;
        constexpr std::uintptr_t modeType=0xB00000,pcType=0xB10000,psType=0xB20000,gsType=0xB30000,level=0xB40000,levelType=0xB50000,package=0xB60000,path=0xB70000;
        const auto nativeType=[&](std::uintptr_t object,std::uintptr_t type,std::uintptr_t parent,std::uintptr_t cache){
            f.reader.Set(object+ObjectClass,type);f.reader.Set(type+ClassSuper,parent);f.reader.Set(type+0xF8,type+0x1000);f.reader.Set(type+0x1000+ObjectClass,type);
            f.reader.Set(f.Module+cache,type);f.reader.Set(f.Module+cache+8,type);};
        nativeType(f.Mode,modeType,f.NativeMode,NativeClasses[OfflineBotsMode].reflectionCacheRva);
        nativeType(f.Controller,pcType,f.ControllerType,d::SCPlayerController_OfflineBotsClass);
        nativeType(f.State,psType,f.StateType,d::SCPlayerState_OfflineBotsClass);
        nativeType(f.GameState,gsType,f.GameStateType,d::SCGameState_OfflineBotsClass);
        nativeType(level,levelType,0,cfgkill::disk::LevelReflection);
        f.reader.Set(f.Mode,f.Module+offlinelifecycledisk::OfflineBotsVtable);f.reader.Set(f.Mode+0x20,level);f.reader.Set(level+cfgkill::disk::OwningWorld,f.World);
        f.reader.Set(level+lifetime::ObjectIndexOffset,int{10});f.reader.Set(f.Registry+24*10,level);f.reader.Set(f.Registry+24*10+16,int{110});
        f.reader.Set(f.Module+lifetime::ObjectArrayRva+28,int{11});f.reader.Set(f.Mode+0x3C0,f.GameState);f.reader.Set(f.State+0x110,std::uint8_t{3});
        f.Name(f.PawnType,"AJ_C");f.Name(package,"/Game/Counselors/AJ");f.Name(path,"/Game/Counselors/AJ.AJ_C");
        f.reader.Set(f.PawnType+0x20,package);f.reader.Set(package+0x20,std::uintptr_t{0});
        std::array<unsigned char,40> active{};cfgnames::Words words;f.reader.Read(path+cfgnames::disk::ObjectName,words);std::memcpy(active.data()+16,&words,8);
        f.reader.Set(f.State+selectiondisk::ActiveCharacterClass,active);f.source.nativeOffline=true;
        if(fault==1)f.source.nativeOffline=false;
        if(fault==2)f.reader.Set(f.GameState+cfgprimary::CurrentKiller,f.Pawn);
        if(fault==3)f.reader.Set(f.GameState+cfgprimary::CurrentKillerState,f.State);
        if(fault==4)f.reader.Set(f.State+selectiondisk::ActiveCharacterClass,std::array<unsigned char,40>{});
        if(fault==5)f.reader.Set(f.State+selectiondisk::SpawnedCharacterClass,std::uintptr_t{0});
        if(fault==6)f.reader.Set(f.Controller+ObjectClass,f.ControllerType);
        if(fault==7)f.reader.Set(level+cfgkill::disk::OwningWorld,f.source.frontendWorld);
        if(fault==8)f.reader.Set(f.State+selectiondisk::SpawnedCharacterClass,f.KillerType);
        if(fault==9){f.reader.Set(f.State+selectiondisk::SpawnedCharacterClass,std::uintptr_t{0});f.reader.Set(f.State+selectiondisk::ActiveCharacterClass,std::array<unsigned char,40>{});}
        if(fault==10){f.reader.Set(f.State+selectiondisk::SpawnedCharacterClass,std::uintptr_t{0});
            f.reader.changeAt=f.State+selectiondisk::ActiveCharacterClass;f.reader.changeSlot=f.State+selectiondisk::SpawnedCharacterClass;f.reader.newPointer=f.PawnType;}
        if(fault>=11){
            f.reader.Set(f.State+selectiondisk::SpawnedCharacterClass,std::uintptr_t{0});
            f.reader.Set(f.Registry+24*10+16,int{0});
            struct Levels{std::uintptr_t data;int count,capacity;};
            constexpr std::uintptr_t levelList=0xF00000; // Separate from MapFixture::NormalStorage.
            f.reader.Set(f.World+0x110,Levels{levelList,1,1});f.reader.Set(levelList,level);
            if(fault==12)f.reader.Set(levelList,f.Pawn);
            if(fault==13)f.reader.Set(level+cfgkill::disk::OwningWorld,f.source.frontendWorld);
            if(fault==14){f.reader.changeAt=f.World+0x110;f.reader.changeSlot=f.Registry+24*10;f.reader.newPointer=f.Pawn;}
        }
        check(f.Arm(),"explicit native fixture armed");oa::Observation out;oa::Failure failure;freshgame::Trace trace;unsigned reads=0;
        const bool okay=oa::Consume(f.reader,f.handoff,f.source,42,true,out,&reads,&failure,&trace);
        if((fault==0||fault==5||fault==11)&&!okay)std::printf("Native arrival fault=%u failure=%u freshStep=%u birthStep=%u birthReads=%u reads=%u\n",fault,unsigned(failure),unsigned(trace.step),unsigned(trace.human.step),trace.human.reads,reads);
        check(okay==(fault==0||fault==5||fault==11),"explicit native family/restored human/pre-AI EmptyPrimary admission with optional class cache and strong Level");
        if(fault==0||fault==5||fault==11)check(out.nativeOffline&&out.configuration.submission==1,"native family recorded in successful arrival");
        else check(!out.configuration.submission,"native arrival refusal publishes no configuration");
    }
    for(unsigned variant=0;variant<4;++variant){ArrivalFixture f;using namespace cfgprobe;
        constexpr std::uintptr_t defaults=0xF20000;
        constexpr std::uintptr_t meta=ArrivalFixture::ClassType;
        f.reader.Set(f.Module+cfgcompat::cfg35::worldMap::NativeClassCache,meta);f.reader.Set(f.JasonType+ObjectClass,meta);f.reader.Set(meta+ClassSuper,std::uintptr_t{0});
        f.Name(f.JasonType,"Jason_UNI_C");f.Name(f.JasonPackage,"/Game/Characters/Killers/Jason/JUNI/Jason_UNI");
        f.reader.Set(f.Module+lifetime::ObjectArrayRva+28,12);
        f.reader.Set(f.JasonType+lifetime::ObjectIndexOffset,10);f.reader.Set(f.Registry+24*10,f.JasonType);f.reader.Set(f.Registry+24*10+16,707);
        f.reader.Set(defaults+lifetime::ObjectIndexOffset,11);f.reader.Set(f.Registry+24*11,defaults);f.reader.Set(f.Registry+24*11+16,0);
        f.reader.Set(f.Registry+24*10+8,std::uint32_t{0});f.reader.Set(f.Registry+24*11+8,std::uint32_t{0});
        f.reader.Set(f.JasonType+cfgspawn::ClassDefaultObject,defaults);f.reader.Set(defaults+ObjectClass,f.JasonType);
        f.reader.Set(f.JasonType+cfgcompat::cfg35::primaryJason::SelectedClassFlags,std::uint32_t{0});
        lifetime::ArrayHeader registry;check(lifetime::ReadArrayHeader(f.reader,f.Module+lifetime::ObjectArrayRva,registry),"fresh explicit registry");
        check(cfgselectedjason::Observe(f.reader,f.Module,registry,f.JasonType,f.KillerType,f.menu.selectedClass),"positive exact selected class proof");
        f.menu.selectedJason={};f.CatalogueCount(18);
        for(unsigned i=0;i<18;++i){const auto object=std::uintptr_t{0xF30000}+256*i;f.Name(object,NicoteroBaseRoster[i]);
            cfgprimary::SoftClassBytes packet{};cfgnames::Words words;f.reader.Read(object+cfgnames::disk::ObjectName,words);
            std::memcpy(packet.data()+16,&words,8);f.reader.Set(f.Catalogue+40*i,packet);}
        if(variant==1){f.Name(f.PathName,"/Game/Characters/Killers/Jason/JUNI/Jason_UNI.Jason_UNI_C");
            cfgnames::Words words;f.reader.Read(f.PathName+cfgnames::disk::ObjectName,words);cfgprimary::SoftClassBytes packet{};
            std::memcpy(packet.data()+16,&words,8);f.reader.Set(f.Catalogue+40*17,packet);}
        check(f.Arm(),"explicit Jason menu armed without fake catalogue index");
        if(variant==2)f.reader.Set(f.Registry+24*10+16,708);
        if(variant==3)f.reader.Set(f.JasonType+cfgcompat::cfg35::primaryJason::SelectedClassFlags,std::uint32_t{1});
        oa::Observation out;oa::Failure why;const bool okay=oa::Consume(f.reader,f.handoff,f.source,42,true,out,nullptr,&why);
        if(!okay&&variant<2)std::printf("Explicit arrival variant%u refused%u\n",variant,static_cast<unsigned>(why));
        check(okay==(variant<2),"exact18 plusJUNI arrival preserves current identity, rejects stale/abstract");
        if(okay)check(variant==0?(out.configuration.selectedClass.Valid()&&out.configuration.jasonCatalogueIndex==cfgselectedjason::NoCatalogueIndex):
            (out.configuration.selectedClass.Empty()&&out.configuration.jasonCatalogueIndex==17),"world catalogue preferred; explicit class used only when absent");
    }
    std::printf("CFG concrete observed menu arrival: %s (%u checks, %u failures). Owned read-only fixtures; not live integration.\n",failures?"FAIL":"PASS",checks,failures);
    return failures?1:0;
}
