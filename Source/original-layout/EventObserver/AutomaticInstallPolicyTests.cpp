#include "../Core/AutomaticInstallPolicy.hpp"
#include <cassert>
#include <cstdio>
int main(){using namespace cfgautoinstall;Policy p(100);
 for(unsigned stage=0;stage<5;++stage){std::uintptr_t module=0;unsigned calls=0;
  const auto result=DiscoverModules([&](bool observer,std::uintptr_t& out){++calls;
   if(!observer){out=stage==1?0:0x140000000ULL;return stage!=0;}
   out=stage==3?0x500000ULL:0;return stage!=2;
  },module);
  assert(result==(stage<2?ModuleDecision::GamePending:stage==2?ModuleDecision::ObserverPending:
   stage==3?ModuleDecision::Present:ModuleDecision::Ready));
  assert(module==(stage==4?0x140000000ULL:0)&&calls==(stage<2?1u:2u));
 }
 {Policy waiting(100);waiting.bound=true;std::uintptr_t module=0;
  for(unsigned i=0;i<20;++i){assert(DiscoverModules([](bool,std::uintptr_t&){return false;},module)==ModuleDecision::GamePending);
   assert(!waiting.attempted&&!module);}
  assert(waiting.Observe(1100,true,true,false,false)==Decision::Closed&&!waiting.attempted);
 }
 assert(p.Observe(100,false,false,false,false)==Decision::Wait);
 assert(p.Due(100)&&!p.Due(100+Interval-1)&&p.Due(100+Interval));
 assert(p.Observe(250,true,false,false,true)==Decision::Wait);
 assert(p.Observe(400,true,false,false,true)==Decision::Wait);
 assert(p.Observe(550,true,false,false,true)==Decision::Install);
 assert(p.Observe(3100,true,false,false,true)==Decision::Wait&&p.attempted);
 assert(p.Observe(4000,true,true,false,true)==Decision::Closed);
 Policy cold(100);assert(cold.Observe(100,true,false,false,true)==Decision::Wait);
 assert(cold.Observe(250,true,false,false,false)==Decision::Wait);
 assert(cold.Observe(400,true,false,false,true)==Decision::Wait);
 assert(cold.Observe(550,true,false,false,true)==Decision::Wait);
 assert(cold.Observe(700,true,false,false,true)==Decision::Install);
 Policy present(0);assert(present.Observe(0,true,false,true,false)==Decision::Present&&!present.attempted);
 Policy rapid(0);assert(rapid.Observe(0,true,false,false,true)==Decision::Wait);
 assert(rapid.Observe(1,true,false,false,true)==Decision::Wait);
 assert(rapid.Observe(2,true,false,false,true)==Decision::Wait&&!rapid.attempted);
 assert(rapid.Observe(2*Interval-1,true,false,false,true)==Decision::Wait&&!rapid.attempted);
 assert(rapid.Observe(2*Interval,true,false,false,true)==Decision::Install);
 assert(Interval==50&&PollSleep==10);
 Policy timeout(100);assert(timeout.Observe(100+Timeout,false,false,false,false)==Decision::Expired&&!timeout.attempted);
 std::puts("Automatic install policy PASS: stable real entry, single attempt, present/exit/deadline stops, no restart");
}
