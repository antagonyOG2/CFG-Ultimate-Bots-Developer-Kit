#pragma once
#include <cstdint>
#include "J24FeatureFlags.hpp"
#include "ChallengeStrongChildrenPolicy.hpp"
namespace cfgmenu::challengeintro {
// A single-owner, single-use transfer receipt. A request is NOT native-call
// authority. The receiving retained gameplay host must independently prove
// current Challenges mode, intro phase, live owner/level identities, no online
// NetDriver, function metadata/code, GC safety and retained native inputs.
struct DispatchRequest {
 std::uintptr_t module=0,world=0,mode=0,settings=0,level=0,function=0;
 std::int32_t worldSerial=0,settingsSerial=0,levelSerial=0;
 std::uint32_t thread=0;std::uint64_t expires=0;
 // A serial-0 child is NEVER a lease root. These slot indices are pinned to
 // the exact post-time owner observation and reproved by the retained host.
 int levelIndex=-1,settingsIndex=-1;bool strongChildren=false;
 bool Valid()const{return module&&world&&mode&&settings&&level&&function&&
     worldSerial>0&&thread&&expires&&(
      (settingsSerial>0&&levelSerial>0&&!strongChildren)||
      (CFG_J33_INTRO_STRONG_CHILDREN&&strongChildren&&settingsSerial>=0&&levelSerial>=0&&
       (!settingsSerial||!levelSerial)&&levelIndex>=0&&settingsIndex>=0));}
};
enum class DispatchStage:unsigned {Empty,Posted,Claimed,Returned,Refused,Expired,Cancelled};
enum class DispatchReason:unsigned {None,Invalid,Busy,Expired,StaleOrUnproven,NativeRefused,Cancelled};
struct DispatchReceipt {
 DispatchRequest request{};DispatchStage stage=DispatchStage::Empty;
 DispatchReason reason=DispatchReason::None;
 unsigned posted=0,claimed=0,returned=0,refused=0;
 // Never overwrite a pending request; never replay a claimed native action.
 bool Post(DispatchRequest next,std::uint64_t now){
  if(stage==DispatchStage::Posted){reason=DispatchReason::Busy;return false;}
  if(!next.Valid()||now>=next.expires){reason=DispatchReason::Invalid;return false;}
  request=next;stage=DispatchStage::Posted;reason=DispatchReason::None;++posted;return true;
 }
 bool Pending()const{return stage==DispatchStage::Posted;}
 void Cancel(){if(Pending()){stage=DispatchStage::Cancelled;reason=DispatchReason::Cancelled;++refused;}request={};}
 // Each host must be a retained-gameplay dispatcher, NOT NativeMenuDispatch.
 // No native function or memory offset is supplied by this policy.
 template<class Host>bool Service(Host& host,std::uint64_t now){
  if(!Pending())return false;
  if(now>=request.expires){stage=DispatchStage::Expired;reason=DispatchReason::Expired;++refused;return false;}
  // An unproven callback consumes the request rather than allowing a replay
  // into a subsequent intro, a different world, or a post-match outro.
  if(!host.CurrentIntro(request)){stage=DispatchStage::Refused;reason=DispatchReason::StaleOrUnproven;++refused;return false;}
  stage=DispatchStage::Claimed;++claimed; // consume BEFORE native invocation
  if(host.InvokeRetainedIntro(request)){stage=DispatchStage::Returned;reason=DispatchReason::None;++returned;return true;}
  stage=DispatchStage::Refused;reason=DispatchReason::NativeRefused;++refused;return false;
 }
};
inline bool WallsWaitForIntro(bool armed,const DispatchReceipt& receipt){
 // Posting consumes the input producer and clears Armed(), but the retained
 // consumer still owns priority until the request reaches a terminal stage.
 return receipt.stage==DispatchStage::Posted||receipt.stage==DispatchStage::Claimed||(armed&&receipt.stage==DispatchStage::Empty);
}
inline bool DeferUnadmittedIntro(const DispatchReceipt& receipt,std::uint64_t now,bool sourceCurrent,bool admitted){
 // No native entry has occurred: preserve this same bounded request for a
 // verified callback. Expiry and positively stale ownership still refuse it.
 return receipt.Pending()&&sourceCurrent&&!admitted&&now<receipt.request.expires;
}
}
