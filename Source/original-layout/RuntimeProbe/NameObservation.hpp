#pragma once
#include "NameBindings.generated.hpp"
#include "ObjectLifetime.hpp"
#include <array>
#include <cstdint>
#include <string_view>
namespace cfgnames {
struct Words {
    std::int32_t index=0,number=0;
    bool operator==(const Words& b)const noexcept{return index==b.index&&number==b.number;}
};
static_assert(sizeof(Words)==8);
template<std::size_t Capacity>struct SizedText {
    std::array<char16_t,Capacity> data{};
    unsigned count=0,baseCount=0;
    std::u16string_view View()const noexcept{return {data.data(),count};}
    std::u16string_view Base()const noexcept{return {data.data(),baseCount};}
};
using Text=SizedText<160>;
using PathText=SizedText<512>; // opt-in full asset paths; normal labels keep their old127 bound
// A bounded, read-only name observation, not a GC pin or call permission. UE's
// converters allocate FString storage; this decoder deliberately never calls
// them and never frees or installs any native string/table storage.
template<class Reader> class Observation {
    Reader& r_;std::uintptr_t module_;
    static bool Pointer(std::uintptr_t p){return cfgprobe::ObjectPointer(p);}
public:
    Observation(Reader& reader,std::uintptr_t module):r_(reader),module_(module){}
    template<std::size_t Capacity>bool Decode(const Words& name,SizedText<Capacity>& output){
        static_assert(Capacity==160||Capacity==512,"Only reviewed bounded label/path decoders");
        constexpr unsigned limit=Capacity==160?128u:512u;
        output={};SizedText<Capacity> result;
        if(name.index<0||name.number<0)return false;
        struct Counts{std::int32_t entries,chunks;bool operator==(const Counts& b)const{return entries==b.entries&&chunks==b.chunks;}} counts{},again{};
        std::uintptr_t table=0,chunk=0,entry=0,fresh=0;std::uint32_t header=0,headerAgain=0;
        if(!r_.Read(module_+disk::Global,table)||!Pointer(table)||!r_.Read(table+disk::ElementCount,counts)||
            counts.entries<=0||counts.entries>static_cast<int>(disk::ChunkSize*disk::MaxChunks)||
            counts.chunks<=0||counts.chunks>static_cast<int>(disk::MaxChunks)||name.index>=counts.entries)return false;
        const auto chunkIndex=static_cast<unsigned>(name.index)/disk::ChunkSize;
        const auto entryIndex=static_cast<unsigned>(name.index)%disk::ChunkSize;
        if(chunkIndex>=static_cast<unsigned>(counts.chunks)||!r_.Read(table+8*chunkIndex,chunk)||!Pointer(chunk)||
            !r_.Read(chunk+8*entryIndex,entry)||!Pointer(entry)||!r_.Read(entry,header))return false;
        bool terminated=false;
        // Eight characters per read. Names exceeding 127 characters are refused
        // rather than truncated into a potentially different reflection label.
        for(unsigned at=0;at<limit&&!terminated;at+=8){
            std::array<char16_t,8> wide{};std::array<unsigned char,8> ansi{};
            if(header&1){if(!r_.Read(entry+disk::EntryText+2*at,wide))return false;}
            else{if(!r_.Read(entry+disk::EntryText+at,ansi))return false;
                for(unsigned i=0;i<8;++i)wide[i]=static_cast<char16_t>(ansi[i]);}
            for(auto c:wide){if(!c){terminated=true;break;}
                // Encoding of non-ASCII narrow entries is not established by
                // the native wide-bit evidence. Reject them, not the padding
                // bytes following the actual terminator in this read block.
                if(result.count==limit-1||c<32||(!(header&1)&&c>127))return false;result.data[result.count++]=c;}
        }
        if(!terminated||!result.count)return false;
        result.baseCount=result.count;
        if(name.number){
            if(result.count+12>=Capacity)return false;
            result.data[result.count++]=u'_';std::array<char16_t,10> digits{};unsigned used=0;
            auto value=static_cast<unsigned>(name.number-1);
            do{digits[used++]=static_cast<char16_t>(u'0'+value%10);value/=10;}while(value);
            while(used)result.data[result.count++]=digits[--used];
        }
        // GNames is append-only in ordinary use, but changes during observation
        // are refused. The caller may retry at its next paced discovery event.
        if(!r_.Read(module_+disk::Global,fresh)||fresh!=table||!r_.Read(table+disk::ElementCount,again)||!(again==counts)||
            !r_.Read(table+8*chunkIndex,fresh)||fresh!=chunk||!r_.Read(chunk+8*entryIndex,fresh)||fresh!=entry||
            !r_.Read(entry,headerAgain)||headerAgain!=header)return false;
        output=result;return true;
    }
    template<std::size_t Capacity>bool Object(std::uintptr_t object,SizedText<Capacity>& output){
        output={};Words name{},again{};SizedText<Capacity> result;
        if(!Pointer(object)||!r_.Read(object+disk::ObjectName,name)||!Decode(name,result)||
            !r_.Read(object+disk::ObjectName,again)||!(again==name))return false;
        output=result;return true;
    }
};
}
