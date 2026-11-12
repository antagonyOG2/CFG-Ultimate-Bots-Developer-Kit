#define CFGMENU_LAUNCH_FIXTURE_ONLY
#include "NativeLaunchSelectionTests.cpp"
#include "../RuntimeProbe/SelectedJasonClassObservation.hpp"
#include "NicoteroCatalogueFixture.hpp"
#include "PrimarySpawnObservation.hpp"
#include "NativeJasonCalls.hpp"
namespace {
std::uintptr_t expectedSelected=0;unsigned selectedSpawnCalls=0;
void* __fastcall SelectedSpawnCtor(void* packet){return packet;}
std::uintptr_t __fastcall SelectedSpawn(void*,void* klass,const void*,const void*,const void*){
    ++selectedSpawnCalls;return reinterpret_cast<std::uintptr_t>(klass)==expectedSelected?0xABC000:0;
}
}
int main(){unsigned failures=0,checks=0;const auto check=[&](bool v){++checks;if(!v)++failures;};
    constexpr std::uintptr_t defaults=0xAF0000;
    for(unsigned fault=0;fault<12;++fault){LaunchFixture f;using namespace cfgprobe;
        constexpr std::uintptr_t meta=0xBE0000;
        f.reader.Set(f.Module+cfgcompat::cfg35::worldMap::NativeClassCache,meta);f.reader.Set(f.SelectedJason+ObjectClass,meta);f.reader.Set(meta+ClassSuper,std::uintptr_t{0});
        f.reader.Set(f.Names+cfgnames::disk::ElementCount,std::array<int,2>{{6,1}});
        f.Name(f.Package,3,u"/Game/Characters/Killers/Jason/JUNI/Jason_UNI");f.Name(f.SelectedJason,4,u"Jason_UNI_C");
        f.reader.Set(f.Module+lifetime::ObjectArrayRva+28,7);
        f.reader.Set(f.SelectedJason+lifetime::ObjectIndexOffset,5);f.reader.Set(f.Registry+24*5,f.SelectedJason);f.reader.Set(f.Registry+24*5+16,707);
        f.reader.Set(defaults+lifetime::ObjectIndexOffset,6);f.reader.Set(f.Registry+24*6,defaults);f.reader.Set(f.Registry+24*6+16,0);
        f.reader.Set(f.Registry+24*5+8,std::uint32_t{0});f.reader.Set(f.Registry+24*6+8,std::uint32_t{0});
        f.reader.Set(f.SelectedJason+cfgspawn::ClassDefaultObject,defaults);f.reader.Set(defaults+ObjectClass,f.SelectedJason);
        f.reader.Set(f.SelectedJason+cfgcompat::cfg35::primaryJason::SelectedClassFlags,std::uint32_t{0});
        if(fault==1)f.reader.Set(f.SelectedJason+ClassSuper,f.NativeCounselor);
        if(fault==2)f.reader.Set(f.SelectedJason+cfgcompat::cfg35::primaryJason::SelectedClassFlags,std::uint32_t{1});
        if(fault==3)f.Name(f.Package,3,u"/Engine/Characters/Killers/Jason/JUNI/Jason_UNI");
        if(fault==4)f.Name(f.Package,3,u"/Game/Characters/Counselors/Tommy");
        if(fault==5)f.reader.Set(f.Registry+24*5+16,0);
        if(fault==6)f.reader.Set(f.SelectedJason+cfgspawn::ClassDefaultObject,std::uintptr_t{0});
        if(fault==7)f.reader.Set(defaults+ObjectClass,f.NativeCounselor);
        if(fault==8)f.reader.Set(f.SelectedJason+cfgcompat::cfg35::primaryJason::SelectedClassFlags,std::uint32_t{0x02000000});
        if(fault==9)f.reader.Set(f.Package+0x20,f.SelectedJason);
        if(fault==10)f.reader.Set(f.Registry+24*5+8,std::uint32_t{0x20000000});
        if(fault==11)f.reader.Set(f.SelectedJason+ObjectClass,f.NativeCounselor);
        lifetime::ArrayHeader registry;check(lifetime::ReadArrayHeader(f.reader,f.Module+lifetime::ObjectArrayRva,registry));
        cfgselectedjason::Choice c;const bool okay=cfgselectedjason::Observe(f.catalogueReader,f.Module,registry,f.SelectedJason,f.NativeJason,c);
        check(okay==(fault==0));if(okay){check(c.Valid()&&c.Path()==u"/Game/Characters/Killers/Jason/JUNI/Jason_UNI.Jason_UNI_C");
            check(cfgselectedjason::Current(f.catalogueReader,f.Module,registry,f.NativeJason,c));
            f.reader.Set(f.Registry+24*5+16,708);check(!cfgselectedjason::Current(f.catalogueReader,f.Module,registry,f.NativeJason,c));}
        else check(c.Empty());
    }
    {LaunchFixture f;using namespace cfgprobe;
        constexpr std::uintptr_t meta=0xBE0000;
        f.reader.Set(f.Module+cfgcompat::cfg35::worldMap::NativeClassCache,meta);f.reader.Set(f.SelectedJason+ObjectClass,meta);f.reader.Set(meta+ClassSuper,std::uintptr_t{0});
        f.Name(f.Package,3,u"/Game/Characters/Killers/Jason/JUNI/Jason_UNI");f.Name(f.SelectedJason,4,u"Jason_UNI_C");
        f.reader.Set(f.SelectedJason+lifetime::ObjectIndexOffset,5);f.reader.Set(f.Registry+24*5,f.SelectedJason);f.reader.Set(f.Registry+24*5+16,707);
        f.reader.Set(defaults+lifetime::ObjectIndexOffset,6);f.reader.Set(f.Registry+24*6,defaults);f.reader.Set(f.Registry+24*6+16,0);
        f.reader.Set(f.Registry+24*5+8,std::uint32_t{0});f.reader.Set(f.Registry+24*6+8,std::uint32_t{0});
        f.reader.Set(f.SelectedJason+cfgspawn::ClassDefaultObject,defaults);f.reader.Set(defaults+ObjectClass,f.SelectedJason);
        f.reader.Set(f.SelectedJason+cfgcompat::cfg35::primaryJason::SelectedClassFlags,std::uint32_t{0});
        f.reader.Set(f.Module+lifetime::ObjectArrayRva+28,7);
        f.reader.Set(f.NativeDefaults+cfgmenu::cataloguedisk::KillerCharacterClasses,cfgmenu::maps::Array{f.Jasons,18,18});
        for(unsigned i=0;i<18;++i){std::u16string text;for(const char* s=NicoteroBaseRoster[i];*s;++s)text+=static_cast<char16_t>(*s);
            f.Name(0xBA0000+256*i,6+i,text);cfgmenu::catalogues::SoftClass packet{};const cfgnames::Words words{static_cast<int>(6+i),0};
            std::memcpy(packet.data()+16,&words,8);f.reader.Set(f.Jasons+40*i,packet);}
        f.reader.Set(f.Names+cfgnames::disk::ElementCount,std::array<int,2>{{24,1}});
        cfgmenu::active::Context c;check(cfgmenu::active::Observe(f.reader,f.Module,c));
        cfgmenu::launchselection::Observation selected;cfgmenu::launchselection::Diagnostic diagnostic;
        check(cfgmenu::launchselection::Observe(f.catalogueReader,c,42,2,3,8,true,selected,nullptr,&diagnostic));
        check(selected.evidence.selectedClass.Valid()&&selected.jason.index==cfgselectedjason::NoCatalogueIndex&&
            cfgmenu::softpath::Empty(selected.evidence.selectedJason)&&diagnostic.jason.rosterCount==18&&diagnostic.jason.matches==0);
        constexpr std::uintptr_t baseAI=0xBD0000;
        f.reader.Set(defaults+cfgspawn::PawnAIClass,baseAI);f.reader.Set(defaults+cfgspawn::CharacterTree,std::uintptr_t{0});
        cfgprobe::lifetime::ArrayHeader registry;check(cfgprobe::lifetime::ReadArrayHeader(f.reader,f.Module+lifetime::ObjectArrayRva,registry));
        cfgspawn::TemplateObservation resolved;
        check(cfgprimary::ObserveSelectedTemplate(f.catalogueReader,f.Module,registry,selected.evidence.selectedClass,f.NativeJason,baseAI,resolved));
        check(resolved.pawnClass==f.SelectedJason&&resolved.defaults==defaults&&resolved.classSlot==selected.evidence.selectedClass.identity);
        cfgnativejason::Fault fault;std::uintptr_t result=0;cfgspawn::Vec3 position{1,2,3},rotation{};expectedSelected=resolved.pawnClass;
        check(cfgnativejason::detail::Spawn(reinterpret_cast<std::uintptr_t>(&SelectedSpawnCtor),reinterpret_cast<std::uintptr_t>(&SelectedSpawn),
            f.World,resolved.pawnClass,&position,&rotation,result,fault)&&result==0xABC000&&selectedSpawnCalls==1&&!fault.code);
        // This is the actual production selection helper + Windows ABI wrapper
        // to an OWNED stub. Engine allocation/replication remains live untested.
        f.reader.Set(f.Registry+24*5+16,708);
        check(!cfgprimary::ObserveSelectedTemplate(f.catalogueReader,f.Module,registry,selected.evidence.selectedClass,f.NativeJason,baseAI,resolved));
    }
    std::printf("Selected Jason exact class/CDO/path/abstract/deprecated/stale serial + exact18/JUNI frontend/spawn: %u checks, %u failures. Owned Windows ABI stub only; no game calls.\n",checks,failures);
    return failures?1:0;
}
