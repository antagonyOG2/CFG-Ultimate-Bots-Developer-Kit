#define CFGMENU_NATIVE_FIXTURE_ONLY
#include "NativeMenuAdapterTests.cpp"
#include "../RuntimeProbe/JasonPickerCatalogueObservation.hpp"
#include "NicoteroCatalogueFixture.hpp"
int main(){unsigned failures=0,checks=0;const auto check=[&](bool v){++checks;if(!v)++failures;};
    for(unsigned fault=0;fault<6;++fault){NativeFixture f;auto& m=f.fixture;auto& r=m.r;
        using namespace cfgmenu;
        constexpr std::uintptr_t field=0x2900000,inner=0x2900100,structure=0x2900200,member=0x2900300,
            structKind=0x2900400,softKind=0x2900500,data=0x2910000;
        m.Name(MenuFixture::Type,"Jason_Select_Widget_C");m.Name(structKind,"StructProperty");m.Name(softKind,"SoftClassProperty");
        m.Name(structure,"FSCCharacterSelectData");
        r.Set(MenuFixture::Type+cfgreflection::disk::Children,field);
        m.Field(field,MenuFixture::Type,MenuFixture::ArrayProperty,"JasonClassList",0);m.Property(field,16,0x678);
        r.Set(field+0x70,inner);m.Field(inner,field,structKind,"JasonClassList",0);m.Property(inner,96,0);r.Set(inner+0x70,structure);
        r.Set(structure+cfgreflection::disk::Children,member);
        m.Field(member,structure,softKind,"CharacterClass_10_29392D434148412C26DBACA4ECDABFEC",0);m.Property(member,40,0);
        r.Set(MenuFixture::Widget+0x678,maps::Array{data,19,64});
        for(unsigned i=0;i<19;++i){const auto object=std::uintptr_t{0x2920000}+i*0x100;
            m.Name(object,i<18?NicoteroBaseRoster[i]:"/Game/Characters/Killers/Jason/JUNI/Jason_UNI.Jason_UNI_C");
            cfgnames::Words path;r.Read(object+cfgnames::disk::ObjectName,path);catalogues::SoftClass packet{};
            std::memcpy(packet.data()+16,&path,8);r.Set(data+96*i,packet);}
        if(fault==1)r.Set(inner+cfgreflection::disk::PropertyElementSize,40);
        if(fault==2)r.Set(member+cfgreflection::disk::PropertyOffset,8);
        if(fault==3)r.Set(MenuFixture::Widget+0x678,maps::Array{data,65,65});
        if(fault==4){catalogues::SoftClass nested{};nested[24]=1;r.Set(data+18*96,nested);}
        OptionsReader reader{r};active::Context c;check(active::Observe(reader,MenuFixture::Base,c));
        if(fault==5)r.Set(f.Registry+24*2+16,9999);
        pickerlist::Snapshot snapshot;unsigned stage=9;const bool okay=pickerlist::Observe(reader,c,snapshot,stage);
        if(okay!=(fault==0))std::printf("Picker row fault%u stage%u\n",fault,stage);
        check(okay==(fault==0));
        if(okay){check(snapshot.classes.header.count==19&&snapshot.classes.header.capacity==64);
            cfgnames::Words path;std::memcpy(&path,snapshot.classes.entries[18].data()+16,8);
            cfgnames::Observation<OptionsReader> names(reader,c.module);cfgnames::PathText text;
            check(names.Decode(path,text)&&text.View()==u"/Game/Characters/Killers/Jason/JUNI/Jason_UNI.Jason_UNI_C");}
    }
    std::printf("Native Jason picker all used96-byte rows/inner schema/capacity/stale owner: %u checks, %u failures. Owned fixture, no game.\n",checks,failures);
    return failures?1:0;
}
