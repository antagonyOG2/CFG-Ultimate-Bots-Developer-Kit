#pragma once
#include "NativeCounselorPromotion.hpp"
#include "../RuntimeProbe/NativeSoftClassLoad.generated.hpp"
#include "../RuntimeProbe/NameObservation.hpp"
namespace cfgmenu::population {
namespace softdetail {
template<class Reader>bool ChallengePagePacket(Reader& reader,std::uintptr_t module,const promotion::Packet& packet){
    cfgnames::Words name{};std::uintptr_t text=0;std::int32_t count=0,capacity=0;
    std::memcpy(&name,packet.bytes.data()+16,8);std::memcpy(&text,packet.bytes.data()+24,8);
    std::memcpy(&count,packet.bytes.data()+32,4);std::memcpy(&capacity,packet.bytes.data()+36,4);
    if(name.index<=0||name.number||text||count||capacity)return false;
    cfgnames::Observation<Reader> names(reader,module);cfgnames::Text value;
    return names.Decode(name,value)&&value.View()==startupretain::ChallengePagePath;
}
template<class Reader,class Native>bool ScopedLoad(Reader& reader,std::uintptr_t module,promotion::Packet& owned,
    startupretain::Call call,std::uintptr_t& object,Native&& native){
    if(call!=startupretain::Call::LoadClass&&call!=startupretain::Call::FrontendChallengePageClass)return false;
    if(call==startupretain::Call::FrontendChallengePageClass&&
       (!CFG_J22_CHALLENGES_RETENTION||!ChallengePagePacket(reader,module,owned)))return false;
    std::array<std::uintptr_t,1> outputs{};
    return startupretain::Invoke(module,call,std::array<std::uintptr_t,1>{0},outputs,[&](){
        const bool returned=native(object);outputs[0]=object;return returned;});
}
inline bool Load(std::uintptr_t function,promotion::Packet& owned,std::uintptr_t& object)noexcept{
    object=0;
    __try {object=reinterpret_cast<std::uintptr_t(__fastcall*)(void*)>(function)(&owned);return true;}
    __except(EXCEPTION_EXECUTE_HANDLER){return false;}
}
template<class Reader> bool OwnPath(Reader& reader,const promotion::Packet& source,promotion::Packet& owned,std::array<wchar_t,1024>& path){
    static_assert(sizeof(wchar_t)==2,"Native CFG FString uses UTF16");
    std::uintptr_t text=0;std::uint64_t asset=0;std::int32_t count=0,capacity=0;
    std::memcpy(&asset,source.bytes.data()+16,8);
    std::memcpy(&text,source.bytes.data()+24,8);
    std::memcpy(&count,source.bytes.data()+32,4);std::memcpy(&capacity,source.bytes.data()+36,4);
    if(!asset||count<0||count>1024||capacity<count||capacity>65536||(count&&!cfgprobe::ObjectPointer(text)))return false;
    for(int i=0;i<count;++i)if(!reader.Read(text+static_cast<std::uintptr_t>(i)*2,path[static_cast<std::size_t>(i)]))return false;
    if(count&&path[static_cast<std::size_t>(count-1)]!=L'\0')return false;
    owned=source;text=count?reinterpret_cast<std::uintptr_t>(path.data()):0;capacity=count;
    std::memcpy(owned.bytes.data()+24,&text,8);std::memcpy(owned.bytes.data()+36,&capacity,4);
    return true;
}
}
// Source is a plain 40-byte observation. Copy the optional UTF16 subpath into
// owned stack storage so synchronous package loading cannot invalidate it.
// The proved native helper only changes weak-pointer/cache fields in this local
// packet; path calls consume the path and allocate/free their own temporaries.
// Current must verify standalone native pre-match ownership before the call and
// refresh the object-array header after allocation while preserving identity
// serials. The returned UObject is provisional: caller verifies UClass, expected
// ancestry and assigned serial. No fallback selection or re-entry is attempted.
template<class Reader,class Current> std::uintptr_t ResolveClass(Reader& reader,std::uintptr_t module,
    const promotion::Packet& source,DWORD thread,Current&& current,bool exactImageVerified,
    startupretain::Call loadCall=startupretain::Call::LoadClass){
    namespace d=softloaddisk;
    if(!exactImageVerified||!thread||GetCurrentThreadId()!=thread||module!=reinterpret_cast<std::uintptr_t>(GetModuleHandleW(nullptr))||!current())return 0;
    promotion::Packet owned{};std::array<wchar_t,1024> path{};
    if(!softdetail::OwnPath(reader,source,owned,path)||
       !promotion::detail::Code(module,module+d::LoadSoftObject,d::LoadSoftObjectCode)||
       !promotion::detail::Code(module,module+d::LoadSoftPath,d::LoadSoftPathCode)||
       !promotion::detail::Code(module,module+d::StaticLoadObject,d::StaticLoadObjectCode)||!current())return 0;
    std::uintptr_t object=0;
    if(!softdetail::ScopedLoad(reader,module,owned,loadCall,object,[&](std::uintptr_t& output){
           return softdetail::Load(module+d::LoadSoftObject,owned,output);})||
       GetCurrentThreadId()!=thread||!current()||!cfgprobe::ObjectPointer(object))return 0;
    return object;
}
}
