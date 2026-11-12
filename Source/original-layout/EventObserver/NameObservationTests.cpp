#include "../RuntimeProbe/NameObservation.hpp"
#include <cstring>
#include "OwnedByteReader.hpp"
#include <cstdio>
#include <cassert>
using Reader=OwnedByteReader;
struct Fixture {
    static constexpr std::uintptr_t Base=0x100000,Table=0x200000,Chunk=0x300000,Entry=0x400000,Object=0x500000;
    Reader r;
    Fixture(){
        r.Set(Base+cfgnames::disk::Global,Table);r.Set(Table+cfgnames::disk::ElementCount,std::array<int,2>{20000,2});
        r.Set(Table,std::uintptr_t{Chunk});r.Set(Table+8,std::uintptr_t{Chunk+0x20000});
        r.Set(Chunk+8*12,Entry);r.Set(Chunk+0x20000+8*4,Entry);
        r.Set(Entry,std::uint32_t{0});r.Set(Object+cfgnames::disk::ObjectName,cfgnames::Words{12,0});
        std::array<unsigned char,128> text{};const char* value="OfflineBotsSettingsMenuWidget_C";std::memcpy(text.data(),value,std::strlen(value));r.Set(Entry+cfgnames::disk::EntryText,text);
    }
};
int main(){
    using namespace cfgnames;Fixture f;Observation<Reader> names(f.r,Fixture::Base);Text text;
    assert(names.Object(Fixture::Object,text)&&text.View()==u"OfflineBotsSettingsMenuWidget_C");assert(f.r.reads<25);
    assert(names.Decode({12,2},text)&&text.View()==u"OfflineBotsSettingsMenuWidget_C_1"&&text.Base()==u"OfflineBotsSettingsMenuWidget_C");
    assert(names.Decode({16388,0},text));
    assert(!names.Decode({-1,0},text)&&!text.count);assert(!names.Decode({20000,0},text));
    assert(!names.Decode({12,-1},text));assert(names.Decode({12,2147483647},text)&&text.View()==u"OfflineBotsSettingsMenuWidget_C_2147483646");
    Fixture wide;wide.r.Set(Fixture::Entry,std::uint32_t{1});std::array<char16_t,128> unicode{};
    const std::u16string_view label=u"Counselor_\u00e9";std::memcpy(unicode.data(),label.data(),label.size()*2);wide.r.Set(Fixture::Entry+disk::EntryText,unicode);
    Observation<Reader> wideNames(wide.r,Fixture::Base);assert(wideNames.Decode({12,0},text)&&text.Base()==label);
    Fixture longName;std::array<unsigned char,128> longText;longText.fill('x');longName.r.Set(Fixture::Entry+disk::EntryText,longText);
    Observation<Reader> longNames(longName.r,Fixture::Base);assert(!longNames.Decode({12,0},text)&&!text.count);
    Fixture replaced;replaced.r.changeAt=Fixture::Entry+disk::EntryText;replaced.r.changeSlot=Fixture::Chunk+8*12;replaced.r.newPointer=0x600000;
    Observation<Reader> changing(replaced.r,Fixture::Base);assert(!changing.Decode({12,0},text)&&!text.count);
    Fixture malformed;malformed.r.Set(Fixture::Table+disk::ElementCount,std::array<int,2>{20000,1});
    Observation<Reader> wrongChunk(malformed.r,Fixture::Base);assert(!wrongChunk.Decode({16388,0},text));
    Fixture empty;empty.r.Set(Fixture::Entry+disk::EntryText,std::array<unsigned char,128>{});Observation<Reader> emptyNames(empty.r,Fixture::Base);assert(!emptyNames.Decode({12,0},text));
    Fixture padding;std::array<unsigned char,128> padded{};padded[0]='x';padded[2]=255;padding.r.Set(Fixture::Entry+disk::EntryText,padded);
    Observation<Reader> paddingNames(padding.r,Fixture::Base);assert(paddingNames.Decode({12,0},text)&&text.View()==u"x");
    std::puts("CFG name observations PASS: native chunk layout, ANSI/UTF16, suffix, extents, changed references, no allocations/native calls/game access.");
    return 0;
}
