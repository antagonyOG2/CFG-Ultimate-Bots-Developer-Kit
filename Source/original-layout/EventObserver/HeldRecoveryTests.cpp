#include "../Core/HeldRecoveryPolicy.hpp"
#include <cstdio>
int main(){unsigned checks=0,failed=0;const auto check=[&](bool value){++checks;if(!value)++failed;};
 using namespace cfgholdrecovery;
 {Policy p;check(p.Observe(1,2,0,0,1000));check(!p.Stale(3499,false,false,false,false,false));
  check(p.Stale(3500,false,false,false,false,false));check(!p.Stale(5000,false,false,false,true,false));
  check(!p.Stale(5000,false,false,false,false,true));check(!p.Stale(5000,false,true,false,false,false));
  check(p.Stale(5000,false,true,true,false,false));p.Input(1,5000);
  check(!p.Stale(9999,false,false,false,false,false));check(p.Stale(10000,false,true,false,false,false));
  p.Attempt(10,10000);check(!p.Stale(20000,false,false,false,false,false));p.Refuse();
  check(!p.Stale(30000,false,false,false,false,false));p.Observe(1,3,0,0,40000);
  check(p.phase==Phase::Observe&&p.since==40000&&!p.inputAt);p.Released(41000);
  check(!p.Allowed(1,3,41999)&&p.Allowed(1,4,41999)&&p.Allowed(1,3,42000));}
 {Policy p;p.Observe(1,2,3,4,1000);check(!p.Stale(6499,true,true,false,false,false));
  check(p.Stale(6500,true,true,false,false,false));p.Observe(1,2,3,5,7000);
  check(!p.Stale(7000,true,false,false,false,false));check(!p.Observe(1,0,3,5,7000));}
 std::printf("Donor hold/extraction timing and ambiguity: %u checks, %u failures; no native calls.\n",checks,failed);return failed?1:0;
}
