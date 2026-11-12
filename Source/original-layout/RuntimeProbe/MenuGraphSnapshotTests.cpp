#define main ExistingWidgetSchemaTests
#include "../EventObserver/SetupWidgetObservationTests.cpp"
#undef main
#include "MenuGraphSnapshot.hpp"
struct GraphReader {
    OwnedByteReader& source;unsigned reads=0;std::uintptr_t trigger=0,target=0;std::uint64_t replacement=0;
    template<class T>bool Read(std::uintptr_t at,T& out){++reads;if(at==trigger&&target)source.Set(target,replacement);return source.Read(at,out);}
    bool ReadBytes(std::uintptr_t at,void* out,std::size_t count){++reads;auto* data=static_cast<unsigned char*>(out);
        for(std::size_t i=0;i<count;++i){const auto found=source.bytes.find(at+i);if(found==source.bytes.end())return false;data[i]=found->second;}return true;}
};
int main(){using namespace cfgmenu::capture;unsigned failures=0,checks=0;
    const auto check=[&](bool okay){++checks;if(!okay)++failures;};
    for(unsigned fault=0;fault<7;++fault){MenuFixture fixture;constexpr std::uintptr_t registry=0x800000,code=0x810000,unreadLocals=0x920000;
        fixture.Name(MenuFixture::Options,"ExecuteUbergraph_Jason_Select_Widget");
        fixture.Field(MenuFixture::Items,MenuFixture::Options,MenuFixture::IntProperty,"EntryPoint",unreadLocals);
        fixture.Property(MenuFixture::Items,4,0);fixture.r.Set(MenuFixture::Options+cfgreflection::disk::StorageBytes,5000);
        fixture.r.Set(MenuFixture::Options+cfgreflection::arraydisk::FunctionParameterBytes,std::uint16_t{4});
        fixture.r.Set(MenuFixture::Options+0xB0,MenuFixture::Base+cfgmenu::startdisk::ScriptExecutor);fixture.r.Set(MenuFixture::Options+0x68,std::uintptr_t{});
        fixture.r.Set(MenuFixture::Options+0xC,0);fixture.r.Set(MenuFixture::Base+cfgprobe::lifetime::ObjectArrayRva+16,registry);
        fixture.r.Set(MenuFixture::Base+cfgprobe::lifetime::ObjectArrayRva+28,1);fixture.r.Set(registry,MenuFixture::Options);fixture.r.Set(registry+16,0);
        std::array<unsigned char,20000> bytes{};bytes.back()=73;fixture.r.Set(code,bytes);
        fixture.r.Set(MenuFixture::Options+cfgmenu::scriptdisk::Script,ScriptHeader{code,20000,65536});
        if(fault==1)fixture.Property(MenuFixture::Items,4,4);
        if(fault==2)fixture.r.Set(MenuFixture::Options+cfgreflection::arraydisk::FunctionParameterBytes,std::uint16_t{8});
        if(fault==3)fixture.r.Set(MenuFixture::Options+0xB0,std::uintptr_t{0xDEAD0000});
        if(fault==4)fixture.Name(MenuFixture::Items,"WrongParameter");
        if(fault==5)fixture.r.Set(MenuFixture::Options+cfgmenu::scriptdisk::Script,ScriptHeader{code,65537,65537});
        GraphReader reader{fixture.r};cfgreflection::Observation<GraphReader> reflection(reader,MenuFixture::Base);cfgreflection::Field field;
        check(reflection.ReadField(MenuFixture::Options,MenuFixture::DonorType,field));
        if(fault==6){reader.trigger=MenuFixture::Options+cfgmenu::scriptdisk::Script;reader.target=MenuFixture::Options+0xB0;reader.replacement=0xDEAD0000;}
        auto out=std::make_unique<GraphSnapshot>();FunctionDiagnostic diagnostic;
        const bool result=Graph(reader,MenuFixture::Base,field,*out,diagnostic);check(result==(fault==0));
        if(result){check(out->fieldsCount==1&&out->script.count==20000&&out->bytes[19999]==73);
            check(reader.reads<256&&std::string_view(diagnostic.stage)=="none");}
        else check(!out->function.object&&out->fieldsCount==0);
    }
    std::printf("Read-only graph snapshot: %s (%u checks; unrelated locals unread; no native call authority).\n",failures?"FAIL":"PASS",checks);return failures?1:0;
}
