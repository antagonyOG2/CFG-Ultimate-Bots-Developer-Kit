#include "NativeOfflineStartup.hpp"
#include <cassert>
#include <cstdio>
struct Reader {
    unsigned reads=0;
    template<class T>bool Read(std::uintptr_t,T&){++reads;return false;}
};
int main(){
    using namespace cfgmenu;
    promotion::Packet counselor{},killer{};counselor.bytes[16]=1;killer.bytes[16]=2;
    const freshgame::Source source{0x100000,0x200000,0x210000,0x220000,0x230000,0x240000,GetCurrentThreadId(),true};
    for(unsigned fault=0;fault<7;++fault){Reader r;offlinestartup::Startup<Reader> s(r);auto c=counselor,k=killer;auto from=source;unsigned bots=4,skill=0;
        if(fault==0)from.nativeOffline=false;if(fault==1)from.thread=0;if(fault==2)from.frontendWorld=0;
        if(fault==3)c.bytes[24]=1;if(fault==4)k.bytes[16]=0;if(fault==5)bots=32;if(fault==6)skill=4;
        assert(!s.Arm(from,c,k,bots,skill)&&r.reads==0&&s.Status().stage==offlinestartup::Stage::Empty);
    }
    Reader r;offlinestartup::Startup<Reader> s(r);
    assert(!s.CanResetEpoch()&&!s.ResetEpoch());
    {Reader invalid;offlinestartup::Startup<Reader> over(invalid);
     assert(!over.Arm(source,counselor,killer,10,2)&&invalid.reads==0);}
    assert(s.Arm(source,counselor,killer,4,2));assert(!s.Arm(source,counselor,killer,4,2));
    assert(!s.CanResetEpoch()&&!s.ResetEpoch());
    assert(!s.Claimed(0x250000,source.thread+1)&&s.Claimed(0x250000,source.thread));
    assert(!s.Intro(0x250000)&&s.Status().stage==offlinestartup::Stage::Refused&&s.Status().failure==110);
    const auto reads=r.reads;assert(s.Claimed(0x250000,source.thread)&&!s.Claimed(0x260000,source.thread));
    assert(!s.Intro(0x250000)&&!s.Match(0x250000)&&r.reads==reads);
    assert(!s.Arm(source,counselor,killer,4,2));
    assert(!s.CanResetEpoch()&&!s.ResetEpoch());
    std::puts("Native startup arming, exact claim, refusal/no-stock-fallthrough and no-retry tests PASS; no native game calls.");
}
