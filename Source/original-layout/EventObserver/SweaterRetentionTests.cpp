#include "../Core/SweaterRetention.hpp"
#include <cassert>
#include <cstdio>
struct Host {
 bool fresh=true,cleared=true,budget=true;unsigned writes=0,fail=0;
 std::uintptr_t owner=0,ability=0;unsigned used=1,has=0,wearing=0,hud=0;
 bool Fresh(const cfgsweater::Retention&){return fresh;}
 bool Cleared(const cfgsweater::Retention&){return cleared;}
 bool Reserve(){return budget;}
 bool Owner(const cfgsweater::Retention& t){if(++writes==fail)return false;owner=t.pawn;return true;}
 bool Ability(const cfgsweater::Retention& t){if(++writes==fail)return false;ability=t.ability;return true;}
 bool Readback(const cfgsweater::Retention& t){return owner==t.pawn&&ability==t.ability;}
};
int main(){
 const cfgsweater::Retention t{1,2,true,true};Host h;
 assert(cfgsweater::RetainConsumed(h,t)&&h.writes==2&&h.owner==1&&h.ability==2);
 assert(h.used==1&&!h.has&&!h.wearing&&!h.hud); // native consumed state stays consumed
 for(unsigned gate=0;gate<3;++gate){Host rejected;if(!gate)rejected.fresh=false;else if(gate==1)rejected.cleared=false;else rejected.budget=false;
  assert(!cfgsweater::RetainConsumed(rejected,t)&&!rejected.writes);}
 for(unsigned fail=1;fail<=2;++fail){Host partial;partial.fail=fail;assert(!cfgsweater::RetainConsumed(partial,t)&&partial.writes==fail);}
 auto unearned=t;unearned.earned=false;Host denied;assert(!cfgsweater::RetainConsumed(denied,unearned)&&!denied.writes);
 unearned=t;unearned.used=false;assert(!cfgsweater::RetainConsumed(denied,unearned)&&!denied.writes);
 std::puts("CFG earned sweater native-cleanup retention: PASS (no early grant/use reset)");
}
