#define CFG_CHALLENGE_ROUTE_FIXTURE_ONLY
#include "NativeChallengeMenuRouteTests.cpp"
#undef CFG_CHALLENGE_ROUTE_FIXTURE_ONLY
#include "NativeChallengePageResolver.hpp"
int main(){using namespace cfgmenu;unsigned checks=0,failed=0;const auto check=[&](bool b){++checks;if(!b){++failed;std::printf("FAIL Inspect%u\n",checks);}};
 for(unsigned fault=0;fault<10;++fault){RouteFixture f;OptionsReader r{f.fixture.r};active::Context source;
  constexpr auto bpKind=std::uintptr_t{0x1900000};f.fixture.Name(bpKind,"WidgetBlueprintGeneratedClass");f.Root(bpKind);
  f.fixture.r.Set(RouteFixture::PageType+cfgprobe::ObjectClass,bpKind);f.Register(RouteFixture::PageType,700);
  f.fixture.r.Set(MenuFixture::Base+challengecalls::PushMenu.factory,challengecalls::PushMenuFactory);
  check(active::Observe(r,MenuFixture::Base,source));
  std::uintptr_t hudType=0;check(r.Read(MenuFixture::Base+execdisk::GetCurrentMenu.reflection,hudType)&&hudType!=RouteFixture::MenuType);
  if(fault==2)f.fixture.r.Set(RouteFixture::PageType+cfgprobe::lifetime::ObjectIndexOffset,-1);
  if(fault==3)f.fixture.Name(RouteFixture::PageType,"WrongWidget_C");
  if(fault==4)f.fixture.Name(bpKind,"Class");
  if(fault==5)f.fixture.r.Set(MenuFixture::Base+pushdisk::MenuConstructor,hudType);
  if(fault==6)f.fixture.Name(RouteFixture::MenuType,"SCHUD");
  if(fault==7){auto bad=challengecalls::PushMenuFactory;bad[0]^=1;f.fixture.r.Set(MenuFixture::Base+challengecalls::PushMenu.factory,bad);}
  if(fault==8)f.fixture.r.Set(RouteFixture::PageType+cfgprobe::ClassSuper,hudType);
  if(fault==9)f.fixture.r.Set(NativeFixture::Viewport+cfgprobe::ViewportWorld,std::uintptr_t{0x1960000});
  unsigned stage=0;cfgprobe::lifetime::SlotObservation slot;
  auto current=[&](){if(fault==1)return false;active::Context actual;
   if(!active::Observe(r,source.module,actual)||!(actual==source))return false;source.registry=actual.registry;return true;};
  const bool result=challengepage::InspectClass(r,source,RouteFixture::PageType,stage,slot,current);
  check(result==(fault==0));check(stage==(fault==0?9u:fault==9?1u:fault));
  if(fault==0){check(slot.object==RouteFixture::PageType&&slot.serial==700);
   // Retained source survives the class load on a later callback only if its
   // actual positive current-context tuple is still identical. No callback ID
   // or stale registry header is treated as authority.
   constexpr std::uintptr_t allocated=0x1970000;const auto oldCount=source.registry.count;
   f.Register(allocated,701); // a synchronous load may extend the object array
   check(challengepage::InspectClass(r,source,RouteFixture::PageType,stage,slot,current));
   check(source.registry.count>oldCount);}
 }
 std::printf("J23 actual Challenge returned-class proof: %u checks,%u failures; separate SCHUD/SCMenuWidget, real UClass super/slot/name layout, pinned factory, wrong/stale owners refused. Owned memory.\n",checks,failed);return failed?1:0;}
