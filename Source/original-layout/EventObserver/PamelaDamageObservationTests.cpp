#define main ExistingDamageGateSuiteMain
#pragma warning(push)
#pragma warning(disable:4715) // reused fixture's main has implicit main-only return0; never called here
#include "KillDamageGateTests.cpp"
#pragma warning(pop)
#undef main
#include "JasonInstanceObservation.hpp"
#include "PamelaDamageObservation.hpp"
struct EventFixture:Fixture {
    static constexpr std::uintptr_t Module=0x140000000,Pawn=0x410000,Point=0x212000,PointOverride=0x213000,Radial=0x214000,RadialOverride=0x215000;
    void Name(unsigned index,const char* text){
        const std::uintptr_t entry=0x600000+index*0x100;
        reader.Put(0x510000+index*8,entry);reader.Put(entry,std::uint32_t{});
        std::array<unsigned char,128> bytes{};std::memcpy(bytes.data(),text,std::strlen(text));
        reader.Put(entry+cfgnames::disk::EntryText,bytes);
    }
    void Event(std::uintptr_t fn,std::uintptr_t owner,std::uintptr_t parent,unsigned name,unsigned bytes){
        Function(fn,owner,parent);reader.Put(fn+cfgnames::disk::ObjectName,cfgnames::Words{static_cast<int>(name),0});
        reader.Put(fn+cfgreflection::disk::StorageBytes,bytes);
        reader.Put(fn+cfgreflection::arraydisk::FunctionParameterBytes,static_cast<std::uint16_t>(bytes));
    }
    EventFixture(){
        Object(FunctionClass,ClassClass);Object(ClassClass,ClassClass);Object(Pawn,Root);
        reader.Put(Module+cfgkill::disk::ReceiveAnyDamage.cache,Base);
        reader.Put(Module+cfgkill::disk::ReceiveAnyDamage.ownerReflection,Actor);
        reader.Put(Module+cfgkill::disk::ReceiveAnyDamage.ownerConstructor,Actor);
        reader.Put(Module+cfgnames::disk::Global,std::uintptr_t{0x500000});
        reader.Put(0x500000+cfgnames::disk::ElementCount,std::array<int,2>{20000,2});
        reader.Put(0x500000,std::uintptr_t{0x510000});reader.Put(0x500008,std::uintptr_t{0x530000});
        Name(42,"ReceiveAnyDamage");Name(43,"ReceivePointDamage");Name(44,"ReceiveRadialDamage");Name(45,"ReceiveTick");
        Event(Point,Actor,0,43,80);Event(PointOverride,Root,Point,43,80);
        Event(Radial,Actor,0,44,96);Event(RadialOverride,Root,Radial,44,96);
    }
    bool Observe(std::uintptr_t fn){reader.reads=0;return cfgkill::pamela::Event(reader,array,Module,Pawn,fn);}
};
int main(){unsigned checks=0,peak=0;const auto check=[&](bool value){++checks;assert(value);};
    for(const auto fn:{EventFixture::Base,EventFixture::Override,EventFixture::Point,EventFixture::PointOverride,EventFixture::Radial,EventFixture::RadialOverride}){
        EventFixture f;check(f.Observe(fn));peak=(std::max)(peak,f.reader.reads);check(f.reader.reads<=1536);
    }
    for(unsigned fault=0;fault<12;++fault){EventFixture f;
        switch(fault){
        case 0:f.reader.Put(EventFixture::Module+cfgkill::disk::ReceiveAnyDamage.cache,std::uintptr_t{});break;
        case 1:f.reader.Put(EventFixture::PointOverride+cfgprobe::ObjectClass,EventFixture::ClassClass);break;
        case 2:f.reader.Put(EventFixture::PointOverride+cfgreflection::disk::FunctionFlags,std::uint32_t{});break;
        case 3:f.reader.Put(EventFixture::PointOverride+cfgreflection::disk::FunctionFlags,cfgkill::disk::ReceiveAnyDamageFlags|0x40u);break;
        case 4:f.reader.Put(EventFixture::PointOverride+cfgprobe::ClassSuper,EventFixture::PointOverride);break;
        case 5:f.reader.Put(EventFixture::Point+cfgnames::disk::ObjectName,cfgnames::Words{44,0});break;
        case 6:f.reader.Put(EventFixture::PointOverride+0x20,EventFixture::FunctionClass);break;
        case 7:f.reader.Put(EventFixture::PointOverride+cfgreflection::arraydisk::FunctionParameterBytes,std::uint16_t{});break;
        case 8:f.reader.Put(EventFixture::Point+cfgreflection::arraydisk::FunctionParameterBytes,std::uint16_t{79});break;
        case 9:{std::int32_t index=0;check(f.reader.Read(EventFixture::PointOverride+0xC,index));
            f.reader.Put(f.array.data+24*static_cast<std::uintptr_t>(index)+8,std::uint32_t{0x20000000});break;}
        case 10:f.reader.Put(EventFixture::PointOverride+cfgnames::disk::ObjectName,cfgnames::Words{45,0});break;
        case 11:f.reader.Put(EventFixture::Base+cfgreflection::disk::FunctionFlags,std::uint32_t{});break;
        }
        check(!f.Observe(EventFixture::PointOverride));
    }
    std::printf("Actual B1 Any/Point/Radial function observation: %u checks PASS, peak%u/1536 reads, zero native calls or writes.\n",checks,peak);
    return 0;
}
