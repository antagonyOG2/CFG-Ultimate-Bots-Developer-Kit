#define CFG_RETAINED_GAMEPLAY_BUNDLE
#define CFG_J22_CHALLENGES_RETENTION 1
#define CFG_CHALLENGE_ROUTE_FIXTURE_ONLY
#include "NativeChallengeMenuRouteTests.cpp"
#undef CFG_CHALLENGE_ROUTE_FIXTURE_ONLY
#include "NativeChallengeMenuTransition.hpp"
struct CallbackHooks {
 bool current=true;
 cfgmenu::rows::NativeRetentionHooks Hooks(){return {this,
  [](void* p,std::uintptr_t){return static_cast<CallbackHooks*>(p)->current;},
  [](void*,std::uintptr_t,std::uintptr_t,std::uintptr_t,void*){return true;},
  [](void*,std::uintptr_t,std::uintptr_t,void*){return true;},
  [](void*,std::uintptr_t,std::uintptr_t,std::uintptr_t){return true;},
  [](void*,std::uintptr_t,std::uintptr_t){return true;},[](void*){},[](void*){},[](void*){return true;}};}
};
int main(){using namespace cfgmenu;using F=RouteFixture;using Role=challengeroute::Role;unsigned checks=0,failed=0;
 const auto check=[&](bool value){++checks;if(!value){++failed;std::printf("FAIL J22 queue%u\n",checks);}};
 const std::array<std::uintptr_t,5> thunks{{reinterpret_cast<std::uintptr_t>(&Thunk0),reinterpret_cast<std::uintptr_t>(&Thunk1),reinterpret_cast<std::uintptr_t>(&Thunk2),reinterpret_cast<std::uintptr_t>(&Thunk3),reinterpret_cast<std::uintptr_t>(&Thunk4)}};
 HMODULE resident=nullptr;check(GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_PIN,reinterpret_cast<LPCWSTR>(&Thunk0),&resident)!=0);
 for(unsigned scenario=0;scenario<5;++scenario){auto f=std::make_unique<F>();OptionsReader r{f->fixture.r};RouteEngine engine{*f};challengeroute::Route<OptionsReader,RouteEngine> route(r,engine);active::Context source;
  for(unsigned i=0;i<f->slotCount;++i)f->fixture.r.Set(NativeFixture::Registry+24*i+8,std::uint32_t{});
  check(active::Observe(r,MenuFixture::Base,source)&&route.Prepare(source,F::PageType,thunks));
  check(route.PublishUsing(resident,[&](std::uintptr_t at,std::uintptr_t old,std::uintptr_t value){std::uintptr_t actual=0;if(!r.Read(at,actual)||actual!=old)return sandboxstart::WriteResult::Refused;f->fixture.r.Set(at,value);return sandboxstart::WriteResult::Changed;}));
  f->SetFrame(F::LobbyStart);check(route.QueueBorrowed(Role::LobbyStart,reinterpret_cast<void*>(MenuFixture::Widget),reinterpret_cast<void*>(F::Frame),1000));
  check(route.QueueBorrowed(Role::LobbyStart,reinterpret_cast<void*>(MenuFixture::Widget),reinterpret_cast<void*>(F::Frame),1001)&&route.pendingCoalesced==1&&!engine.pushes);
  unsigned refreshes=0,commits=0,cancels=0;
  const auto refresh=[&](const active::Context&){++refreshes;return true;};
  const auto commit=[&](const active::Context&){++commits;return true;};
  const auto cancel=[&](){++cancels;};
  check(!route.ServicePending(1002,commit,refresh,cancel)&&!engine.pushes&&!refreshes); // no callback scope
  CallbackHooks hooks;rows::NativeRetentionScope callback(source.module,hooks.Hooks(),true);check(callback.Entered());
  if(scenario==1){check(!route.ServicePending(3001,commit,refresh,cancel)&&route.pendingExpired==1&&!engine.pushes);continue;}
  if(scenario==2){f->Register(MenuFixture::Widget,999);check(!route.ServicePending(1100,commit,refresh,cancel)&&route.pendingDropped==1&&!engine.pushes);continue;}
  check(route.ServicePending(1100,commit,refresh,cancel)&&engine.pushes==1&&refreshes==1&&!engine.starts);
  active::Context page;check(challengetransition::Observe(r,source,1,F::PageType,F::Page,page));
  active::Context bad;check(!challengetransition::Observe(r,source,1,F::PageType,F::Page+8,bad));
  check(!route.ServicePending(1200,commit,refresh,cancel)&&engine.pushes==1); // no replay
  route.choicesReady=true;const auto role=scenario==3?Role::PageBack:Role::PageStart;
  f->SetFrame(scenario==3?F::PageBack:F::PageStart);
  check(route.QueueBorrowed(role,reinterpret_cast<void*>(F::Page),reinterpret_cast<void*>(F::Frame),1300));
  if(scenario==4){f->fixture.r.Set(F::Frame+startdisk::FrameNode,std::uintptr_t{});check(!route.QueueBorrowed(role,reinterpret_cast<void*>(F::Page),reinterpret_cast<void*>(F::Frame),1301));}
  check(route.ServicePending(1400,commit,refresh,cancel)&&engine.pops==1);
  check(challengetransition::Observe(r,page,2,0,0,bad));
  check(scenario==3?(cancels==1&&!commits&&!engine.starts):(commits==1&&engine.starts==1&&!cancels));
  check(!route.ServicePending(1500,commit,refresh,cancel)&&engine.pops==1);
 }
 std::printf("J22 actual Challenge route/frame/deferred callback/transition: %u checks,%u failures; expiry,serial,coalescing,no-scope,no-replay,commit-before-one-Start. Engine calls mocked, no game access.\n",checks,failed);return failed?1:0;}
