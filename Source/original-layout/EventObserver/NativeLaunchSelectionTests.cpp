#define CFGMENU_CATALOGUE_FIXTURE_ONLY
#include "NativeFrontendCataloguesTests.cpp"
#include "../RuntimeProbe/NativeLaunchSelection.hpp"
#include "NativeSandboxStartHost.hpp"
#include "NativeMenuLaunchRuntime.hpp"
#include "NativeMenuDispatch.hpp"
struct LaunchFixture:CatalogueFixture {
    static constexpr std::uintptr_t PlayerType=0xA00000,SaveType=0xA10000,Save=0xA20000,SaveDefaults=0xA30000,
        Cache=0xA40000,SelectedJason=0xA50000,NativeJason=0xA60000,Package=0xA70000,SelectedCounselor=0xA80000,NativeCounselor=0xA90000;
    LaunchFixture(){using namespace cfgmenu;using namespace selectiondisk;using namespace cfgprobe;
        reader.Set(Controller+0x3D0,Player);reader.Set(Player+ObjectClass,PlayerType);reader.Set(PlayerType+ClassSuper,std::uintptr_t{0});
        reader.Set(Module+ILLLocalPlayerClass,PlayerType);reader.Set(Module+ILLLocalPlayerConstructor,PlayerType);
        reader.Set(Module+SCCharacterSelectionsSaveGameClass,SaveType);reader.Set(Module+SCCharacterSelectionsSaveGameConstructor,SaveType);
        reader.Set(Player+SettingsSaveGameCache,maps::Array{Cache,1,4});reader.Set(Cache,Save);
        reader.Set(Save+ObjectClass,SaveType);reader.Set(SaveType+ClassSuper,std::uintptr_t{0});reader.Set(SaveType+cfgspawn::ClassDefaultObject,SaveDefaults);
        reader.Set(Save+CounselorPick,std::uintptr_t{0});reader.Set(Save+KillerPick,SelectedJason);
        reader.Set(Module+lifetime::ObjectArrayRva+28,5);reader.Set(Save+lifetime::ObjectIndexOffset,4);
        reader.Set(Registry+24*4,Save);reader.Set(Registry+24*4+16,0); // Actual event-local save is not a retained serial identity.
        reader.Set(Module+cataloguedisk::SCKillerCharacterClass,NativeJason);reader.Set(Module+cataloguedisk::SCKillerCharacterConstructor,NativeJason);
        reader.Set(Module+cataloguedisk::SCCounselorCharacterClass,NativeCounselor);reader.Set(Module+cataloguedisk::SCCounselorCharacterConstructor,NativeCounselor);
        reader.Set(SelectedJason+ClassSuper,NativeJason);reader.Set(NativeJason+ClassSuper,std::uintptr_t{0});
        reader.Set(SelectedCounselor+ClassSuper,NativeCounselor);reader.Set(NativeCounselor+ClassSuper,std::uintptr_t{0});
        reader.Set(SelectedJason+0x20,Package);reader.Set(Package+0x20,std::uintptr_t{0});
        reader.Set(Names+cfgnames::disk::ElementCount,std::array<int,2>{{6,1}});
        Name(0xAA0000,2,u"/Game/Jasons/Chosen.Chosen_C");Name(Package,3,u"/Game/Jasons/Chosen");Name(SelectedJason,4,u"Chosen_C");
        // Frontend is NOT a gameplay roster owner. Read the constructed native
        // SCGameMode CDO, then validate the real new-world roster separately.
        reader.Set(FrontendType+ClassSuper,std::uintptr_t{0});reader.Set(NativeMode+cfgspawn::ClassDefaultObject,NativeDefaults);
        reader.Set(NativeDefaults+ObjectClass,NativeMode);
        reader.Set(NativeDefaults+cataloguedisk::CounselorCharacterClasses,maps::Array{});
        reader.Set(NativeDefaults+cataloguedisk::KillerCharacterClasses,maps::Array{Jasons,28,1000});
        for(unsigned i=0;i<28;++i){catalogues::SoftClass packet{};if(i==20){const cfgnames::Words path{2,0};std::memcpy(packet.data()+16,&path,8);}reader.Set(Jasons+40*i,packet);}
    }
};
template class cfgmenu::sandboxstart::NativeHost<CatalogueReader>;
template class cfgmenu::launch::NativeMenuLaunch<CatalogueReader,cfgmenu::rows::NativeDispatch>;
#ifndef CFGMENU_LAUNCH_FIXTURE_ONLY
int main(){using namespace cfgmenu;unsigned failures=0;
    const auto check=[&](bool okay,const char* label){if(!okay){++failures;std::printf("FAIL %s\n",label);}};
    for(unsigned fault=0;fault<13;++fault){LaunchFixture f;active::Context context;
        check(active::Observe(f.reader,f.Module,context),"fresh active native owner");
        if(fault==1)f.reader.Set(f.Save+selectiondisk::KillerPick,std::uintptr_t{0});
        if(fault==2)f.Name(f.Package,3,u"/Game/Jasons/Other");
        if(fault==3)f.reader.Set(f.Package+0x20,f.SelectedJason);
        if(fault==4)f.reader.Set(f.SelectedJason+cfgprobe::ClassSuper,f.NativeCounselor);
        if(fault==5){catalogues::SoftClass duplicate;f.reader.Read(f.Jasons+40*20,duplicate);f.reader.Set(f.Jasons+40*21,duplicate);}
        if(fault==6){catalogues::SoftClass nested;f.reader.Read(f.Jasons+40*20,nested);nested[32]=1;f.reader.Set(f.Jasons+40*20,nested);}
        if(fault==7)f.reader.Set(f.Registry+24*4,f.OtherWidget);
        if(fault==8)f.reader.Set(f.Save+selectiondisk::CounselorPick,f.SelectedCounselor);
        if(fault==9)f.reader.Set(f.Save+selectiondisk::CounselorPick,f.SelectedJason);
        if(fault==10){f.reader.changeAt=f.SelectedJason+0x20;f.reader.changeSlot=f.Save+selectiondisk::KillerPick;f.reader.newPointer=f.SelectedCounselor;}
        launchselection::Observation out;unsigned reads=999;
        launchselection::Diagnostic diagnostic;
        const bool ok=launchselection::Observe(f.catalogueReader,context,fault==11?41u:42u,2,3,8,fault!=12,out,&reads,&diagnostic);
        const unsigned expectedPhase[]={0,4,7,7,7,0,7,3,0,8,9,1,1};
        check(diagnostic.phase==expectedPhase[fault],"exact failed selection predicate group retained without another read");
        check(ok==(fault==0||fault==5||fault==8),"exact native saved Jason/full-path membership, identical duplicates allowed");
        check(reads<=launchselection::ReadLimit,"Start acquisition bounded, no native calls");
        if(ok){check(out.evidence.jason==f.SelectedJason&&out.jason.index==20,"CFG extended catalogue index beyond legacy18 preserved");
            check(out.evidence.preserveNativeCounselor==(fault!=8)&&out.evidence.counselor==(fault==8?f.SelectedCounselor:0),"native unset customization remains explicit deferred policy");
            Session session;Capabilities caps;caps.automaticJason=true;check(session.Open(context.widget,Mode::Counselor,caps,9),"create claimed session");
            auto evidence=out.evidence;evidence.generation=session.Generation();
            check(fault!=8?session.PickNativeCustomization(context.widget,session.Generation(),evidence.map,evidence.jason):
                session.Pick(context.widget,session.Generation(),evidence.map,evidence.jason,evidence.counselor),"native selected values into menu");
            GameplayHandoff handoff(session);check(session.Commit(context.widget,session.Generation())&&handoff.Arm(evidence),"atomic launch handoff from real selection reader");
            check(handoff.ArmedFor(context.widget,session.Generation(),context.world)&&!handoff.ArmedFor(context.widget,session.Generation()+1,context.world),"only current committed owner arms pending-mode route");
            handoff.Cancel();check(!handoff.ArmedFor(context.widget,session.Generation(),context.world),"cancel removes launch authority");
            if(!fault)std::printf("Native launch selection reads=%u; native default catalogue only, final world validation required\n",reads);
        }else check(!out.evidence.ownedSelections&&!out.evidence.selectedJason[16],"no partial selection publishes");
    }
    for(unsigned fault=0;fault<3;++fault){
        LaunchFixture f;std::u16string package=u"/Game/Jasons/Nicotero/";package.append(140,u'n');
        const auto full=package+u".Nicotero_C";
        const auto longName=[&](std::uintptr_t object,unsigned index,const std::u16string& text){
            const auto at=std::uintptr_t{0xE00000}+index*2048;std::array<char16_t,512> value{};
            std::copy(text.begin(),text.end(),value.begin());f.reader.Set(at,std::uint32_t{1});
            f.reader.Set(f.Chunk+8*index,at);f.reader.Set(at+cfgnames::disk::EntryText,value);
            f.reader.Set(object+cfgnames::disk::ObjectName,cfgnames::Words{static_cast<int>(index),0});};
        longName(f.Package,3,package);longName(f.SelectedJason,4,u"Nicotero_C");
        longName(0xAA0000,2,fault==1?full+u"_Other":fault==2?full.substr(0,full.size()-2):full);
        active::Context context;check(active::Observe(f.reader,f.Module,context),"long path owner");
        launchselection::Observation out;launchselection::Diagnostic d;
        check(launchselection::Observe(f.catalogueReader,context,42,2,3,8,true,out,nullptr,&d)==(fault==0),"Nicotero long exact full path passes; different/prefix refuse");
        check(d.jason.length==full.size()&&d.jason.selected[full.size()]==0&&d.jason.rosterLengths[20]>127,"bounded path diagnostics preserve long selected/roster length");
    }
    {
        LaunchFixture f;
        for(int cycle=0;cycle<10;++cycle){
            active::Context menu;
            check(active::Observe(f.reader,f.Module,menu),"Nth menu callback current owner");
            const auto savedCount=menu.registry.count;
            const auto save=std::uintptr_t{0xD00000}+static_cast<std::uintptr_t>(cycle)*0x1000;
            f.reader.Set(f.Module+cfgprobe::lifetime::ObjectArrayRva+28,savedCount+1);
            f.reader.Set(save+cfgprobe::lifetime::ObjectIndexOffset,savedCount);
            f.reader.Set(save+cfgprobe::ObjectClass,f.SaveType);
            f.reader.Set(save+selectiondisk::CounselorPick,std::uintptr_t{0});
            f.reader.Set(save+selectiondisk::KillerPick,f.SelectedJason);
            f.reader.Set(f.Registry+24*static_cast<std::uintptr_t>(savedCount),save);
            f.reader.Set(f.Registry+24*static_cast<std::uintptr_t>(savedCount)+16,int{0});
            f.reader.Set(f.Cache,save);
            check(active::Current(f.reader,menu,42),"registry growth preserves exact existing menu identities");
            cfgprobe::lifetime::SlotObservation staleSlot;
            check(!cfgprobe::lifetime::ReadSlot(f.reader,menu.registry,save,staleSlot),"old header path reproduces exact rejection; no stretched old extent");
            launchselection::Observation selected;launchselection::Diagnostic diagnostic;
            unsigned reads=0;
            const auto okay=launchselection::Observe(f.catalogueReader,menu,42,2,3,8,true,selected,&reads,&diagnostic);
            if(!okay)std::printf("Registry-growth cycle%d phase%u profile=%s reads%u savedCount%d newSaveIndex%d\n",
                cycle+1,diagnostic.phase,diagnostic.profile.stage,reads,menu.registry.count,savedCount);
            check(okay&&selected.evidence.jason==f.SelectedJason,"fresh event-local save beyond old header remains selectable after Nth return");
        }
    }
    std::printf("CFG concrete selected-content to armed native Start integration: %s (%u failures). No game access/calls.\n",failures?"FAIL":"PASS",failures);
    return failures?1:0;
}
#endif
