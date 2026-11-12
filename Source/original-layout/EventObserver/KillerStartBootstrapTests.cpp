#include "../Core/KillerStartBootstrap.hpp"
#include <cassert>
#include <cstdio>
struct Host {
    std::uintptr_t reflection=0,constructor=0;bool current=true,allow=true,ambiguous=false;
    unsigned getters=0,factories=0;
    bool Current(){return current;}
    bool Caches(std::uintptr_t& a,std::uintptr_t& b){a=reflection;b=constructor;return true;}
    bool InitializeClass(bool getter,std::uintptr_t& result){
        if(getter)++getters;else ++factories;
        if(!allow)return false;result=0x10000;if(getter)constructor=result;else reflection=result;
        return !ambiguous;
    }
};
int main(){using namespace cfgkillerstart;
    {Host h;Bootstrap b;assert(b.Step(h)==BootstrapState::Pending&&h.getters==1&&h.factories==0);
        assert(b.Step(h)==BootstrapState::Pending&&h.factories==1);assert(b.Step(h)==BootstrapState::Ready&&b.calls==2);
        assert(b.Step(h)==BootstrapState::Ready&&b.calls==2);}
    {Host h;h.constructor=0x10000;Bootstrap b;assert(b.Step(h)==BootstrapState::Pending&&h.factories==1&&!h.getters);assert(b.Step(h)==BootstrapState::Ready);}
    {Host h;h.reflection=h.constructor=0x10000;Bootstrap b;assert(b.Step(h)==BootstrapState::Ready&&!b.calls);}
    {Host h;h.reflection=0x10000;h.constructor=0x20000;Bootstrap b;assert(b.Step(h)==BootstrapState::Refused&&!b.calls);}
    for(unsigned failure=0;failure<3;++failure){Host h;Bootstrap b;
        if(!failure)h.current=false;else if(failure==1)h.allow=false;else h.ambiguous=true;
        assert(b.Step(h)==BootstrapState::Refused);const auto calls=b.calls;assert(b.Step(h)==BootstrapState::Refused&&b.calls==calls);}
    std::puts("Killer start native-class bootstrap PASS: separate phases, no repeated/ambiguous calls; no game access.");
}
