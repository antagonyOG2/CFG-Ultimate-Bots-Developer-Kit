#pragma once
#include "NameObservation.hpp"
namespace cfgobjectpath {
// A bounded, same-snapshot diagnostic only. Zero weak serials are permitted
// only with complete slot/outer/name resamples; this grants no call authority.
template<class R,std::size_t N>bool Observe(R& r,std::uintptr_t module,std::uintptr_t object,std::array<wchar_t,N>& output){
    output={};if(!object)return false;
    cfgprobe::lifetime::ArrayHeader registry{};
    if(!cfgprobe::lifetime::ReadArrayHeader(r,module+cfgprobe::lifetime::ObjectArrayRva,registry))return false;
    std::array<cfgnames::Text,8> names{};std::array<std::uintptr_t,8> seen{};unsigned count=0;
    cfgnames::Observation<R> observation(r,module);
    for(;object&&count<seen.size();++count){
        for(unsigned i=0;i<count;++i)if(seen[i]==object)return false;
        cfgprobe::lifetime::SlotObservation before{},after{};std::uintptr_t outer=0,again=0;
        if(!cfgprobe::lifetime::ReadSlot(r,registry,object,before)||!observation.Object(object,names[count])||
            !r.Read(object+0x20,outer)||!r.Read(object+0x20,again)||again!=outer||
            !cfgprobe::lifetime::ReadSlot(r,registry,object,after)||!(after==before))return false;
        seen[count]=object;object=outer;
    }
    if(object)return false;unsigned used=0;std::array<wchar_t,N> result{};
    for(unsigned i=count;i;--i){if(used){if(used+1>=N)return false;result[used++]=L'.';}
        for(auto c:names[i-1].View()){if(used+1>=N)return false;result[used++]=static_cast<wchar_t>(c);}}
    if(!used)return false;output=result;return true;
}
}
