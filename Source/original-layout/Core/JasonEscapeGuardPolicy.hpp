#pragma once
#include "JasonHuntPolicy.hpp"
#include <array>
#include <cmath>
#include <cstdint>
// Escape guard (user direction): once an exit is live (police arrived, or a
// car/boat exit in use), a counselor approaching it is Jason's top priority,
// like a car pursuit. Pure decision over positions the runtime already
// observes; the runtime supplies which exits are CURRENTLY active (proved
// from native state) and owns every Morph/move/ownership gate. No reads here.
namespace cfgescapeguard {
using cfghunt::V3;
constexpr unsigned MaximumExits=8,MaximumCounselors=12;
constexpr float Threat=3500.f;        // counselor within 35 m of a live exit
constexpr float MorphGap=1000.f;      // Jason would arrive >10 m later than them
constexpr float MorphMinimum=5000.f;  // never Morph for an exit already within 50 m
constexpr float GuardOffset=300.f;    // land 3 m from the exit toward the runner
constexpr std::uint64_t Hold=10000;   // keep guarding while the threat persists

struct Exit {bool active=false;V3 at{};};
struct Counselor {bool valid=false;V3 at{};};
enum class Action {None,Hunt,Morph};
struct Decision {Action action=Action::None;unsigned exit=0,counselor=0;V3 goal{};};

inline float Flat(const V3& a,const V3& b){return std::sqrt(cfghunt::Flat2(a,b));}

class Guard {
    unsigned exit_=~0u,counselor_=~0u;std::uint64_t until_=0;
public:
    // morphReady: the existing Morph cooldown/ownership gates already allow it.
    Decision Decide(const V3& jason,const std::array<Exit,MaximumExits>& exits,
                    const std::array<Counselor,MaximumCounselors>& counselors,bool morphReady,std::uint64_t now){
        Decision d;if(!cfghunt::Finite(jason))return d;
        float best=Threat;bool found=false;
        for(unsigned e=0;e<exits.size();++e){if(!exits[e].active||!cfghunt::Finite(exits[e].at))continue;
            for(unsigned c=0;c<counselors.size();++c){if(!counselors[c].valid||!cfghunt::Finite(counselors[c].at))continue;
                float distance=Flat(counselors[c].at,exits[e].at);
                // Hysteresis: the guarded pair wins ties for the hold window.
                if(e==exit_&&c==counselor_&&now<until_)distance-=500.f;
                if(distance<best){best=distance;found=true;d.exit=e;d.counselor=c;}}}
        if(!found){exit_=counselor_=~0u;until_=0;return d;}
        if(d.exit!=exit_||d.counselor!=counselor_){exit_=d.exit;counselor_=d.counselor;}
        until_=now+Hold;
        const V3& gate=exits[d.exit].at;const V3& runner=counselors[d.counselor].at;
        const float runnerToExit=Flat(runner,gate),jasonToExit=Flat(jason,gate);
        // Goal: a point just in front of the exit on the runner's side, so
        // Jason stands between the counselor and the way out.
        V3 toward{runner.x-gate.x,runner.y-gate.y,0.f};const float len=std::sqrt(toward.x*toward.x+toward.y*toward.y);
        d.goal=len>1.f?V3{gate.x+toward.x/len*std::min(GuardOffset,len),gate.y+toward.y/len*std::min(GuardOffset,len),gate.z}:gate;
        d.action=morphReady&&jasonToExit>MorphMinimum&&jasonToExit>runnerToExit+MorphGap?Action::Morph:Action::Hunt;
        return d;
    }
    bool Guarding(std::uint64_t now)const{return exit_!=~0u&&now<until_;}
    void Clear(){exit_=counselor_=~0u;until_=0;}
};
}
