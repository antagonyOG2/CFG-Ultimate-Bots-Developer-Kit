#pragma once
#include <cstdint>
namespace cfgmenu::setupslice {
// Only coalesce existing verified phases when the SAME callback's bounded
// reader still has room. Never reset its counter or enlarge its cap.
constexpr unsigned Reserve=16384,MaximumSteps=4;
template<class Reader>auto Remaining(const Reader& r,int)->decltype(r.cap,r.reads,unsigned{}){
    return r.reads<r.cap?r.cap-r.reads:0;
}
template<class Reader>auto Remaining(const Reader& r,long)->decltype(r.limit,r.reads,unsigned{}){
    return r.reads<r.limit?r.limit-r.reads:0;
}
inline unsigned Remaining(...){return 0;} // Unknown budget: original single phase.
template<class Reader>bool MayContinue(const Reader& r){return Remaining(r,0)>=Reserve;}
}
