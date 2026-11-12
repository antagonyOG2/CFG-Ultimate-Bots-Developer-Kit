#pragma once
#include "../Core/MenuReentryPolicy.hpp"
namespace cfgmenu::challengeservice {
// Completed owner-thread callbacks provide scheduling, not widget authority.
// In particular, the previous frontend HUD/event may be gone after a mission.
// The published route must prove its CURRENT positive page before each phase.
template<class Route> bool OwnsView(const Route& route,const active::Context& view)noexcept{
 return route.Published()&&route.state==3&&route.Page()==view.widget&&route.PageType()==view.type&&
  route.PageIdentity()==view.identities[2]&&reentry::Infrastructure(route.Source(),view);
}
class Service {
 std::uint64_t next_=0;
public:
 unsigned services=0,refusals=0;bool current=false;
 template<class Route,class Work>bool After(Route& route,std::uint32_t thread,bool nested,bool completed,std::uint64_t now,Work work){
  if(!thread||thread!=route.Source().thread||nested||!completed||!route.Published()||route.Busy()||
     (route.state!=2&&route.state!=3)||now<next_||now>UINT64_MAX-25)return false;
  next_=now+25;active::Context page;current=false;
  if(!route.PageCurrent(page)){route.choicesReady=false;++refusals;return false;}
  current=true;++services;work(page,now);return true;
 }
};
}
