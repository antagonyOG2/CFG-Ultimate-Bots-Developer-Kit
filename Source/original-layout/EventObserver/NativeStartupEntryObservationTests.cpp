#include "../RuntimeProbe/NativeStartupEntryObservation.hpp"
#include <map>
#include <cstring>
#include <cstdio>
struct EntryReader {
    std::map<std::uintptr_t,unsigned char> bytes;unsigned reads=0;
    template<class T>void Set(std::uintptr_t at,const T& v){auto* p=reinterpret_cast<const unsigned char*>(&v);for(unsigned i=0;i<sizeof(T);++i)bytes[at+i]=p[i];}
    bool ReadBytes(std::uintptr_t at,void* dst,std::size_t count){++reads;auto* p=static_cast<unsigned char*>(dst);
        for(std::size_t i=0;i<count;++i){const auto it=bytes.find(at+i);if(it==bytes.end())return false;p[i]=it->second;}return true;}
    template<class T>bool Read(std::uintptr_t at,T& out){return ReadBytes(at,&out,sizeof(T));}
};
int main(){using namespace cfgmenu;namespace p=startupentrydisk::profile;namespace d=offlinelifecycledisk;
    constexpr std::uintptr_t module=0x100000,mode=0x200000;unsigned failures=0;
    const auto check=[&](bool ok){if(!ok)++failures;};
    for(unsigned roleIndex=0;roleIndex<2;++roleIndex)for(unsigned fault=0;fault<11;++fault){EntryReader r;
        const auto role=roleIndex?startupretain::Role::Match:startupretain::Role::Intro;
        auto pc=module+(roleIndex?p::MatchReturnPc:p::IntroReturnPc);unsigned stage=99;
        r.Set(module+p::IntroDispatcher,startupentrydisk::IntroDispatcherCode);r.Set(module+p::MatchBridge,startupentrydisk::MatchBridgeCode);
        r.Set(module+p::MatchDispatcher,startupentrydisk::MatchDispatcherCode);r.Set(mode,module+d::OfflineBotsVtable);
        r.Set(module+d::OfflineBotsVtable+0x808,module+p::IntroDispatcher);r.Set(module+d::OfflineBotsVtable+d::IntroSlot,module+d::OfflineBotsIntro958);
        r.Set(module+d::OfflineBotsVtable+d::MatchStartedSlot,module+d::OfflineBotsMatchStarted818);
        r.Set(mode+p::ModeState,std::uint64_t{123});r.Set(module+(roleIndex?p::MatchState:p::IntroState),std::uint64_t{123});
        if(fault==1)++pc;if(fault==2)r.Set(module+p::MatchBridge,std::uint8_t{0});
        if(fault==3)r.Set(mode,module+d::SandboxVtable);if(fault==4)r.Set(mode+p::ModeState,std::uint64_t{124});
        if(fault==5)r.Set(module+(roleIndex?p::MatchState:p::IntroState),std::uint64_t{0});
        if(fault==6)r.Set(module+d::OfflineBotsVtable+d::IntroSlot,module+d::OfflineIntro958);
        if(fault==7)r.Set(module+d::OfflineBotsVtable+d::MatchStartedSlot,module+d::OfflineMatchStarted818);
        if(fault==8)pc=module+p::MatchDispatcher+0x2D; // Not the tail-jump caller's return address.
        check(startupentry::Origin(r,module,mode,role,pc,42,fault==9?43u:42u,fault!=10,stage)==(fault==0));
        if(fault==1||fault==8||fault==9||fault==10)check(!r.reads&&stage==1);
    }
    std::printf("CFG3.5 exact native startup caller/state origin: %u failures; owned memory only.\n",failures);return failures?1:0;
}
