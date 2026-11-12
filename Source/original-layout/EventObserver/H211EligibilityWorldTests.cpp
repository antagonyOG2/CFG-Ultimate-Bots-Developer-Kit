#define CFG_H1_JASON 1
#include <cstdint>
#include <cstdio>
namespace cfgprobe {constexpr unsigned WorldNetDriver=0x40;}
struct Read {bool offline=true;};
namespace cfgobserver {bool Same(Read& r,std::uintptr_t,std::uintptr_t){return r.offline;}}
bool active=false;
bool HostedRoutineActive(){return active;}
struct Fixture {
 Read r_;struct {std::uintptr_t world=1;}plan;int context=0;
 bool observation=true,tickLease=false;unsigned reads=0,dispatchProofs=0;
 bool SummaryObservationWorld(){++reads;return observation;}
 bool GameplayWorld(int&){++dispatchProofs;return tickLease;}
#include "EligibilityWorldRuntimeMembers.hpp"
};
int main(){unsigned n=0,bad=0;auto check=[&](bool ok){++n;if(!ok)++bad;};
 Fixture f;active=true;f.r_.offline=false;
 check(f.EligibilityWorldCurrent(true)&&f.reads==1&&!f.dispatchProofs); // actual native callback: NO completed tick
 check(!f.EligibilityWorldCurrent(false)&&f.dispatchProofs==1); // mod dispatch still needs the hosted tick proof
 f.observation=false;check(!f.EligibilityWorldCurrent(true));
 f.tickLease=true;check(f.EligibilityWorldCurrent(false));
 active=false;check(!f.EligibilityWorldCurrent(true)&&!f.EligibilityWorldCurrent(false)); // NetDriver blocks offline
 f.r_.offline=true;check(f.EligibilityWorldCurrent(true)&&f.EligibilityWorldCurrent(false));
 std::printf("H211 actual eligibility-world member: %u checks, %u failures; engine query without fake tick, dispatch still strict, stale host/offline guarded.\n",n,bad);return bad?1:0;
}
