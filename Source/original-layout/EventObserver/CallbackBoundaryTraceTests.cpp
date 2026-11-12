#include "CallbackBoundaryTrace.hpp"
#include <cstdio>
using namespace cfgcallbacktrace;
#define CHECK(c) do{if(!(c)){std::printf("FAIL %d: %s\n",__LINE__,#c);return 1;}}while(false)
int main(){
    Capture c;c.Arm(0x10000,0x20000,42);Sample sample;sample.valid=1;sample.tick=0x30000;
    CHECK(c.Begin(0x20000,0x30000,0x40000,43,0,100,sample)==Capacity);
    CHECK(c.Begin(0x20001,0x30000,0x40000,42,0,100,sample)==Capacity);
    auto bad=sample;bad.valid=0;
    Capture unknown;unknown.Arm(0x10000,0x20000,42);
    auto u=unknown.Begin(0x20000,0x30001,0x40000,42,0,100,bad);
    CHECK(u==0);unknown.Finish(u,true,bad);CHECK(unknown.CompletePrefix()==1);
    const auto root=c.Begin(0x20000,0x30000,0x40000,42,0,100,sample);CHECK(root==0);
    const auto child=c.Begin(1,2,3,42,1,101,sample);CHECK(child==1);
    c.Finish(child,true,sample);CHECK(c.CompletePrefix()==0);
    c.Finish(root,false,bad);CHECK(c.CompletePrefix()==2);
    c.Data().published=c.CompletePrefix();const auto first=c.Data().events[0];
    CHECK(!first.returned&&first.finished&&first.before.valid&&!first.after.valid&&first.returnPc==0x40000);
    for(unsigned i=2;i<Capacity;++i){const auto n=c.Begin(0x20000,0x30000,0x50000,42,0,100+i,sample);CHECK(n==i);c.Finish(n,true,sample);c.Data().published=c.CompletePrefix();}
    CHECK(c.Data().published==Capacity&&c.Begin(0x20000,0x30000,0x50000,42,0,200,sample)==Capacity);
    CHECK(c.Data().events[0].finished==first.finished&&c.Data().events[0].returned==first.returned);
    Capture timeout;timeout.Arm(0x10000,0x20000,42);
    auto n=timeout.Begin(0x20000,0x30000,0,42,0,100,sample);timeout.Finish(n,true,sample);
    CHECK(timeout.Begin(0x20000,0x30000,0,42,0,5101,sample)==Capacity);
    std::puts("PASS bounded callback trees publish only completed prefixes; wrong owner/thread, unknown function/cache, timeout, exception outcome and capacity covered; no process access");return 0;
}
