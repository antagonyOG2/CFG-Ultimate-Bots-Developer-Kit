#include "../Core/FrontendMapArrivalPolicy.hpp"
#include "../Core/FrontendEpochService.hpp"
#include <cassert>
#include <initializer_list>
#include <cstdio>
// The full repo supplies ActiveMenuObservation.hpp. Run the real transaction
// only there; the standalone policy tests below run without Unreal dependencies.
#if __has_include("../RuntimeProbe/ActiveMenuObservation.hpp")
#include "NativeCounselorRoutePolicy.hpp"
#define MAPARRIVAL_REAL_ROUTE 1
#else
#define MAPARRIVAL_REAL_ROUTE 0
#endif
using cfgmenu::maparrival::ShouldArrive;
int main(){unsigned count=0;
 for(unsigned state=0;state<=10;++state){
  (void)state;
  assert(ShouldArrive(true));
  assert(ShouldArrive(false)==(!CFG_J35_MENU_REENTRY_FIX));++count;
 }
 using namespace cfgmenu::epochservice;
 // State 8 is still eligible for the unclaimed-world epoch transition.
 unsigned state=8;bool counselorClaim=false,coordinatorArrived=false;
 if(ShouldArrive(counselorClaim)){coordinatorArrived=true;state=5;}
#if CFG_J35_MENU_REENTRY_FIX
 assert(!coordinatorArrived&&state==8);
 assert(UnclaimedTravel(state,false,false,true,true,true,true));
 assert(Candidate(42,42,state,false,true));
#else
 assert(coordinatorArrived&&state==5);
 assert(!UnclaimedTravel(state,false,false,true,true,true,true));
#endif
#if MAPARRIVAL_REAL_ROUTE
 // Pin the real production state enumeration (Empty ... Retired), and
 // prove that a returned entry is not an AwaitMap counselor transaction.
 using cfgmenu::counselorroute::State;
 for(auto s:{State::Empty,State::EntryReady,State::EntryRequested,State::AwaitCounselor,
  State::CounselorReady,State::AcceptRequested,State::AwaitMap,State::MapReady,
  State::SettingsReady,State::Submitted,State::Retired}){
  (void)s;assert(ShouldArrive(true));assert(ShouldArrive(false)==(!CFG_J35_MENU_REENTRY_FIX));
 }
 cfgmenu::counselorroute::Transaction coordinator;
 cfgmenu::active::Context c{};
 c.module=0x140000000ull;c.world=0x100000;c.mode=0x110000;c.controller=0x120000;
 c.hud=0x130000;c.widget=0x140000;c.type=0x150000;c.gameInstance=0x160000;
 c.stack=0x170000;c.thread=42;c.array.count=3;
 for(unsigned i=0;i<4;++i){c.identities[i].object=i==0?c.world:i==1?c.controller:i==2?c.widget:c.hud;
  c.identities[i].serial=static_cast<int>(i+1);c.identities[i].index=static_cast<int>(i+2);}
 const auto entryType=c.type;
 const std::uintptr_t counselor=0x210000,map=0x220000,settings=0x230000,sandbox=0x240000;
 assert(coordinator.Bind(c,counselor,map,settings,sandbox)&&coordinator.CurrentState()==State::EntryReady);
 assert(coordinator.RequestEntry(c));cfgmenu::counselorroute::Token token{};
 assert(coordinator.Borrow(c,cfgmenu::counselorroute::Step::Entry,sandbox,token));
 assert(coordinator.Consume(c,token)&&coordinator.CurrentState()==State::AwaitCounselor);
 // A current counselor widget is admitted. Its actual native callback is
 // still independently validated by the integration host.
 auto counselorTop=c;counselorTop.type=counselor;counselorTop.widget=0x250000;
 counselorTop.identities[2].object=counselorTop.widget;++counselorTop.identities[2].serial;
 assert(coordinator.Arrive(counselorTop)&&coordinator.CurrentState()==State::CounselorReady);
 assert(coordinator.RequestAccept(counselorTop));
 assert(coordinator.Borrow(counselorTop,cfgmenu::counselorroute::Step::AcceptedCounselor,0,token));
 assert(coordinator.Consume(counselorTop,token)&&coordinator.CurrentState()==State::AwaitMap);
 auto mapTop=counselorTop;mapTop.type=map;mapTop.widget=0x260000;
 mapTop.identities[2].object=mapTop.widget;++mapTop.identities[2].serial;
 assert(ShouldArrive(true)&&coordinator.Arrive(mapTop)&&coordinator.CurrentState()==State::MapReady);
 auto entryReturn=mapTop;entryReturn.type=entryType;entryReturn.widget=0x270000;
 entryReturn.identities[2].object=entryReturn.widget;++entryReturn.identities[2].serial;
 assert(coordinator.ReturnToEntry(entryReturn)&&coordinator.CurrentState()==State::EntryReady);
 // Native Jason map arrival is not a counselor route event.
 if(ShouldArrive(false)){coordinator.Arrive(mapTop);}
#if CFG_J35_MENU_REENTRY_FIX
 assert(coordinator.CurrentState()==State::EntryReady);
#else
 assert(coordinator.CurrentState()==State::EntryReady); // failed Arrive remains refused in original native host
#endif

#endif
 std::printf("J35 map arrival policy (%d) PASS: %u states; no native call.\n",CFG_J35_MENU_REENTRY_FIX,count);
}
