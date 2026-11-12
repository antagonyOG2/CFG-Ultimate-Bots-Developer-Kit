#define CFGMENU_BIRTH_FIXTURE_ONLY
#include "NativeCounselorBirthTests.cpp"
#include "../RuntimeProbe/FreshWorldMapObservation.hpp"
namespace wm=cfgmenu::worldmap;
static_assert(cfgmenu::worldmapdisk::SCMapDefinitionClass==cfgmenu::selectiondisk::SCMapDefinitionClass);
static_assert(cfgmenu::worldmapdisk::SCMapDefinitionConstructor==cfgmenu::selectiondisk::SCMapDefinitionConstructor);
static_assert(cfgmenu::worldmapdisk::StructPropertyVtable==cfgcompat::cfg35::reflection::StructPropertyVtable);
static_assert(cfgmenu::worldmapdisk::FilePathStructCache!=0x32EB268&&cfgmenu::worldmapdisk::WorldClass!=0x32F53B8&&
    cfgmenu::worldmapdisk::NativeClassCache!=0x3196750&&cfgmenu::worldmapdisk::NativePackageCache!=0x3196E30);
struct MapFixture:BirthFixture {
    static constexpr std::uintptr_t MapBase=0x500000,Definition=0x510000,Defaults=0x520000,Package=0x530000,
        WorldType=0x540000,ClassType=0x550000,BlueprintType=0x560000,PackageType=0x570000,FilePath=0x580000,
        Path=0x590000,Snow=0x5A0000,String=0x5B0000,StructKind=0x5C0000,StringKind=0x5D0000,ScriptStructKind=0x5E0000,
        NameTable=0xA10000,NameChunk=0xA20000,NormalStorage=0xC00000,SnowStorage=0xC10000;
    wm::ArmedMap source{Module,0x900000,Definition,7,11};unsigned nextName=0;
    void Name(std::uintptr_t object,const char* label){
        const auto index=nextName++;const auto entry=std::uintptr_t{0xB00000}+index*0x100;
        reader.Set(NameChunk+8*index,entry);reader.Set(entry,std::uint32_t{0});std::array<unsigned char,128> text{};
        for(unsigned i=0;label[i]&&i<127;++i)text[i]=static_cast<unsigned char>(label[i]);
        reader.Set(entry+cfgnames::disk::EntryText,text);reader.Set(object+cfgnames::disk::ObjectName,cfgnames::Words{static_cast<int>(index),0});
        struct Counts{int entries,chunks;};reader.Set(NameTable+cfgnames::disk::ElementCount,Counts{static_cast<int>(nextName),1});
    }
    void Field(std::uintptr_t f,std::uintptr_t owner,std::uintptr_t kind,const char* name,unsigned offset,std::uintptr_t next=0){
        reader.Set(f+0x20,owner);reader.Set(f+cfgprobe::ObjectClass,kind);reader.Set(f+0x28,next);Name(f,name);
        reader.Set(f+0x30,int{1});reader.Set(f+0x34,int{16});reader.Set(f+0x44,static_cast<int>(offset));reader.Set(f+0x38,std::uint64_t{0x10000000000005});
    }
    void Text(bool snow,std::u16string_view text){
        std::array<char16_t,256> chars{};std::copy(text.begin(),text.end(),chars.begin());
        const auto address=snow?SnowStorage:NormalStorage;reader.Set(address,chars);
        reader.Set(Defaults+(snow?0x58:0x48),wm::StringHeader{address,static_cast<int>(text.size()+1),static_cast<int>(text.size()+1)});
    }
    MapFixture(){using namespace cfgprobe;using namespace cfgmenu::worldmapdisk;
        plan.objectArray.count=8;reader.Set(Module+lifetime::ObjectArrayRva+28,int{8});
        reader.Set(Definition+lifetime::ObjectIndexOffset,int{6});reader.Set(Registry+6*24,Definition);reader.Set(Registry+6*24+16,int{106});
        reader.Set(Defaults+lifetime::ObjectIndexOffset,int{7});reader.Set(Registry+7*24,Defaults);reader.Set(Registry+7*24+16,int{107});
        reader.Set(Module+SCMapDefinitionClass,MapBase);reader.Set(Module+SCMapDefinitionConstructor,MapBase);
        reader.Set(Module+cfgmenu::worldmapdisk::WorldClass,WorldType);reader.Set(Module+cfgmenu::worldmapdisk::WorldConstructor,WorldType);
        reader.Set(Module+NativeClassCache,ClassType);reader.Set(Module+NativePackageCache,PackageType);reader.Set(Module+FilePathStructCache,FilePath);
        reader.Set(Definition+ObjectClass,BlueprintType);reader.Set(BlueprintType+ClassSuper,ClassType);reader.Set(ClassType+ClassSuper,std::uintptr_t{0});
        reader.Set(Definition+ClassSuper,MapBase);reader.Set(MapBase+ClassSuper,std::uintptr_t{0});reader.Set(Definition+0xF8,Defaults);
        reader.Set(Defaults+ObjectClass,Definition);reader.Set(World+ObjectClass,WorldType);reader.Set(World+0x20,Package);
        reader.Set(Package+ObjectClass,PackageType);reader.Set(Package+0x20,std::uintptr_t{0});
        reader.Set(Module+cfgnames::disk::Global,NameTable);reader.Set(NameTable,NameChunk);
        Name(Package,"/Game/Maps/Lake");Name(World,"Lake");Name(StructKind,"StructProperty");Name(StringKind,"StrProperty");Name(ScriptStructKind,"ScriptStruct");
        Name(FilePath,"FilePath");reader.Set(FilePath+ObjectClass,ScriptStructKind);reader.Set(FilePath+0x40,int{16});reader.Set(FilePath+0x38,String);
        reader.Set(Definition+0x38,std::uintptr_t{0});reader.Set(MapBase+0x38,Path);
        Field(Path,MapBase,StructKind,"Path",0x48,Snow);Field(Snow,MapBase,StructKind,"SnowMapPath",0x58);Field(String,FilePath,StringKind,"FilePath",0);
        reader.Set(Path,Module+StructPropertyVtable);reader.Set(Snow,Module+StructPropertyVtable);
        reader.Set(Path+0x70,FilePath);reader.Set(Snow+0x70,FilePath);
        Text(false,u"/Game/Maps/Lake");Text(true,u"/Game/Maps/Lake_Snow");
    }
};
#ifndef CFGMENU_MAP_FIXTURE_ONLY
int main(){unsigned checks=0,failures=0;const auto check=[&](bool ok,const char* what){++checks;if(!ok){++failures;std::printf("FAIL %s\n",what);}};
    for(unsigned form=0;form<5;++form){MapFixture f;
        if(form==1){f.Name(f.Package,"/Game/Maps/Lake_Snow");f.Name(f.World,"Lake_Snow");}
        if(form==2)f.Text(false,u"/Game/Maps/Lake.Lake");
        if(form==3){f.Name(f.Package,"/Game/Maps/Lake_Snow");f.Name(f.World,"Lake_Snow");f.Text(true,u"/Game/Maps/Lake_Snow.Lake_Snow");}
        if(form==4)f.Text(true,u"/Game/Maps/Lake.Lake");
        wm::Observation out;unsigned reads=0;wm::Failure failure;
        check(wm::Observe(f.reader,f.plan,f.source,42,true,out,&reads,&failure),"exact normal/snow canonical association");
        check(out.world==f.World&&out.rootPackage==f.Package&&out.definition==f.Definition&&out.defaults==f.Defaults&&out.worldIdentity.serial==100,"actual world/package read independently; token exposed after matching");
        check(out.variant==(form==4?wm::Variant::Both:form==1||form==3?wm::Variant::Snow:wm::Variant::Normal),"actual matched normal/snow path recorded, never weather guessed");
        if(form==2)check(out.normalForm==wm::Form::Object,"exact object path uses actual World name");
        if(!form)std::printf("Fresh-world map association reads=%u\n",reads);
        check(reads<=wm::ReadLimit&&failure==wm::Failure::None,"bounded read-only proof");
    }
    for(unsigned fault=0;fault<31;++fault){MapFixture f;
        if(fault==0)f.source.frontendWorld=f.World;
        if(fault==1)f.source.submission=0;
        if(fault==2)f.source.generation=0;
        if(fault==3)f.source.module+=8;
        if(fault==4)f.plan.objects[0].serial=0;
        if(fault==5)f.reader.Set(f.Controller+cfgprobe::ControllerAcknowledgedPawn,f.State);
        if(fault==6)f.reader.Set(f.World+cfgprobe::WorldNetDriver,f.State);
        if(fault==7)f.reader.Set(f.Package+cfgprobe::ObjectClass,f.WorldType);
        if(fault==8)f.reader.Set(f.Package+0x20,f.Definition);
        if(fault==9)f.reader.Set(f.Definition+cfgprobe::ClassSuper,f.ClassType);
        if(fault==10)f.reader.Set(f.Defaults+cfgprobe::ObjectClass,f.MapBase);
        if(fault==11)f.reader.Set(f.Path+0x70,f.MapBase);
        if(fault==12)f.reader.Set(f.Path+0x44,int{0x58});
        if(fault==13)f.reader.Set(f.Path+0x34,int{8});
        if(fault==14)f.Name(f.FilePath,"OtherStruct");
        if(fault==15)f.Name(f.String,"OtherString");
        if(fault==16)f.reader.Set(f.String+0x44,int{8});
        if(fault==17)f.Text(false,u"Lake");
        if(fault==18)f.Text(false,u"/Game/Maps/Lake?game=snbx");
        if(fault==19)f.Text(false,u"World'/Game/Maps/Lake.Lake'");
        if(fault==20)f.Text(false,u"/Game/Maps/Lake.umap");
        if(fault==21)f.Name(f.Package,"/Game/Maps/UEDPIE_0_Lake");
        if(fault==22)f.Name(f.Package,"/Game/Maps/Other/Lake");
        if(fault==23)f.Text(false,u"/Game/Maps/lake");
        if(fault==24)f.reader.Set(f.Defaults+0x48,wm::StringHeader{f.NormalStorage,257,257});
        if(fault==25)f.reader.Set(f.NormalStorage+2*15,char16_t{u'X'}); // Replaces the required terminator.
        if(fault==26)f.reader.Set(f.NormalStorage+2*8,char16_t{0});
        if(fault==27)f.reader.Set(f.Path+0x28,f.Path);
        if(fault==28)f.reader.Set(f.Registry+24*6,f.State);
        wm::Observation out;unsigned reads=0;wm::Failure failure;
        const bool okay=wm::Observe(f.reader,f.plan,f.source,fault==29?43:42,fault!=30,out,&reads,&failure);
        check(!okay&&!out.definition&&!out.rootPackage&&out.variant==wm::Variant::None,"wrong identity/schema/path/thread fails without partial association");
        check(failure!=wm::Failure::None&&reads<=wm::ReadLimit,"bounded explicit refusal");
        if(fault==17||fault==18||fault==19)check(failure==wm::Failure::UnsupportedSyntax,"unproven URL normalization not guessed");
    }
    const std::array<std::uintptr_t,6> currentCaches{{cfgmenu::worldmapdisk::SCMapDefinitionClass,cfgmenu::worldmapdisk::SCMapDefinitionConstructor,
        cfgmenu::worldmapdisk::WorldClass,cfgmenu::worldmapdisk::WorldConstructor,cfgmenu::worldmapdisk::NativeClassCache,cfgmenu::worldmapdisk::NativePackageCache}};
    const std::array<std::uintptr_t,6> oldCaches{{0x304CDC8,0x304CDD0,0x32F53B8,0x32F53C0,0x3196750,0x3196E30}};
    for(unsigned i=0;i<currentCaches.size();++i){MapFixture f;std::uintptr_t value=0;
        check(f.reader.Read(f.Module+currentCaches[i],value),"current cache populated in fixture");
        f.reader.Set(f.Module+currentCaches[i],std::uintptr_t{});f.reader.Set(f.Module+oldCaches[i],value);
        wm::Observation out;wm::Failure failure;
        check(!wm::Observe(f.reader,f.plan,f.source,42,true,out,nullptr,&failure)&&failure==wm::Failure::ClassIdentity&&!out.definition,
            "old3.4 cache cannot substitute for missing current3.5 identity");
    }
    for(unsigned mutation=0;mutation<6;++mutation){MapFixture f;
        f.reader.changeAt=f.NormalStorage;
        if(mutation==0){f.reader.changeSlot=f.Registry+16;f.reader.newPointer=999;}
        if(mutation==1){f.reader.changeSlot=f.Path+0x44;f.reader.newPointer=0x58;}
        if(mutation==2){f.reader.changeSlot=f.World+0x20;f.reader.newPointer=f.Definition;}
        if(mutation==3){f.reader.changeSlot=f.Definition+0xF8;f.reader.newPointer=f.State;}
        if(mutation==4){f.reader.changeSlot=f.Registry+6*24+16;f.reader.newPointer=999;}
        if(mutation==5){f.reader.changeSlot=f.Path+0x38;f.reader.newPointer=0;}
        wm::Observation out;check(!wm::Observe(f.reader,f.plan,f.source,42,true,out)&&!out.definition,"mid-observation metadata/world replacement refuses");
    }
    std::printf("CFG fresh-world map association: %s (%u checks, %u failures). Owned read-only fixtures; no game/native calls.\n",failures?"FAIL":"PASS",checks,failures);return failures?1:0;
}
#endif
