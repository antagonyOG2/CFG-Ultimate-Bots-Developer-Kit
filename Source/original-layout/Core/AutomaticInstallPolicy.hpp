#pragma once
#include <cstdint>
namespace cfgautoinstall {
// Same three separated current-owner observations and full installer checks;
// faster read-only polling reduces scheduling latency, not verification work.
constexpr std::uint64_t Timeout=15*60*1000,Interval=50,PollSleep=10;
enum class Decision {Wait,Install,Present,Closed,Expired};
enum class ModuleDecision {GamePending,ObserverPending,Ready,Present};
// Read-only startup discovery may be transient while the SAME process loads.
// Publish no main-module address until both enumerations are complete. This
// never retries installation, adopts another PID, or changes access rights.
template<class Probe> ModuleDecision DiscoverModules(Probe probe,std::uintptr_t& module){
 std::uintptr_t game=0,existing=0;
 if(!probe(false,game)||!game)return ModuleDecision::GamePending;
 if(!probe(true,existing))return ModuleDecision::ObserverPending;
 if(existing)return ModuleDecision::Present;
 module=game;return ModuleDecision::Ready;
}
struct Policy {
 std::uint64_t started=0,next=0,readySince=0;unsigned stable=0;bool attempted=false,bound=false;
 explicit Policy(std::uint64_t now):started(now),next(now){}
 bool Due(std::uint64_t now){if(now<next)return false;next=now+Interval;return true;}
 Decision Observe(std::uint64_t now,bool process,bool closed,bool existing,bool ready){
  if(now<started||now-started>=Timeout)return Decision::Expired;
  if(closed&&bound)return Decision::Closed;if(existing)return Decision::Present;
  if(!process||!ready){stable=0;readySince=0;return Decision::Wait;}bound=true;
  if(!stable)readySince=now;++stable;
  if(attempted||stable<3||now<readySince||now-readySince<2*Interval)return Decision::Wait;
  attempted=true;return Decision::Install;
 }
};
}
