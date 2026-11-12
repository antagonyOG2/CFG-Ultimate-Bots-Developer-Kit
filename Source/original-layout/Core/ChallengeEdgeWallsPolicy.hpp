#pragma once
#include <array>
#include <cmath>
#include <cstdint>
#include <algorithm>
#include <string_view>
#include "J215FeatureFlags.hpp"
namespace cfgj36walls {
// 176 named decisions keep the complete matching frontend report below its
// existing 32 KiB read bound. Overflow refuses the census before any writes.
constexpr unsigned Capacity=176;
struct Vec {float x=0,y=0,z=0;};
struct Bounds {Vec center{},extent{};};
inline bool Finite(const Bounds& b){
 for(const float v:{b.center.x,b.center.y,b.center.z,b.extent.x,b.extent.y,b.extent.z})if(!std::isfinite(v)||std::fabs(v)>10000000.f)return false;
 return b.extent.x>0&&b.extent.y>0&&b.extent.z>0;
}
inline bool SameWorldBounds(const Bounds& a,const Bounds& b){
 if(!Finite(a)||!Finite(b))return false;
 // 0.05 cm allows native float recomputation, not actor motion or local/world substitution.
 return std::fabs(a.center.x-b.center.x)<=.05f&&std::fabs(a.center.y-b.center.y)<=.05f&&std::fabs(a.center.z-b.center.z)<=.05f&&
  std::fabs(a.extent.x-b.extent.x)<=.05f&&std::fabs(a.extent.y-b.extent.y)<=.05f&&std::fabs(a.extent.z-b.extent.z)<=.05f;
}
enum class Decision:std::uint32_t {Pending,Malformed,NotThinWall,IncompletePerimeter,Interior,Edge,OriginallyOff,Changed,Uncertain};
struct Record {
 std::uintptr_t actor=0;std::int32_t index=-1,serial=0;
 Bounds bounds{};std::array<char,24> name{};std::uint64_t nameHash=0;
 Decision decision=Decision::Pending;std::uint32_t state=0; // 1=owned off,2=restored,3=unknown,4=departed untouched
#if CFG_J215_CHALLENGE_WALL_OWNERSHIP
 std::uint32_t owner=UINT32_MAX,geometry=0;Vec rotation{},localExtent{};
#endif
};
#if CFG_J215_CHALLENGE_WALL_OWNERSHIP
constexpr unsigned OwnerCapacity=32;
struct LevelOwner {
 std::uintptr_t level=0,outerWorld=0,package=0;
 std::uint32_t persistent=0,challenge=0; // 1=exact mission package,2=mission sublevel,3=other package
 std::array<char,32> levelName{};std::array<char,160> packageName{};
};
inline bool ChallengePackage(std::u16string_view map,std::u16string_view package){
 if(map.find(u"/Game/Maps/Single_Player/")!=0)return false;
 return package==map||(package.size()>map.size()+1&&package.substr(0,map.size())==map&&
     (package[map.size()]==u'_'||package[map.size()]==u'/'));
}
inline bool BlockingLabel(std::string_view name){
 constexpr std::string_view prefix="BlockingVolume";
 if(name.substr(0,prefix.size())!=prefix)return false;name.remove_prefix(prefix.size());
 if(!name.empty()&&name.front()=='_')name.remove_prefix(1);if(name.empty())return false;
 for(const auto c:name)if(c<'0'||c>'9')return false;return true;
}
#endif
struct Diagnostics {
 std::uint32_t enabled=0,stage=0,failure=0,count=0,chosen=0,applied=0,restored=0,uncertain=0,departed=0;
 std::array<Record,Capacity> records{};
#if CFG_J215_CHALLENGE_WALL_OWNERSHIP
 std::uint32_t ownerCount=0,unknownOwners=0,diagnosticOnly=0;
 std::array<LevelOwner,OwnerCapacity> owners{};
#endif
#if CFG_J218_FINISHING
 struct Mismatch {std::uint32_t index=0;Bounds stored{},fresh{};};
 std::uint32_t sampled=0,sampleFailures=0,mismatchCount=0,mismatchTruncated=0;
 std::array<Mismatch,16> mismatches{};
 // H2.76: which frontend gate refused during the apply stage (counts per attempt) and the last refusal detail.
 std::uint32_t gateScope=0,gateBounds=0,gateApply=0,gateExhausted=0,gateApplyStep=0,gateWhy=0,gateEngine=0,gateInput=0,gateOutput=0;
 std::array<std::uint32_t,7> gateAdmit{};std::uint32_t gateSampleScope=0,gateSampleInput=0,gateSampleOutput=0,memberClause=0,memberRefreshed=0,memberRefreshFailed=0; // H2.79: first failing Member clause (0=none), list refreshes, failed proofs; H2.78: provider admission clause counts, stage-2 scope failures
#endif
};
// Conservative map-edge selection requires a complete enclosing four-side
// perimeter. Long/thin/tall volumes at its outer envelope qualify; interior
// walls, malformed geometry and incomplete/unknown boundaries keep collision.
inline void Classify(Diagnostics& d){
#if CFG_J215_CHALLENGE_WALL_OWNERSHIP
 d.chosen=d.diagnosticOnly=0;
 for(unsigned i=0;i<d.count;++i){auto& r=d.records[i];r.decision=Decision::Pending;
  if(!Finite(r.bounds)){r.decision=Decision::Malformed;continue;}
  if(r.owner>=d.ownerCount||!d.owners[r.owner].challenge||d.owners[r.owner].challenge>2){
   // The observed 1500-z/serial-zero cluster is evidence for a diagnostic,
   // never a replacement for mission package/level ownership.
   if(!r.serial&&std::fabs(r.bounds.extent.z-1500.f)<1.f)++d.diagnosticOnly;
   r.decision=Decision::Interior;continue;
  }
  // Ownership selects the challenge set. These guards merely exclude small,
  // low or specially named blockers; rotated world AABBs need not be thin.
  if(!BlockingLabel(std::string_view(r.name.data()))||r.bounds.extent.z<1200.f||
     (std::max)(r.bounds.extent.x,r.bounds.extent.y)<1000.f||!(r.geometry&1u)){
   r.decision=Decision::NotThinWall;continue;
  }
  r.decision=Decision::Edge;++d.chosen;
 }
#else
 float left=INFINITY,right=-INFINITY,bottom=INFINITY,top=-INFINITY;
 for(unsigned i=0;i<d.count;++i){auto& r=d.records[i];r.decision=Decision::Pending;
  if(!Finite(r.bounds)){r.decision=Decision::Malformed;continue;}
  const auto& b=r.bounds;const auto small=(std::min)(b.extent.x,b.extent.y),large=(std::max)(b.extent.x,b.extent.y);
  if(small>200||large<1000||large<small*10||b.extent.z<200){r.decision=Decision::NotThinWall;continue;}
  left=(std::min)(left,b.center.x-b.extent.x);right=(std::max)(right,b.center.x+b.extent.x);
  bottom=(std::min)(bottom,b.center.y-b.extent.y);top=(std::max)(top,b.center.y+b.extent.y);
 }
 const auto width=right-left,height=top-bottom;
 unsigned sides=0;
 const auto side=[&](const Record& r){unsigned result=0;const auto& b=r.bounds;
  if(b.extent.x<b.extent.y&&b.extent.y*2>=height*0.75f){
   if(std::fabs(b.center.x-b.extent.x-left)<=width*0.02f)result|=1;
   if(std::fabs(b.center.x+b.extent.x-right)<=width*0.02f)result|=2;
  }
  if(b.extent.y<b.extent.x&&b.extent.x*2>=width*0.75f){
   if(std::fabs(b.center.y-b.extent.y-bottom)<=height*0.02f)result|=4;
   if(std::fabs(b.center.y+b.extent.y-top)<=height*0.02f)result|=8;
  }return result;
 };
 const bool envelope=std::isfinite(width)&&std::isfinite(height)&&width>=2000&&height>=2000;
 if(envelope)for(unsigned i=0;i<d.count;++i)if(d.records[i].decision==Decision::Pending)sides|=side(d.records[i]);
 d.chosen=0;
 for(unsigned i=0;i<d.count;++i){auto& r=d.records[i];if(r.decision!=Decision::Pending)continue;
  if(!envelope||sides!=15){r.decision=Decision::IncompletePerimeter;continue;}
  r.decision=side(r)?Decision::Edge:Decision::Interior;if(r.decision==Decision::Edge)++d.chosen;
 }
#endif
}
enum class Write {NotEntered,Returned,Uncertain};
// State transitions for ONE collision setter. Host must independently reprove
// callback, Challenges scope, current actor membership/geometry and exact flag.
// Mark uncertain before allowing any later step; entered ambiguity never retries.
template<class Host>bool Apply(Record& r,Diagnostics& d,Host& host){
 if(r.decision!=Decision::Edge||r.state)return false;
#if CFG_J218_FINISHING
 bool collision=false;if(!host.Current(r)){d.gateApplyStep=1;return false;}
 if(!host.ReadCollision(r,collision)){d.gateApplyStep=2;return false;}
#else
 bool collision=false;if(!host.Current(r)||!host.ReadCollision(r,collision))return false;
#endif
 if(!collision){r.decision=Decision::OriginallyOff;return false;}
 const auto result=host.Set(r,false);
#if CFG_J218_FINISHING
 if(result==Write::NotEntered){d.gateApplyStep=3;return false;}
#else
 if(result==Write::NotEntered)return false;
#endif
 if(result==Write::Uncertain){r.state=3;r.decision=Decision::Uncertain;++d.uncertain;return false;}
 if(!host.Current(r)||!host.ReadCollision(r,collision)||collision){r.state=3;r.decision=Decision::Uncertain;++d.uncertain;return false;}
 r.state=1;++d.applied;return true;
}
template<class Host>bool Restore(Record& r,Diagnostics& d,Host& host){
 if(r.state!=1)return false;
 // Check World before any old actor dereference. Departure never authorizes a
 // write or a claimed restore; the diagnostic preserves that distinction.
 if(!host.WorldCurrent()){r.state=4;++d.departed;return false;}
 bool collision=false;if(!host.Current(r)||!host.ReadCollision(r,collision))return false;
 if(collision){r.state=3;++d.uncertain;return false;} // external change: no overwrite
 const auto result=host.Set(r,true);if(result==Write::NotEntered)return false;
 if(result==Write::Uncertain||!host.Current(r)||!host.ReadCollision(r,collision)||!collision){r.state=3;++d.uncertain;return false;}
 r.state=2;++d.restored;return true;
}
}
