#include "../Core/J24FeatureFlags.hpp"
#include "../Core/ChallengeNativeScopePolicy.hpp"
#include <cassert>
#include <cstdio>
using namespace cfgmenu::challengeintro;
struct ModelHost {
 unsigned current=0,calls=0;bool valid=true,native=true;
 bool CurrentIntro(const DispatchRequest& r){++current;return valid&&r.world==2&&r.mode==3&&r.function==7;}
 bool InvokeRetainedIntro(const DispatchRequest&){++calls;return native;}
};
int main(){
 ScopeEvidence e{};assert(!e.Admitted()&&e.Refusal()==ScopeRefusal::Disabled);
 e.flag=e.posted=e.completed=e.gameThread=e.offline=e.challengeMode=e.challengeState=e.routeOwner=
 e.worldCurrent=e.levelSettingsCurrent=e.gcClear=e.callbackProof=e.retainedInputs=e.exclusiveScope=true;
 assert(e.Admitted());
 auto refusal=[&](bool ScopeEvidence::*field,ScopeRefusal expected){e.*field=false;assert(!e.Admitted()&&e.Refusal()==expected);e.*field=true;};
 refusal(&ScopeEvidence::flag,ScopeRefusal::Disabled);
 refusal(&ScopeEvidence::posted,ScopeRefusal::NoRequest);
 refusal(&ScopeEvidence::completed,ScopeRefusal::WrongCallback);
 e.nested=true;assert(e.Refusal()==ScopeRefusal::WrongCallback);e.nested=false;
 refusal(&ScopeEvidence::gameThread,ScopeRefusal::WrongThread);
 refusal(&ScopeEvidence::offline,ScopeRefusal::Online);
 refusal(&ScopeEvidence::challengeMode,ScopeRefusal::WrongMode);
 refusal(&ScopeEvidence::challengeState,ScopeRefusal::WrongMode);
 refusal(&ScopeEvidence::worldCurrent,ScopeRefusal::WrongWorld);
 refusal(&ScopeEvidence::routeOwner,ScopeRefusal::OwnerStale);
 refusal(&ScopeEvidence::levelSettingsCurrent,ScopeRefusal::OwnerStale);
 refusal(&ScopeEvidence::gcClear,ScopeRefusal::GCBlocked);
 refusal(&ScopeEvidence::callbackProof,ScopeRefusal::MissingCallbackProof);
 refusal(&ScopeEvidence::retainedInputs,ScopeRefusal::MissingRetention);
 refusal(&ScopeEvidence::exclusiveScope,ScopeRefusal::ScopeOccupied);
 const DispatchRequest request{1,2,3,4,5,7,10,11,12,42,1100};
 {ScopeDiagnostics d;d.Observe(e);assert(d.entered==1&&d.refused==0);
  DispatchReceipt receipt;ModelHost host;assert(receipt.Post(request,100));assert(Consume(receipt,host,101,d));
  assert(host.current==1&&host.calls==1&&d.serviced==1&&d.dropped==0);
  assert(!Consume(receipt,host,102,d)&&host.calls==1);
 }
 {ScopeDiagnostics d;DispatchReceipt receipt;ModelHost host;host.valid=false;
  assert(receipt.Post(request,100));assert(!Consume(receipt,host,101,d));
  assert(d.dropped==1&&d.serviced==0&&host.calls==0);host.valid=true;
  assert(!Consume(receipt,host,102,d)&&host.calls==0);
 }
 {ScopeDiagnostics d;DispatchReceipt receipt;ModelHost host;
  assert(receipt.Post(request,100));assert(!Consume(receipt,host,1100,d));
  assert(d.dropped==1&&host.calls==0&&receipt.reason==DispatchReason::Expired);
 }
 {ScopeDiagnostics d;DispatchReceipt receipt;ModelHost host;host.native=false;
  assert(receipt.Post(request,100));assert(!Consume(receipt,host,101,d));
  assert(host.calls==1&&d.dropped==1&&receipt.claimed==1&&receipt.reason==DispatchReason::NativeRefused);
  host.native=true;assert(!Consume(receipt,host,102,d)&&host.calls==1);
 }
 std::printf("J32 Challenge native scope policy + one-shot consumer PASS (flag %d); simulated provider only.\n",CFG_J32_CHALLENGE_NATIVE_SCOPE);
}
