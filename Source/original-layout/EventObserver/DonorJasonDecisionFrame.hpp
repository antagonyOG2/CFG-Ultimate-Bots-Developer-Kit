#pragma once
#include "NativeJasonControllerTickObservation.hpp"
#include "DonorCounselorObservation.hpp"
#include "../Core/DonorJasonTargeting.hpp"
#include "../Core/J22TargetPolicy.hpp"
namespace cfgdonorframe {
using Slot=cfgprobe::lifetime::SlotObservation;
struct Source {
    cfgjasontick::Owner owner{};
    std::array<Slot,64> targets{};
    unsigned count=0;
    std::uint64_t revision=0;
    bool Same(const Source& b)const {
        return revision==b.revision&&count==b.count&&targets==b.targets&&
            owner.source.module==b.owner.source.module&&owner.source.threadId==b.owner.source.threadId&&
            owner.source.objects==b.owner.source.objects&&owner.actor.pawn==b.owner.actor.pawn&&
            owner.actor.killerController==b.owner.actor.killerController&&owner.actor.path==b.owner.actor.path&&
            owner.pawnClass==b.owner.pawnClass&&owner.defaults==b.owner.defaults&&
            owner.killerBase==b.owner.killerBase&&owner.killerAI==b.owner.killerAI&&owner.pathClass==b.owner.pathClass;
    }
};
enum class State {Empty,Collecting,Ready,Refused};
struct Target {
    cfgdonorhunt::CounselorObservation value{};
    cfgdonorhunt::Position position{};
    bool observed=false;
};
// Weak, readonly decision inputs, NOT a GC pin or permission to call native.
// Large rosters can exceed a single1536read callback. Preserve the cursor over
// an unentered read-budget yield rather than repeatedly selecting a prefix.
// The selected actor MUST be freshly proved again at its effect boundary.
class Frame {
    Source source_{};std::array<Target,64> targets_{};cfgdonorhunt::Position origin_{};
    unsigned cursor_=0;State state_=State::Empty;
public:
    State Status()const {return state_;}unsigned Cursor()const {return cursor_;}
    const cfgdonorhunt::Position& Origin()const{return origin_;}
    const Source& Inputs()const {return source_;}
    bool ObservedDistance(std::uintptr_t pawn,float& distance)const {
        if(state_!=State::Ready)return false;
        for(unsigned i=0;i<source_.count;++i)if(source_.targets[i].object==pawn&&targets_[i].observed){
            const auto& p=targets_[i].position;
            const float x=p.X-origin_.X,y=p.Y-origin_.Y,z=p.Z-origin_.Z;
            distance=std::sqrt(x*x+y*y+z*z);return std::isfinite(distance);
        }
        return false;
    }
    bool ObservedLive(std::uintptr_t pawn)const {
        if(state_!=State::Ready)return false;
        for(unsigned i=0;i<source_.count;++i)if(source_.targets[i].object==pawn)
            return targets_[i].observed&&targets_[i].value.Live();
        return false;
    }
    void Invalidate(){state_=State::Empty;cursor_=0;targets_={};}
    void Consumed(){if(state_==State::Ready)Invalidate();}
    template<class Host>State Step(Host& host,const Source& source){
        if(source.count>64||!source.owner.actor.pawn.HasAssignedSerial()||
           !source.owner.actor.killerController.HasAssignedSerial())return state_=State::Refused;
        if(state_==State::Empty||state_==State::Refused||!source_.Same(source)){
            source_=source;targets_={};cursor_=0;state_=State::Collecting;
        }
        if(state_==State::Refused)return state_;
        if(!host.Current(source_.owner)){
            if(host.BudgetExhausted())return state_=State::Collecting;
            targets_={};return state_=State::Refused;
        }
        if(state_==State::Ready)return state_;
        while(cursor_<source_.count){
            if(!host.TargetBudgetAvailable())return state_;
            Target value;
            const bool observed=host.Observe(source_.targets[cursor_],value.value,value.position);
            if(!observed&&host.BudgetExhausted())return state_; // SAME slot next callback
            value.observed=observed;targets_[cursor_]=value;++cursor_;
        }
        // Use a fresh Jason origin at dispatch, not the first scan callback.
        if(!host.Current(source_.owner)||!host.Origin(source_.owner.actor.pawn,origin_)){
            if(host.BudgetExhausted())return state_=State::Collecting;
            targets_={};return state_=State::Refused;
        }
        return state_=State::Ready;
    }
    // H2.86 report-only: why Nearest found nobody, and per-candidate skip causes (0 notReady 1 count 2 order 3 none;
    // skips 0 blocked 1 notObservedLive 2 noController 3 noPosition 4 excluded).
    struct NearestStats {std::array<std::uint64_t,4> empty{};std::array<std::uint64_t,5> skips{};std::uint64_t chosen=0;};
    static NearestStats& Stats(){static NearestStats s;return s;}
    std::uintptr_t Nearest(cfgdonorhunt::Targets& roster,std::uint64_t now,std::uintptr_t excluded=0,bool retryFatigue=false){
        auto& stats=Stats();
        if(state_!=State::Ready){++stats.empty[0];return 0;}
        if(roster.count!=static_cast<std::int32_t>(source_.count)){++stats.empty[1];return 0;}
        // Never apply a completed value frame to reordered/changed membership.
        for(unsigned i=0;i<source_.count;++i)if(roster.actors[i]!=source_.targets[i].object){++stats.empty[2];return 0;}
        for(unsigned i=0;i<source_.count;++i){const auto p=source_.targets[i].object;const auto& t=targets_[i];
            if(!p)continue;if(p==excluded){++stats.skips[4];continue;}
            if(roster.DonorIsJasonAITargetBlocked(p,now)){++stats.skips[0];continue;}
            if(!t.observed||!t.value.Live()){++stats.skips[1];continue;}
            if(!t.value.possessed||!t.value.controller.HasAssignedSerial()){++stats.skips[2];continue;}}
        struct Host {
            Frame& frame;cfgdonorhunt::Targets& roster;std::uint64_t now;bool retryFatigue;
            std::uint64_t Now(){return now;}
            bool IsJasonAITargetBlocked(std::uintptr_t p,std::uint64_t t){
                const auto i=roster.DonorFindJasonAITargetIndex(p);
                // Retry only the donor's long fatigue block when every normal
                // choice failed. Never bypass the short post-grab quarantine.
                if(retryFatigue&&i>=0&&roster.blockStrikes[static_cast<unsigned>(i)]&&
                    roster.blockedUntil[static_cast<unsigned>(i)]>t&&roster.blockedUntil[static_cast<unsigned>(i)]-t>2500)return false;
                return roster.DonorIsJasonAITargetBlocked(p,t);}
            const Target* Find(std::uintptr_t p){
                for(unsigned i=0;i<frame.source_.count;++i)if(frame.source_.targets[i].object==p)return &frame.targets_[i];
                return nullptr;
            }
            bool ReadableActor(std::uintptr_t p){if(p==frame.source_.owner.actor.pawn.object)return true;
                const auto t=Find(p);return t&&t->observed&&t->value.Live();}
            bool ReadablePawnController(std::uintptr_t p){const auto t=Find(p);
                return t&&t->observed&&t->value.possessed&&t->value.controller.HasAssignedSerial();}
            bool GetJasonAIActorLocation(std::uintptr_t p,cfgdonorhunt::Position& position){
                if(p==frame.source_.owner.actor.pawn.object){position=frame.origin_;return true;}
                const auto t=Find(p);if(!t||!t->observed)return false;position=t->position;return true;
            }
        } host{*this,roster,now,retryFatigue};
        const auto chosen=roster.DonorFindNearestJasonAICounselorTarget(host,source_.owner.actor.pawn.object,excluded);
        if(chosen)++stats.chosen;else ++stats.empty[3];
        return chosen;
    }
};
}
