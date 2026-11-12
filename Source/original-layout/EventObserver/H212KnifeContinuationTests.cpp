#include <cstdint>
#include <cstdio>
#define CFG_H1_JASON 1
bool hosted=false;
bool HostedRoutineActive(){return hosted;}
struct Knife {unsigned calls=0;template<class T>void Tick(T&,std::uint64_t){++calls;}}bundleKnife;
struct Host {int a=0;bool busy=false;bool KnifeSequenceActive(){return busy;}
#include "DonorKnifeContinuationMembers.hpp"
};
int main(){Host h;unsigned n=0,bad=0;auto check=[&](bool v){++n;if(!v)++bad;};
 hosted=true;check(!h.TickJasonAIKnifeSequenceOnGameThread(1000)&&!bundleKnife.calls);
 h.busy=true;check(h.TickJasonAIKnifeSequenceOnGameThread(1000)&&bundleKnife.calls==1);
 hosted=false;check(h.TickJasonAIKnifeSequenceOnGameThread(1000)&&bundleKnife.calls==2);
 std::printf("H212 actual knife continuation member: %u checks, %u failures; hosted and offline use the same J2.19 continuation.\n",n,bad);return bad?1:0;
}
