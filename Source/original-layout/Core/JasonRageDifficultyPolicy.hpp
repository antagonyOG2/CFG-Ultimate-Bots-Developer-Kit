#pragma once
#include <cmath>
#include <cstdint>
namespace cfgragethreshold {
// Port OfflineSetup::ApplyCommittedJasonDifficultyToPawn. It changes the
// native timer threshold, not Rage state or current animation/ability flags.
struct Rage {
    std::uintptr_t pawn=0;std::int32_t serial=0;bool attempted=false,applied=false;
    float original=0,desired=0;
    bool Prepare(std::uintptr_t object,std::int32_t id,unsigned difficulty,float current){
        if(attempted||!object||id<=0||difficulty>2||!std::isfinite(current)||current<=0||current>86400)return false;
        pawn=object;serial=id;original=current;desired=current*(difficulty==0?2.f:difficulty==2?.5f:1.f);
        return std::isfinite(desired)&&desired>0&&desired<=172800;
    }
};
}
