#pragma once
#include "NativeSetupRows.hpp"
#include "../Core/J22FeatureFlags.hpp"
namespace cfgmenu::challengemenu {
// Donor ChallengeSetupPolicy.hpp order: eight Off/On options.
namespace choice {
constexpr unsigned Knives=0,Recharge=1,Movement=2,Stealth=3,Untimed=4,SkipReplay=5,
 PreventEscapes=6-CFG_J217_FINISHING,Boundary=7-CFG_J217_FINISHING,EdgeWalls=8-CFG_J217_FINISHING,Count=8+CFG_J36_CHALLENGE_WALLS-CFG_J217_FINISHING;
}
inline std::array<rows::Spec,choice::Count> Specs(){
 std::array<rows::Spec,choice::Count> specs{};
 constexpr std::array<RowId,choice::Count> ids{{RowId::Knives,RowId::Recharge,RowId::Movement,RowId::Stealth,RowId::Untimed,
#if !CFG_J217_FINISHING
 RowId::SkipReplay,
#endif
 RowId::PreventEscapes,RowId::Boundary
#if CFG_J36_CHALLENGE_WALLS
 ,RowId::EdgeWalls
#endif
 }};
 constexpr std::array<std::u16string_view,choice::Count> titles{{u"Unlimited Throwing Knives",u"Faster Ability Recharge",u"Faster Jason Movement (2x)",
    u"Stealth Practice (Low Detection)",u"No Match Timeout",
#if !CFG_J217_FINISHING
    u"Skip Intro on Replay",
#endif
    u"Prevent Counselors Escaping",u"Disable Jason Boundary"
#if CFG_J36_CHALLENGE_WALLS
    ,u"Disable Outer Blocking Walls"
#endif
 }};
 for(unsigned i=0;i<specs.size();++i){auto& s=specs[i];s.id=ids[i];s.title=titles[i];s.count=2;s.labels[0]=u"Off";s.labels[1]=u"On";s.preserveNativeValue=true;}
 if(!CFG_J217_FINISHING&&CFG_J22_CHALLENGES_RETENTION&&!CFG_J22_CHALLENGE_REPLAY){auto& replay=specs[choice::SkipReplay];
  replay.title=u"Skip Intro on Replay (Unavailable)";replay.labels[0]=replay.labels[1]=u"Unavailable";}
#if CFG_J36_CHALLENGE_WALLS
 specs[choice::EdgeWalls].preserveNativeValue=CFG_J216_FINISHING!=0; // selected=0; retain the authored live label binding
#endif
 specs[0].reuseChallengeDonor=true;return specs;
}
template<std::size_t N>bool ReplayChoice(const std::array<unsigned,N>& choices){
 return !CFG_J217_FINISHING&&choices[choice::SkipReplay]!=0;
}
}
