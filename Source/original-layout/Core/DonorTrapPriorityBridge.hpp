#pragma once
#include <cstdint>
namespace cfgj214trap {
// FJB ProcessQueuedTrapPriority: event intent only, never native-call authority.
// The driver supplies fresh retained ownership and the unchanged donor nav ring.
enum class MorphResult {NotEntered,Pending,ReturnedFalse,ReturnedTrue,Uncertain};
struct Intent {
 std::uintptr_t victim=0;std::uint64_t until=0,baselineMorph=0,nextAttempt=0;
 bool completed=false,inFlight=false,ambiguous=false;
 unsigned attempts=0,confirmed=0,sharedSatisfied=0;
 void Post(std::uintptr_t value,std::uint64_t now,std::uint64_t baseline){
  if(!value||now>UINT64_MAX-60000)return;
  victim=value;until=now+60000;baselineMorph=baseline;nextAttempt=now;
  completed=inFlight=ambiguous=false;
 }
 void Clear(){victim=0;until=baselineMorph=nextAttempt=0;completed=inFlight=ambiguous=false;}
};
template<class Original,class Queue,class V,class T,class P>
int Forward(V victim,T trap,P output,Original original,bool ownerThread,Queue&& queue){
 const int result=original?original(victim,trap,output):-1;
 if(result>=0&&ownerThread)queue(victim,trap);
 return result;
}
// True owns this driver slice. Waiting on cooldown/busy does not starve normal
// hunting; an unentered native operation yields without spending its retry.
template<class Host>bool Service(Intent& s,Host& host,std::uint64_t now){
 if(!s.victim)return false;
 if(!host.Primary()||now>=s.until){s.Clear();return false;}
 if(!host.Usable(s.victim)){
  if(host.Deferred())return true; // budget denial is not evidence the victim died
  s.Clear();return false;
 }
 const auto morph=host.LastMorph();
 if(!s.completed&&morph&&morph!=s.baselineMorph){
  s.baselineMorph=morph;s.completed=true;s.inFlight=false;++s.sharedSatisfied;
 }
 if(s.completed)return false;
 if(s.inFlight){
  if(host.NavigationPending())return true;
  s.inFlight=false;s.nextAttempt=now+3000;return false;
 }
 if(now<s.nextAttempt)return false;
 if(host.Busy(now))return host.Deferred(); // donor hierarchy: combat before cooldown
 if(host.MorphRemaining(now))return false;
 if(!host.Stop()){
  if(host.Uncertain()){s.completed=s.ambiguous=true;return true;}
  return host.Deferred();
 }
 ++s.attempts;
 switch(host.Morph(s.victim)){
 case MorphResult::ReturnedTrue:
  host.MarkMorph(now);s.baselineMorph=host.LastMorph();s.completed=true;
  s.nextAttempt=now+3000;++s.confirmed;return true;
 case MorphResult::Pending:s.inFlight=true;return true;
 case MorphResult::Uncertain:
  // An entered ambiguous effect is consumed, never retried after settling.
  s.completed=s.ambiguous=true;return true;
 case MorphResult::ReturnedFalse:s.nextAttempt=now+3000;return false;
 case MorphResult::NotEntered:--s.attempts;return host.Deferred();
 }
 return false;
}
}
