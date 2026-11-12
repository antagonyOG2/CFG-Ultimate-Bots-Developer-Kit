#pragma once
#include <cstdint>
namespace cfgmenu::challengeintro {
// Donor: a held menu Start does not skip the first intro frame. A release and
// fresh foreground press is required. Never carry an edge across phase/world.
struct InputPolicy {
 std::uintptr_t world=0;bool previous=true,consumed=false,pending=false;
 std::uint64_t deadline=0;
 unsigned edges=0,attempts=0,returns=0;
 void Observe(std::uintptr_t current,bool intro,bool foreground,bool down,std::uint64_t now){
  if(current!=world){world=current;previous=true;consumed=pending=false;deadline=0;}
  if(!intro||!foreground){previous=true;pending=false;return;}
  if(down&&!previous&&!consumed){pending=true;deadline=now+1000;++edges;}
  previous=down;
  if(pending&&now>=deadline)pending=false;
 }
 bool Due(std::uint64_t now)const{return pending&&!consumed&&now<deadline;}
 void Consume(){pending=false;consumed=true;++attempts;}
};
}
