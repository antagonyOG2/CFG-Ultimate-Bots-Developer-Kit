#include "../Core/ChallengeBoundaryWallCensusPolicy.hpp"
#include "../Core/ChallengeEscapeStartPolicy.hpp"
#include "../Core/J24FeatureFlags.hpp"
#include <cassert>
#include <cstdio>
#include <string>
int main(){
 unsigned native=0,hooks=0,failures=0,cancels=0;
 assert(!cfgj29escape::InstallAfterNative(false,[&]{++hooks;return true;}));assert(!hooks);
 assert(cfgj29escape::InstallAfterNative(true,[&]{++hooks;return false;}));assert(hooks==1);
 assert(cfgj29escape::AllowStart(false,failures,[&]{++cancels;}));assert(failures==1&&cancels==1);
 assert(cfgj29escape::AllowStart(true,failures,[&]{++cancels;}));assert(failures==1&&cancels==1);
 (void)native;
 cfgj29walls::Census walls;
 assert(!walls.Observe(u"HouseActor")&&!walls.total);
 assert(!walls.Observe(u"BlockingVolume_C",false)&&!walls.total);
 assert(walls.Observe(u"BlockingVolume_C")&&walls.Observe(u"BlockingVolume_C"));
 assert(walls.Observe(u"Wall_C")&&walls.Observe(u"KillBounds_C"));
 assert(walls.total==4&&walls.distinct==3&&walls.entries[0].count==2);
 std::u16string longName=u"BlockingVolume_";longName+=std::u16string(100,u'x');
 assert(walls.Observe(longName)&&walls.entries[3].name[63]=='\0');
 for(unsigned i=0;i<30;++i){std::u16string s=u"BarrierClass";s+=std::u16string(i+1,u'z');walls.Observe(s);}
 assert(walls.distinct==12&&walls.total==35);
 const auto before=walls;
 assert(!walls.Observe(u"OtherBlockingVolume",false));assert(walls.total==before.total);
 // Flag OFF: production code does not feed wall labels, so report remains
 // zero even for a matching name; default-on escape install is independent.
 cfgj29walls::Census disabled;
 if(CFG_J29_CHALLENGE_BOUNDARY_FULL)disabled.Observe(u"BlockingVolume_C");
 assert(disabled.total==(CFG_J29_CHALLENGE_BOUNDARY_FULL?1u:0u));
 assert(CFG_J29_CHALLENGE_ESCAPE_HOOKS==0||CFG_J29_CHALLENGE_ESCAPE_HOOKS==1);
 assert(CFG_J29_CHALLENGE_ESCAPE_PHYSICAL==0); // this handoff has no GC root scope
 std::puts("Task005 portable policies PASS");
}
