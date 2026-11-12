#define main ObjectiveAttachmentFixtureMain
#include "SetupWidgetObservationTests.cpp"
#undef main
#include "../Core/SpectatorStatusObservation.hpp"
#include <algorithm>
#include <string_view>
int main(){unsigned failures=0;const auto check=[&](bool okay){if(!okay)++failures;};
    {cfgspectatorstatus::Frame caller{{0x20000,9,12},{0x30000,4,8}},replacement{};const auto saved=caller;
     std::array<char16_t,96> text{};const char16_t prefix[]=u"Spectating: Other";
     std::copy(std::begin(prefix),std::end(prefix),text.begin());
     check(cfgspectatorstatus::JasonFrame(text,caller,replacement));
     unsigned calls=0;
     const auto dispatch=[&](void* object,void* function,void* p){++calls;
       check(object==reinterpret_cast<void*>(1)&&function==reinterpret_cast<void*>(2));
       const auto& frame=*static_cast<cfgspectatorstatus::Frame*>(p);
       check(std::memcmp(&frame.mode,&saved.mode,sizeof(saved.mode))==0);
       if(p!=&caller)check(std::u16string_view(reinterpret_cast<const char16_t*>(frame.text.data))==u"Spectating: Jason");};
     for(unsigned i=0;i<250;++i)cfgspectatorstatus::Forward(dispatch,reinterpret_cast<void*>(1),reinterpret_cast<void*>(2),&caller,replacement,true);
     check(calls==250&&std::memcmp(&caller,&saved,sizeof(saved))==0);
     cfgspectatorstatus::Forward(dispatch,reinterpret_cast<void*>(1),reinterpret_cast<void*>(2),&caller,replacement,false);
     check(calls==251&&std::memcmp(&caller,&saved,sizeof(saved))==0);
     text={};check(cfgspectatorstatus::JasonFrame(text,caller,replacement)&&replacement.text.count==6&&replacement.text.capacity==6);}
    struct Reader:OwnedByteReader{bool ReadBytes(std::uintptr_t at,void* target,unsigned count){auto* out=static_cast<unsigned char*>(target);
        for(unsigned i=0;i<count;++i)if(!Read(at+i,out[i]))return false;return true;}}r;
    constexpr std::uintptr_t address=0xB00000,text=0xB10000;
    std::array<char16_t,8> out{};const std::array<char16_t,8> source{{u'J',u'a',u's',u'o',u'n',0,0,0}};
    r.Set(address,cfgspectatorstatus::String{text,6,8});r.Set(text,source);
    check(cfgspectatorstatus::Copy(r,address,out)&&out==source);
    r.Set(address,cfgspectatorstatus::String{text,9,9});check(!cfgspectatorstatus::Copy(r,address,out)&&out==std::array<char16_t,8>{});
    r.Set(address,cfgspectatorstatus::String{text,6,5});check(!cfgspectatorstatus::Copy(r,address,out));
    r.Set(address,cfgspectatorstatus::String{text,-1,8});check(!cfgspectatorstatus::Copy(r,address,out));
    r.Set(address,cfgspectatorstatus::String{0,0,0});check(cfgspectatorstatus::Copy(r,address,out)&&out==std::array<char16_t,8>{});
    r.Set(address,cfgspectatorstatus::String{text,5,8});check(!cfgspectatorstatus::Copy(r,address,out));
    std::printf("Native spectator status FString bounded copy/refusal: %s (%u failures); no native calls/writes.\n",failures?"FAIL":"PASS",failures);
    return failures?1:0;
}
