#pragma once
#include <array>
#include <cstdint>
namespace cfgspectatorstatus {
struct String {std::uintptr_t data=0;std::int32_t count=0,capacity=0;};
static_assert(sizeof(String)==16);
struct Frame {String text{},mode{};};
static_assert(sizeof(Frame)==32);
inline void NeutralFrame(const Frame& original,Frame& output){
    static constexpr char16_t text[]=u"Spectating";output=original;
    output.text={reinterpret_cast<std::uintptr_t>(text),11,11};
}
inline bool JasonFrame(const std::array<char16_t,96>& text,const Frame& original,Frame& output){
    static constexpr char16_t name[]=u"Jason",prefixed[]=u"Spectating: Jason";
    static constexpr char16_t prefix[]=u"Spectating:";
    bool formatted=true;for(unsigned i=0;i<sizeof(prefix)/sizeof(prefix[0])-1;++i)formatted&=text[i]==prefix[i];
    output=original;
    const auto* selected=formatted?prefixed:name;
    const auto count=formatted?sizeof(prefixed)/sizeof(prefixed[0]):sizeof(name)/sizeof(name[0]);
    output.text={reinterpret_cast<std::uintptr_t>(selected),static_cast<std::int32_t>(count),static_cast<std::int32_t>(count)};
    return true;
}
template<class Dispatch>void Forward(Dispatch&& original,void* object,void* function,void* caller,Frame& replacement,bool accepted){
    // Synchronous borrowed frame only. Never overwrite the caller's owned
    // FString headers: CFG's native wrapper frees those after ProcessEvent.
    original(object,function,accepted?static_cast<void*>(&replacement):caller);
}
template<class Reader,std::size_t N>bool Copy(Reader& reader,std::uintptr_t address,std::array<char16_t,N>& output){
    output={};String s{},again{};
    if(!reader.Read(address,s)||s.count<0||s.count>static_cast<int>(N)||s.capacity<s.count||s.capacity>4096||
       (s.count&&s.data<0x10000))return false;
    std::array<char16_t,N> first{},second{};
    if(s.count&&(!reader.ReadBytes(s.data,first.data(),static_cast<unsigned>(s.count)*2)||first[static_cast<unsigned>(s.count)-1]!=0||
        !reader.ReadBytes(s.data,second.data(),static_cast<unsigned>(s.count)*2)||first!=second))return false;
    if(!reader.Read(address,again)||again.data!=s.data||again.count!=s.count||again.capacity!=s.capacity)return false;
    output=first;return true;
}
}
