// Standalone portable gate fixture. Run with CFG_J29_CHALLENGE_INTRO_SCOPE=0
// and =1; NativeChallengeIntroTests.cpp exercises the real Step on Windows.
#define CFG_RETAINED_GAMEPLAY_BUNDLE
#include "../Core/J24FeatureFlags.hpp"
#include "../RuntimeProbe/PrimaryLookupEligibility.hpp"
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <map>
struct ScopeReader {
 std::map<std::uintptr_t,std::uint64_t> cells;unsigned reads=0;
 template<class T>bool Read(std::uintptr_t at,T& result){++reads;auto it=cells.find(at);if(it==cells.end())return false;result=static_cast<T>(it->second);return true;}
};
struct ScopeOwner {
 std::uintptr_t module=0,world=0,callback=0;bool offline=true,current=true;unsigned checks=0;
 static bool Prove(ScopeReader& r,void* p){auto& owner=*static_cast<ScopeOwner*>(p);++owner.checks;
  std::uint64_t observed=0;return owner.current&&owner.offline&&owner.module&&
   r.Read(owner.world+0x10,observed)&&observed==owner.callback;}
};
struct ScopeIntro {
 unsigned calls=0,attempts=0,edges=0;bool consumed=false;
 bool Skip(ScopeReader& reader,ScopeOwner& owner){
  ++edges;if(consumed)return false;
  // Production Step first positively observes owner+world, then independently
  // rechecks all its links before entering. This fixture isolates the shared
  // reader-typed capability and the inherited strict LookupSafe gate.
  if(!ScopeOwner::Prove(reader,&owner))return false;
  cfgprimary::ScopedLookupEligibility<ScopeReader> scope(owner.module,&owner,&ScopeOwner::Prove,true);
  if(!scope.Entered()||!cfgprimary::LookupSafe(reader,owner.module))return false;
  consumed=true;++attempts;++calls;return true;
 }
};
int main(){constexpr std::uintptr_t module=0x100000,world=0x200000,callback=0x300000;
 const auto fixture=[&](){ScopeReader r;r.cells[module+cfgprimary::SerializationFlag]=0;
  r.cells[module+cfgprimary::GarbageCollectionCounter]=0;r.cells[world+0x10]=callback;return r;};
 ScopeOwner owner{module,world,callback};
 {auto r=fixture();assert(!cfgprimary::LookupSafe(r,module));ScopeIntro intro;
  if(CFG_J29_CHALLENGE_INTRO_SCOPE){assert(intro.Skip(r,owner));assert(!intro.Skip(r,owner));assert(intro.calls==1);}
  else assert(!cfgprimary::LookupSafe(r,module));
  assert(!cfgprimary::LookupSafe(r,module));}
 {auto r=fixture();ScopeIntro intro;
  cfgprimary::ScopedLookupEligibility<ScopeReader> occupied(module,&owner,&ScopeOwner::Prove,true);
  assert(occupied.Entered());assert(!intro.Skip(r,owner)&&intro.calls==0);}
 for(unsigned veto=0;veto<2;++veto){auto r=fixture();ScopeIntro intro;
  r.cells[module+(veto?cfgprimary::SerializationFlag:cfgprimary::GarbageCollectionCounter)]=1;
  assert(!intro.Skip(r,owner)&&intro.calls==0);
  // Veto may become active *after* a valid scope has been entered.
  r.cells[module+(veto?cfgprimary::SerializationFlag:cfgprimary::GarbageCollectionCounter)]=0;
  cfgprimary::ScopedLookupEligibility<ScopeReader> scope(module,&owner,&ScopeOwner::Prove,true);
  assert(scope.Entered()&&cfgprimary::LookupSafe(r,module));
  r.cells[module+(veto?cfgprimary::SerializationFlag:cfgprimary::GarbageCollectionCounter)]=1;
  assert(!cfgprimary::LookupSafe(r,module));}
 {auto r=fixture();ScopeIntro intro;owner.offline=false;
  assert(!intro.Skip(r,owner)&&intro.calls==0);owner.offline=true;
  owner.current=false;assert(!intro.Skip(r,owner));owner.current=true;}
 {auto r=fixture();ScopeIntro intro;r.cells[world+0x10]=callback+1;
  assert(!intro.Skip(r,owner)&&intro.calls==0);}
 {auto r=fixture();ScopeIntro intro;
  if(CFG_J29_CHALLENGE_INTRO_SCOPE){assert(intro.Skip(r,owner));assert(!intro.Skip(r,owner));
   ScopeIntro nextEdge;assert(nextEdge.Skip(r,owner)&&nextEdge.attempts==1);}}
 std::printf("Challenge lookup scope fixture PASS (flag %d): gated scope, GC veto, outsider, single use, repeated edge. No game access.\n",CFG_J29_CHALLENGE_INTRO_SCOPE);
}
