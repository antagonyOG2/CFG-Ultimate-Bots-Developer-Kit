#include <Windows.h>
#include <cstdio>
#include "NativePopulationCalls.hpp"
#include "../Core/HostedCounselorBirth.hpp"
static unsigned notifications=0;static bool readyValue=false;static void* expected=nullptr;
bool __fastcall TravelStub(void* state,bool notify){if(state!=expected||!notify)RaiseException(0xE0000001,0,0,nullptr);++notifications;return readyValue;}
struct Birth {
 unsigned stage=0,accounts=0,posts=0,rollbacks=0;bool clean=true,post=true,ready=false;
 bool Preflight(){return true;}bool Spawn(){return true;}bool Clean(){return clean;}
 bool NewController(){return true;}bool SetClass(){return true;}bool Restart(){return true;}
 bool Traveled(){stage=11;return cfgmenu::population::detail::Travel(reinterpret_cast<std::uintptr_t>(&TravelStub),reinterpret_cast<std::uintptr_t>(expected),ready);}
 bool Account(){++accounts;return true;}bool Postconditions(){++posts;return post;}
 bool Rollback(){++rollbacks;return false;}
};
int main(){unsigned checks=0,failures=0;const auto check=[&](bool ok){++checks;if(!ok)++failures;};
 int state=0;expected=&state;unsigned reason=0;Birth good;
 check(cfghostedfill::AddOne(good,reason)==cfghostedfill::Result::Completed&&reason==0&&!good.ready&&notifications==1&&good.accounts==1&&good.posts==1&&!good.rollbacks);
 readyValue=true;Birth good2;check(cfghostedfill::AddOne(good2,reason)==cfghostedfill::Result::Completed&&good2.ready&&notifications==2);
 Birth bad;bad.post=false;check(cfghostedfill::AddOne(bad,reason)==cfghostedfill::Result::Quarantined&&reason==8&&bad.rollbacks==1);

 std::printf("H27 native notification + birth postproof: %u checks, %u failures\n",checks,failures);return failures?1:0;
}
