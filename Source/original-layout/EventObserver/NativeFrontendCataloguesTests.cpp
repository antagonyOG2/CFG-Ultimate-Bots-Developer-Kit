#define CFGMENU_MAP_FIXTURE_ONLY
#include "NativeMapSelectionTests.cpp"
#include "../RuntimeProbe/NativeFrontendCatalogues.hpp"
struct CatalogueReader {
    OwnedByteReader& source;
    template<class T> bool Read(std::uintptr_t at,T& value){return source.Read(at,value);}
    bool ReadBytes(std::uintptr_t at,void* value,std::size_t size){
        ++source.reads;auto* bytes=static_cast<unsigned char*>(value);
        for(std::size_t i=0;i<size;++i){const auto found=source.bytes.find(at+i);if(found==source.bytes.end())return false;bytes[i]=found->second;}return true;
    }
};
struct CatalogueFixture:MapFixture {
    static constexpr std::uintptr_t NativeMode=0x800000,Modes=0x810000,Sandbox=0x820000,SandboxDefaults=0x830000,
        SandboxMaps=0x840000,Counselors=0x850000,Jasons=0x860000,Names=0x870000,Chunk=0x880000,
        Entry=0x890000,Alias=0x8A0000,SandboxAlias=0x8B0000,NativeDefaults=0x8C0000;
    CatalogueReader catalogueReader{reader};
    CatalogueFixture(){using namespace cfgmenu;using namespace cataloguedisk;
        reader.Set(Module+GameModeClass,NativeMode);reader.Set(Module+GameModeConstructor,NativeMode);
        reader.Set(FrontendType+cfgprobe::ClassSuper,NativeMode);reader.Set(NativeMode+cfgprobe::ClassSuper,std::uintptr_t{0});
        reader.Set(Defaults+ShippingGameModes,maps::Array{Modes,2,1000});reader.Set(Modes,OfflineMode);reader.Set(Modes+8,Sandbox);
        reader.Set(Sandbox+cfgprobe::ClassSuper,ModeDefinition);reader.Set(Sandbox+cfgspawn::ClassDefaultObject,SandboxDefaults);
        reader.Set(SandboxDefaults+cfgprobe::ObjectClass,Sandbox);reader.Set(SandboxDefaults+selectiondisk::SupportedMaps,maps::Array{SandboxMaps,1,1000});reader.Set(SandboxMaps,MapB);
        reader.Set(Module+cfgnames::disk::Global,Names);reader.Set(Names,Chunk);reader.Set(Names+cfgnames::disk::ElementCount,std::array<int,2>{{2,1}});
        Name(OfflineMode,0,u"Offline_Mode_C");Name(Sandbox,1,u"Sandbox_Mode_C");
        Text(ModeDefaults+selectiondisk::Alias,Alias,u"offline");Text(SandboxDefaults+selectiondisk::Alias,SandboxAlias,u"sandbox");
        reader.Set(Mode+CounselorCharacterClasses,maps::Array{Counselors,21,1000});reader.Set(Mode+KillerCharacterClasses,maps::Array{Jasons,28,1000});
        for(unsigned i=0;i<21;++i){catalogues::SoftClass bytes{};if(i)bytes[16]=static_cast<unsigned char>(i);reader.Set(Counselors+40*i,bytes);}
        for(unsigned i=0;i<28;++i){catalogues::SoftClass bytes{};bytes[16]=static_cast<unsigned char>(i+1);reader.Set(Jasons+40*i,bytes);}
    }
    void Name(std::uintptr_t object,unsigned index,std::u16string_view text){
        const auto at=Entry+256*index;reader.Set(Chunk+8*index,at);reader.Set(at,std::uint32_t{1});
        std::array<char16_t,128> value{};std::copy(text.begin(),text.end(),value.begin());reader.Set(at+cfgnames::disk::EntryText,value);
        reader.Set(object+cfgnames::disk::ObjectName,cfgnames::Words{static_cast<int>(index),0});
    }
    void Text(std::uintptr_t field,std::uintptr_t data,std::u16string_view text){
        std::array<char16_t,128> value{};std::copy(text.begin(),text.end(),value.begin());reader.Set(data,value);
        reader.Set(field,cfgmenu::catalogues::StringHeader{data,static_cast<int>(text.size()+1),1000});
    }
};
#ifndef CFGMENU_CATALOGUE_FIXTURE_ONLY
int main(){using namespace cfgmenu;unsigned failures=0;auto check=[&](bool okay,const char* label){if(!okay){++failures;std::printf("FAIL %s\n",label);}};
    for(unsigned fault=0;fault<15;++fault){CatalogueFixture f;active::Context current;check(active::Observe(f.reader,Fixture::Module,current),"active fixture");using F=CatalogueFixture;
        if(fault==1)f.reader.Set(F::Defaults+cataloguedisk::ShippingGameModes,maps::Array{F::Modes,33,33});
        if(fault==2)f.reader.Set(F::Modes+8,F::OfflineMode);
        if(fault==3)f.reader.Set(F::Sandbox+cfgprobe::ClassSuper,F::MapA);
        if(fault==4)f.reader.Set(F::SandboxDefaults+selectiondisk::Alias,catalogues::StringHeader{F::SandboxAlias,129,129});
        if(fault==5)f.reader.Set(F::SandboxAlias+14,char16_t{u'x'});
        if(fault==6)f.reader.Set(F::SandboxDefaults+selectiondisk::SupportedMaps,maps::Array{F::SandboxMaps,2,2});
        if(fault==6)f.reader.Set(F::SandboxMaps+8,F::MapB);
        if(fault==7){f.reader.changeAt=F::SandboxMaps;f.reader.changeSlot=F::Modes+8;f.reader.newPointer=F::OfflineMode;}
        if(fault==8)f.reader.Set(F::Module+selectiondisk::SCModeDefinitionConstructor,F::MapB);
        if(fault==9)current.thread=41;
        if(fault==11)f.reader.Set(F::Registry+24*2+16,int{999});
        if(fault==12)f.reader.Set(F::SandboxMaps,F::MapA);
        if(fault==13)f.reader.Set(F::Defaults+cataloguedisk::ShippingGameModes,maps::Array{});
        if(fault==14)f.reader.Set(F::SandboxDefaults+selectiondisk::Alias,catalogues::StringHeader{});
        catalogues::Modes out;unsigned reads=0;const auto okay=catalogues::ObserveModes(f.catalogueReader,current,fault!=10,out,&reads);
        check(okay==(fault==0||fault>=12),"modes refuse bad/mutating source; no fallback");check(reads<=catalogues::ReadLimit,"bounded mode reads");
        if(!okay)check(out.count==0,"no partial mode publication");
        if(fault==0){check(out.count==2&&out.entries[1].alias.View()==u"sandbox"&&out.entries[1].name.View()==u"Sandbox_Mode_C"&&out.entries[1].supportsSelectedMap,"authored alias and native map membership");std::printf("Mode catalogue reads=%u\n",reads);}
        if(fault==12)check(!out.entries[1].supportsSelectedMap,"unsupported selected map is not silently replaced");
    }
    for(unsigned fault=0;fault<18;++fault){CatalogueFixture f;active::Context current;check(active::Observe(f.reader,Fixture::Module,current),"active characters fixture");using F=CatalogueFixture;
        if(fault>=12){
            f.reader.Set(F::FrontendType+cfgprobe::ClassSuper,std::uintptr_t{0});
            f.reader.Set(F::NativeMode+cfgspawn::ClassDefaultObject,F::NativeDefaults);
            f.reader.Set(F::NativeDefaults+cfgprobe::ObjectClass,F::NativeMode);
            f.reader.Set(F::NativeDefaults+cataloguedisk::CounselorCharacterClasses,maps::Array{});
            f.reader.Set(F::NativeDefaults+cataloguedisk::KillerCharacterClasses,maps::Array{});
            if(fault==13)f.reader.Set(F::Module+cataloguedisk::GameModeConstructor,F::MapA);
            if(fault==14)f.reader.Set(F::NativeDefaults+cfgprobe::ObjectClass,F::OfflineMode);
            if(fault==15){f.reader.changeAt=F::NativeDefaults+cataloguedisk::KillerCharacterClasses;f.reader.changeSlot=F::NativeMode+cfgspawn::ClassDefaultObject;f.reader.newPointer=F::ModeDefaults;}
            if(fault==16)f.reader.Set(F::NativeMode+cfgspawn::ClassDefaultObject,std::uintptr_t{0});
            if(fault==17){f.reader.Set(F::NativeDefaults+cataloguedisk::CounselorCharacterClasses,maps::Array{F::Counselors,21,1000});
                f.reader.Set(F::NativeDefaults+cataloguedisk::KillerCharacterClasses,maps::Array{F::Jasons,28,1000});}
        }
        if(fault==1)f.reader.Set(F::Mode+cataloguedisk::CounselorCharacterClasses,maps::Array{F::Counselors,65,65});
        if(fault==2)f.reader.Set(F::Mode+cataloguedisk::KillerCharacterClasses,maps::Array{F::Jasons,28,27});
        if(fault==3)f.reader.Set(F::Module+cataloguedisk::GameModeConstructor,F::MapB);
        if(fault==4)f.reader.Set(F::FrontendType+cfgprobe::ClassSuper,F::MapA);
        if(fault==5){f.reader.changeAt=F::Counselors+40*20;f.reader.changeSlot=F::Counselors;f.reader.newPointer=0x900000;}
        if(fault==6)current.thread=41;
        if(fault==8)f.reader.Set(F::Registry+24*2+16,int{999});
        if(fault==9)f.reader.Set(F::Mode+cataloguedisk::CounselorCharacterClasses,maps::Array{});
        if(fault==10){catalogues::SoftClass a{};a[16]=1;f.reader.Set(F::Counselors+80,a);}
        if(fault==11){catalogues::SoftClass a{};f.reader.Set(F::Jasons+40*27,a);}
        catalogues::Characters out;unsigned reads=0;const auto okay=catalogues::ObserveCharacters(f.catalogueReader,current,fault!=7,out,&reads);
        check(okay==(fault==0||(fault>=9&&fault<=12)||fault==17),"opaque characters refuse bad/mutating ownership");check(reads<=catalogues::ReadLimit,"bounded character reads");
        if(!okay)check(out.counselors.header.count==0&&out.jasons.header.count==0,"no partial character publication");
        if(fault==0){check(out.counselors.header.count==21&&out.jasons.header.count==28&&!catalogues::Nonempty(out.counselors.entries[0])&&out.counselors.entries[20][16]==20,"native order/empty entries/40-byte opaque layout preserved");std::printf("Character catalogue reads=%u\n",reads);}
        if(fault==10)check(out.counselors.entries[1]==out.counselors.entries[2],"duplicates retained, not implicit concrete picks");
        if(fault==12)check(out.nativeDefaults&&out.source==F::NativeDefaults&&out.counselors.header.count==0&&out.jasons.header.count==0,
            "frontend never reads wrong roster offsets; empty native defaults are not fabricated cooked roster");
        if(fault==17)check(out.nativeDefaults&&out.source==F::NativeDefaults&&out.counselors.header.count==21&&out.jasons.header.count==28,
            "native constructor roster retained but not claimed as cooked extended roster");
    }
    std::printf("CFG native frontend catalogue tests: %s (%u failures). No game access.\n",failures?"FAIL":"PASS",failures);return failures?1:0;
}
#endif
