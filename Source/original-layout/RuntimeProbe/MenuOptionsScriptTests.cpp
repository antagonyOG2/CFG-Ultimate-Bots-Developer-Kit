#define main ExistingWidgetSchemaTests
#include "../EventObserver/SetupWidgetObservationTests.cpp"
#undef main
#include "MenuOptionsObservation.hpp"
struct OptionsFixture {
    static constexpr std::uintptr_t DestinationItems=0x660000,DestinationSelected=0x670000,DestinationInner=0x680000,ScriptData=0x800000;
    MenuFixture fixture;cfgmenu::capture::FunctionEvidence function;
    OptionsFixture(){
        auto& f=fixture;function.function.object=MenuFixture::Options;function.function.owner=MenuFixture::DonorType;
        function.parameterBytes=20;function.storageBytes=20;function.fieldsCount=2;function.script={ScriptData,57,57};
        f.r.Set(MenuFixture::Items+cfgreflection::disk::PropertyFlags,std::uint64_t{0x08000394});
        f.r.Set(MenuFixture::Selected+cfgreflection::disk::PropertyFlags,std::uint64_t{0x0008001040000294});
        cfgreflection::Observation<OwnedByteReader> reflection(f.r,MenuFixture::Base);
        for(unsigned i=0;i<2;++i){cfgreflection::Field field;
            reflection.ReadField(i?MenuFixture::Selected:MenuFixture::Items,MenuFixture::Options,field);reflection.ReadProperty(field,function.fields[i]);}
        using namespace cfgmenu::copydisk;auto& code=function.bytes;
        code[0]=Let;code[9]=InstanceVariable;code[18]=LocalOutVariable;code[27]=Let;code[36]=InstanceVariable;code[45]=LocalVariable;
        code[54]=Return;code[55]=Nothing;code[56]=EndOfScript;
        for(const auto& operand:std::array<std::pair<unsigned,std::uintptr_t>,6>{{{1,DestinationItems},{10,DestinationItems},{19,MenuFixture::Items},{28,DestinationSelected},{37,DestinationSelected},{46,MenuFixture::Selected}}})
            std::memcpy(code.data()+operand.first,&operand.second,8);
        f.r.Set(MenuFixture::DonorType+cfgreflection::disk::Children,DestinationItems);
        f.Field(DestinationItems,MenuFixture::DonorType,MenuFixture::ArrayProperty,"Options",DestinationSelected);f.Property(DestinationItems,16,0x200);
        f.Field(DestinationSelected,MenuFixture::DonorType,MenuFixture::IntProperty,"CurrentOption",MenuFixture::Options);f.Property(DestinationSelected,4,0x210);
        f.Field(DestinationInner,DestinationItems,MenuFixture::StrProperty,"Options",0);f.Property(DestinationInner,16,0);
        f.r.Set(DestinationItems+cfgreflection::arraydisk::Inner,DestinationInner);
        f.r.Set(MenuFixture::Options+cfgmenu::scriptdisk::Script,function.script);
        for(unsigned i=0;i<57;++i)f.r.Set(ScriptData+i,code[i]);
    }
};
struct OptionsReader {
    OwnedByteReader& r;
    unsigned reads=0;
    template<class T>bool Read(std::uintptr_t address,T& out){++reads;return r.Read(address,out);}
    bool ReadBytes(std::uintptr_t address,void* out,std::size_t bytes){auto* p=static_cast<unsigned char*>(out);for(std::size_t i=0;i<bytes;++i)if(!r.Read(address+i,p[i]))return false;return true;}
};
#ifndef CFGMENU_OPTIONS_FIXTURE_ONLY
int main(){unsigned failures=0;auto check=[&](bool ok){if(!ok)++failures;};using namespace cfgmenu::capture;
    check(ExistingWidgetSchemaTests()==0);
    {OptionsFixture f;OptionsOperands out;check(OptionsScript(f.function,out)&&out.destinationItems==OptionsFixture::DestinationItems&&out.sourceItems==MenuFixture::Items);}
    {OptionsFixture f;auto& function=f.function;
        // Replay the actual 57 captured bytes; these old addresses are evidence
        // operands only and are NEVER dereferenced or reused as live bindings.
        constexpr char hex[]="9E009EA2649B020000DB009EA2649B020000F3009FA2649B0200009EB06B716F9B020000DBB06B716F9B020000761070716F9B020000A539E5";
        const auto nibble=[](char c){return c<='9'?c-'0':c-'A'+10;};
        for(unsigned i=0;i<57;++i)function.bytes[i]=static_cast<unsigned char>((nibble(hex[2*i])<<4)|nibble(hex[2*i+1]));
        function.function.object=0x29B6F138A40ULL;function.function.owner=0x29B6EC98580ULL;
        function.fields[0].field.object=0x29B64A29F00ULL;function.fields[1].field.object=0x29B6F717010ULL;
        for(unsigned i=0;i<2;++i)function.fields[i].field.owner=function.function.object;
        OptionsOperands out;check(OptionsScript(function,out)&&out.destinationItems==0x29B64A29E00ULL&&out.destinationSelected==0x29B6F716BB0ULL);
    }
    // Every changed opcode or pointer byte must refuse; never execute a partial script.
    for(unsigned i=0;i<57;++i){OptionsFixture f;f.function.bytes[i]^=1;OptionsOperands out;check(!OptionsScript(f.function,out)&&out.destinationItems==0);}
    for(unsigned fault=0;fault<12;++fault){OptionsFixture f;auto& function=f.function;
        if(fault==0)function.script.count=56;if(fault==1)function.script.count=58;if(fault==2)function.parameterBytes=24;
        if(fault==3)function.fieldsCount=3;if(fault==4)function.fields[0].flags=0x180;if(fault==5)function.fields[1].offset=12;
        if(fault==6)function.fields[0].field.owner=MenuFixture::Type;if(fault==7)function.fields[1].field.object=MenuFixture::Items;
        if(fault==8)function.fields[0].dimension=2;
        if(fault==9)function.script.capacity=56;if(fault==10)function.storageBytes=19;if(fault==11)function.storageBytes=513;
        OptionsOperands out;check(!OptionsScript(function,out)&&out.destinationItems==0);
    }
    {OptionsFixture f;f.fixture.r.Set(OptionsFixture::DestinationItems+cfgreflection::disk::PropertyOffset,0x208);
        f.fixture.r.Set(OptionsFixture::DestinationSelected+cfgreflection::disk::PropertyOffset,0x218);
        OptionsReader reader{f.fixture.r};OptionsEvidence out;check(OptionsDependencies(reader,MenuFixture::Base,f.function,out)&&out.destinationItems.offset==0x208);}
    for(unsigned fault=0;fault<9;++fault){OptionsFixture f;auto& r=f.fixture.r;
        if(fault==1)r.Set(OptionsFixture::DestinationItems+cfgreflection::disk::PropertyFlags,std::uint64_t{cfgmenu::copydisk::PodMask});
        if(fault==2)r.Set(OptionsFixture::DestinationInner+cfgreflection::disk::PropertyFlags,std::uint64_t{cfgmenu::copydisk::PodMask});
        if(fault==3)r.Set(MenuFixture::Inner+cfgreflection::disk::PropertyFlags,std::uint64_t{cfgmenu::copydisk::PodMask});
        if(fault==4)r.Set(OptionsFixture::DestinationSelected+cfgreflection::disk::PropertyOffset,0x208);
        if(fault==5)r.Set(OptionsFixture::DestinationInner+0x10,MenuFixture::IntProperty);
        if(fault==6)r.Set(OptionsFixture::DestinationItems+0x20,MenuFixture::Type);
        if(fault==7)r.Set(OptionsFixture::ScriptData+56,static_cast<unsigned char>(0));
        if(fault==8){r.changeAt=OptionsFixture::DestinationInner+24;r.changeSlot=OptionsFixture::DestinationItems+cfgreflection::arraydisk::Inner;r.newPointer=MenuFixture::Inner;}
        OptionsReader reader{r};OptionsEvidence out;const bool ok=OptionsDependencies(reader,MenuFixture::Base,f.function,out);check(ok==(fault==0));
        if(ok)check(out.operands.destinationSelected==OptionsFixture::DestinationSelected&&out.destinationItems.offset==0x200);
        else check(out.operands.destinationItems==0);
    }
    std::printf("MenuOptionsScriptTests: %u failures; exact CFG copy script, metadata/ownership/POD/overlap/mutation refusals; no native calls\n",failures);return failures?1:0;
}
#endif
