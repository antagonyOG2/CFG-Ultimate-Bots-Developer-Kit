#include "../Core/SweaterLifecyclePolicy.hpp"
#include <cassert>
#include <cstdio>
int main(){using namespace cfgsweater;
 Policy p;p.ConfirmPublish();assert(p.ObserveUsed(1000,1));
 assert(p.lastUse==1000&&p.rearmAt==12000);
 assert(p.RebindNative(2000,0)&&p.stage==Stage::Waiting&&p.rebinds==1);
 assert(p.Decide(11999,false,true,true)==Action::None);
 assert(p.RebindNative(8000,1)&&p.rebinds==2&&p.rearmAt==12000);
 assert(p.Decide(12000,false,true,true)==Action::Rearm);
 p.ConfirmPublish();assert(p.ObserveUsed(13000,1));
 assert(p.RebindNative(13500,0)&&p.rearmAt==24000&&!p.publicationStarted);
 assert(p.RebindNative(14000,0)&&p.rebinds==4);
 assert(!p.RebindNative(14500,2)&&p.rebinds==4);
 p.Refuse();assert(!p.RebindNative(15000,0));
 Policy q;q.ConfirmPublish();assert(q.QueueRetiredOwned(1000));
 assert(q.RebindNative(1100,0)&&!q.repairingOwned&&q.rebinds==1&&q.stage==Stage::Waiting);
 assert(q.Decide(1100,false,true,true)==Action::Rearm);
 Policy r;r.ConfirmPublish();r.BeginPublish();assert(!r.RebindNative(1000,0)&&!r.rebinds);
 std::puts("Sweater J25 policy PASS: two replacements, repair retirement, no early recharge, refusal/uncertainty gate");
}
