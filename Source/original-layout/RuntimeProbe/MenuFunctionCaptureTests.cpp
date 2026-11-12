#define main ExistingWidgetSchemaTests
#include "../EventObserver/SetupWidgetObservationTests.cpp"
#undef main
#include "MenuScriptObservation.hpp"
#include "MenuOptionsScript.hpp"
struct ByteAdapter {
    OwnedByteReader& r;
    template<class T>bool Read(std::uintptr_t address,T& value){return r.Read(address,value);}
    bool ReadBytes(std::uintptr_t address,void* output,std::size_t size){auto* p=static_cast<unsigned char*>(output);for(std::size_t i=0;i<size;++i)if(!r.Read(address+i,p[i]))return false;return true;}
};
int main(){unsigned failures=0;auto check=[&](bool b){if(!b)++failures;};
    for(unsigned fault=0;fault<8;++fault){MenuFixture f;constexpr std::uintptr_t data=0x800000,script=0x810000;
        f.r.Set(MenuFixture::Base+cfgprobe::lifetime::ObjectArrayRva+0x10,data);f.r.Set(MenuFixture::Base+cfgprobe::lifetime::ObjectArrayRva+0x1C,1);
        f.r.Set(MenuFixture::Options+0xC,0);f.r.Set(data,MenuFixture::Options);f.r.Set(data+0x10,0);
        f.r.Set(MenuFixture::Options+0xB0,std::uintptr_t{0x7AA100});f.r.Set(MenuFixture::Options+0x68,std::uintptr_t{0});
        f.r.Set(MenuFixture::Options+cfgmenu::scriptdisk::Script,cfgmenu::capture::ScriptHeader{script,4,4});
        f.r.Set(script,std::array<unsigned char,4>{0x1,0x2,0x3,0x4});
        if(fault==1)f.r.Set(MenuFixture::Items+0x20,MenuFixture::Type);
        if(fault==2)f.r.Set(MenuFixture::Selected+0x28,MenuFixture::Items);
        if(fault==3)f.r.Set(data,MenuFixture::Start);
        if(fault==4)f.r.Set(MenuFixture::Options+cfgreflection::arraydisk::FunctionParameterBytes,std::uint16_t{513});
        if(fault>=5){const char* kinds[]{"StructProperty","NameProperty","UnknownProperty"};f.Name(MenuFixture::ArrayProperty,kinds[fault-5]);}
        ByteAdapter reader{f.r};cfgmenu::capture::FunctionEvidence out;
        const bool ok=cfgmenu::capture::Function(reader,MenuFixture::Base,MenuFixture::DonorType,u"SetOptions",out);
        check(ok==(fault==0||fault>=5));if(ok)check(out.fieldsCount==2&&out.parameterBytes==24&&out.storageBytes==24&&out.nativeExecutor==0x7AA100&&out.destructorLink==0&&out.script.count==4&&out.bytes[3]==4&&out.slot.serial==0);
        else check(out.function.object==0&&out.fieldsCount==0);
    }
    {
        MenuFixture f;constexpr std::uintptr_t registry=0x800000,script=0x810000,locals=0x900000;
        f.r.Set(MenuFixture::Base+cfgprobe::lifetime::ObjectArrayRva+16,registry);f.r.Set(MenuFixture::Base+cfgprobe::lifetime::ObjectArrayRva+28,1);
        f.r.Set(MenuFixture::Options+0xC,0);f.r.Set(registry,MenuFixture::Options);f.r.Set(registry+16,0);
        f.r.Set(MenuFixture::Options+0xB0,std::uintptr_t{0x7AA100});f.r.Set(MenuFixture::Options+0x68,std::uintptr_t{0});
        f.r.Set(MenuFixture::Options+cfgmenu::scriptdisk::Script,cfgmenu::capture::ScriptHeader{script,4,4});
        f.r.Set(script,std::array<unsigned char,4>{1,2,3,4});
        f.r.Set(MenuFixture::Options+cfgreflection::disk::Children,locals);
        f.r.Set(MenuFixture::Options+cfgreflection::disk::StorageBytes,160);
        f.r.Set(MenuFixture::Options+cfgreflection::arraydisk::FunctionParameterBytes,std::uint16_t{4});
        for(unsigned i=0;i<40;++i){char label[32]{};std::snprintf(label,sizeof(label),"BlueprintLocal_%u",i);
            f.Field(locals+0x100*i,MenuFixture::Options,MenuFixture::IntProperty,label,i==39?0:locals+0x100*(i+1));
            f.Property(locals+0x100*i,4,static_cast<int>(4*i));
        }
        ByteAdapter reader{f.r};cfgmenu::capture::FunctionEvidence runtime;
        cfgmenu::capture::DiagnosticFunctionEvidence diagnostic;
        check(!cfgmenu::capture::Function(reader,MenuFixture::Base,MenuFixture::DonorType,u"SetOptions",runtime));
        check(cfgmenu::capture::Function(reader,MenuFixture::Base,MenuFixture::DonorType,u"SetOptions",diagnostic)&&diagnostic.fieldsCount==40);
        // Increasing diagnostic local capacity does not increase runtime ABI
        // capacity or turn extra Blueprint locals into SetOptions call inputs.
        cfgmenu::capture::OptionsOperands operands;
        check(!cfgmenu::capture::OptionsScript(diagnostic,operands));
    }
    for(unsigned count:{129u,257u}){
        MenuFixture f;constexpr std::uintptr_t registry=0x800000,script=0x810000,locals=0x900000;
        f.r.Set(MenuFixture::Table+0x400,std::array<int,2>{1024,1});
        f.r.Set(MenuFixture::Base+cfgprobe::lifetime::ObjectArrayRva+16,registry);f.r.Set(MenuFixture::Base+cfgprobe::lifetime::ObjectArrayRva+28,1);
        f.r.Set(MenuFixture::Options+0xC,0);f.r.Set(registry,MenuFixture::Options);f.r.Set(registry+16,0);
        f.r.Set(MenuFixture::Options+0xB0,std::uintptr_t{0x7AA100});f.r.Set(MenuFixture::Options+0x68,std::uintptr_t{0});
        f.r.Set(MenuFixture::Options+cfgmenu::scriptdisk::Script,cfgmenu::capture::ScriptHeader{script,4,4});
        f.r.Set(script,std::array<unsigned char,4>{1,2,3,4});
        f.r.Set(MenuFixture::Options+cfgreflection::disk::Children,locals);
        f.r.Set(MenuFixture::Options+cfgreflection::disk::StorageBytes,static_cast<int>(count*4));
        f.r.Set(MenuFixture::Options+cfgreflection::arraydisk::FunctionParameterBytes,std::uint16_t{4});
        for(unsigned i=0;i<count;++i){char label[32]{};std::snprintf(label,sizeof(label),"PickerLocal_%u",i);
            f.Field(locals+0x100*i,MenuFixture::Options,MenuFixture::IntProperty,label,i+1==count?0:locals+0x100*(i+1));
            f.Property(locals+0x100*i,4,static_cast<int>(4*i));}
        ByteAdapter reader{f.r};cfgmenu::capture::DiagnosticFunctionEvidence small;
        cfgmenu::capture::FunctionDiagnostic detail;
        check(!cfgmenu::capture::KnownFunction(reader,MenuFixture::Base,small.function,u"SetOptions",small,&detail));
        check(!cfgmenu::capture::Function(reader,MenuFixture::Base,MenuFixture::DonorType,u"SetOptions",small));
        auto large=std::make_unique<cfgmenu::capture::PickerFunctionEvidence>();cfgreflection::Field field;
        cfgreflection::Observation<ByteAdapter> reflection(reader,MenuFixture::Base);
        check(reflection.Find(MenuFixture::DonorType,u"SetOptions",field));
        const bool observed=cfgmenu::capture::KnownFunction(reader,MenuFixture::Base,field,u"SetOptions",*large,&detail);
        check(observed==(count==129));
        if(observed)check(large->fieldsCount==129&&std::string_view(detail.stage)=="none"&&detail.scriptBytes==4);
        else check(!large->function.object&&std::string_view(detail.stage)=="local_field_limit"&&detail.fields==256);
    }
    std::printf("MenuFunctionCaptureTests: %u failures; function/parameter/script snapshot, serial0 labelled only\n",failures);return failures?1:0;
}
