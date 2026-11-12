#include "../Core/J24FeatureFlags.hpp"
#include "../Core/ChallengeIntroDispatchPolicy.hpp"
#include <cassert>
#include <cstdio>
using namespace cfgmenu::challengeintro;
struct Host {
 unsigned proofs=0,calls=0;bool intro=true,valid=true,retained=true;
 bool CurrentIntro(const DispatchRequest& r){++proofs;return intro&&valid&&r.world==1&&r.mode==2&&r.settings==3&&r.level==4&&r.function==5;}
 bool InvokeRetainedIntro(const DispatchRequest&){++calls;return retained;}
};
int main(){
 const DispatchRequest request{9,1,2,3,4,5,1,2,3,7,1100};
 {DispatchReceipt q;Host h;assert(q.Post(request,100));assert(!q.Post(request,100));assert(q.posted==1);
  assert(q.Service(h,101)&&h.proofs==1&&h.calls==1);assert(!q.Service(h,102)&&h.calls==1);
  assert(q.stage==DispatchStage::Returned&&q.claimed==1&&q.returned==1);}
 {DispatchReceipt q;Host h;h.intro=false;assert(q.Post(request,100));assert(!q.Service(h,101)&&h.calls==0);
  assert(q.stage==DispatchStage::Refused&&q.reason==DispatchReason::StaleOrUnproven);h.intro=true;assert(!q.Service(h,102));}
 {DispatchReceipt q;Host h;assert(q.Post(request,100));assert(!q.Service(h,1100)&&h.calls==0);
  assert(q.reason==DispatchReason::Expired&&q.refused==1);}
 {DispatchReceipt q;Host h;assert(q.Post(request,100));q.Cancel();assert(!q.Service(h,101)&&h.calls==0);
  assert(q.reason==DispatchReason::Cancelled&&q.refused==1);}
 {DispatchReceipt q;Host h;h.retained=false;assert(q.Post(request,100));assert(!q.Service(h,101));
  assert(h.calls==1&&q.claimed==1&&q.reason==DispatchReason::NativeRefused);
  h.retained=true;assert(!q.Service(h,102)&&h.calls==1);}
 {DispatchReceipt q;Host h;assert(q.Post(request,100));assert(q.Service(h,101));
  const auto next=request;assert(q.Post(next,200));assert(q.Service(h,201)&&h.calls==2);
  assert(q.posted==2&&q.claimed==2&&q.returned==2);}
 {DispatchReceipt q;auto invalid=request;invalid.settingsSerial=0;assert(!q.Post(invalid,100));
  assert(q.reason==DispatchReason::Invalid&&q.posted==0);}
 {DispatchReceipt q;auto child=request;child.levelSerial=0;child.settingsSerial=0;
  child.levelIndex=4;child.settingsIndex=7;child.strongChildren=true;
  assert(q.Post(child,100)==static_cast<bool>(CFG_J33_INTRO_STRONG_CHILDREN));
  if(CFG_J33_INTRO_STRONG_CHILDREN){Host h;assert(q.Service(h,101)&&h.calls==1&&!q.Service(h,102));}}
 {DispatchReceipt q;auto child=request;child.levelSerial=0;child.strongChildren=true;
  assert(!q.Post(child,100)); // missing stable slot indices
  child.levelIndex=4;child.settingsIndex=7;
  assert(q.Post(child,100)==static_cast<bool>(CFG_J33_INTRO_STRONG_CHILDREN));}

 std::printf("J31 request handoff PASS (flag %d): post/claim/no-replay/stale/expiry/refusal/second edge; dispatch host simulated only.\\n",CFG_J31_INTRO_SKIP_GAMEPLAY_DISPATCH);
}
