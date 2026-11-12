#pragma once
#include "../RuntimeProbe/ActiveMenuObservation.hpp"
#include <cstdint>
namespace cfgmenu::frontend {
enum class HudState {Unbound,Observing,Armed,Finished,Refused};
struct HudDescriptor {
    active::Context context{};std::uintptr_t event=0;
    bool Valid()const noexcept{
        return cfgprobe::ObjectPointer(context.hud)&&cfgprobe::ObjectPointer(context.widget)&&
            cfgprobe::ObjectPointer(event)&&context.thread&&context.identities[3].HasAssignedSerial();
    }
};
struct HudReport {
    HudState state=HudState::Unbound;unsigned samples=0,phases=0;
    std::uint64_t first=0,last=0;std::uint32_t thread=0;
};
// An auxiliary widget/HUD return cannot establish the three real recurring
// HUD samples. Do not let it consume an OUTER shared pacing slot while those
// samples are still required: the real HUD event often follows in the same
// frame. Otherwise a stream of no-op auxiliary events can starve recurrence.
inline bool MayClaimSetupPacing(HudState state,bool auxiliary) noexcept {
    return !auxiliary||state!=HudState::Observing;
}
// Supplies pacing, not hook installation or event/reflection authority. The
// descriptor provider must bind the exact resolved class-owned HUD event and
// keep its metadata pinned/validated. The existing actor trampoline forwards
// original exactly once, then calls After with the nesting guard still active.
// Gameplay preparation remains a separate immutable identity transaction.
class HudGate {
    HudDescriptor descriptor_{};HudReport report_{};bool servicing_=false;
public:
    bool Bind(const HudDescriptor& value,bool exactDescriptorValidated){
        if(servicing_||report_.state==HudState::Observing||report_.state==HudState::Armed||
            !exactDescriptorValidated||!value.Valid())return false;
        descriptor_=value;report_={};report_.state=HudState::Observing;report_.thread=value.context.thread;return true;
    }
    template<class Host> bool After(Host& host,std::uintptr_t object,std::uintptr_t function,
        std::uint32_t thread,bool nested,bool originalReturnedNormally,std::uint64_t now){
        // Pointer/thread/state-only rejection: no metadata walk for unrelated
        // actor events or once setup is Ready. Never infer the owner thread.
        if(servicing_||nested||(report_.state!=HudState::Observing&&report_.state!=HudState::Armed)||
            object!=descriptor_.context.hud||function!=descriptor_.event||thread!=report_.thread)return false;
        if(!originalReturnedNormally){report_.state=HudState::Refused;return false;}
        if(report_.samples&&(now<report_.last||now-report_.last<25))return false;
        // Limited foreground setup work, never a gameplay maintenance lane.
        if(report_.samples&&(now-report_.first>30000||report_.phases==256)){
            report_.state=HudState::Refused;return false;
        }
        struct Guard {bool& flag;explicit Guard(bool& f):flag(f){flag=true;}~Guard(){flag=false;}} guard(servicing_);
        if(!host.FrontendMatches(descriptor_)){report_.state=HudState::Refused;return false;}
        if(!report_.samples)report_.first=now;
        report_.last=now;++report_.samples;
        // Three time-separated native callbacks establish recurrence before
        // any native row operation. The first three are observation-only.
        if(report_.state==HudState::Observing){if(report_.samples==3)report_.state=HudState::Armed;return true;}
        ++report_.phases;const bool worked=host.Service(now);
        if(host.SetupFinished())report_.state=HudState::Finished;
        if(host.SetupRefused())report_.state=HudState::Refused;
        return worked;
    }
    void Retire()noexcept{if(!servicing_){descriptor_={};report_.state=HudState::Unbound;}}
    template<class Host>bool CompletedHudOpportunity(Host& host,std::uintptr_t hud,std::uint32_t thread,bool nested,bool returned,std::uint64_t now){
        // Additional ACTUAL native callbacks on this positively owned HUD can
        // advance an already armed setup. They never count toward recurrence.
        if(servicing_||nested||!returned||report_.state!=HudState::Armed||hud!=descriptor_.context.hud||
           thread!=report_.thread||now<report_.last||now-report_.last<25)return false;
        if(now-report_.first>30000||report_.phases==256){report_.state=HudState::Refused;return false;}
        struct Guard {bool& flag;explicit Guard(bool& f):flag(f){flag=true;}~Guard(){flag=false;}}guard(servicing_);
        if(!host.FrontendMatches(descriptor_)){report_.state=HudState::Refused;return false;}
        report_.last=now;++report_.phases;const bool worked=host.Service(now);
        if(host.SetupFinished())report_.state=HudState::Finished;
        if(host.SetupRefused())report_.state=HudState::Refused;return worked;
    }
    template<class Host> bool Opportunity(Host& host,std::uintptr_t widget,std::uint32_t thread,bool nested,bool returned,std::uint64_t now){
        // A native widget return can accelerate an ALREADY armed view. It
        // cannot invent the three real HUD recurrence samples or a HUD event.
        if(servicing_||nested||!returned||report_.state!=HudState::Armed||
           widget!=descriptor_.context.widget||thread!=report_.thread||now<report_.last||now-report_.last<25)return false;
        if(now-report_.first>30000||report_.phases==256){report_.state=HudState::Refused;return false;}
        struct Guard {bool& flag;explicit Guard(bool& f):flag(f){flag=true;}~Guard(){flag=false;}}guard(servicing_);
        if(!host.FrontendMatches(descriptor_)){report_.state=HudState::Refused;return false;}
        report_.last=now;++report_.samples;++report_.phases;const bool worked=host.Service(now);
        if(host.SetupFinished())report_.state=HudState::Finished;
        if(host.SetupRefused())report_.state=HudState::Refused;return worked;
    }
    const HudReport& Report()const noexcept{return report_;}
};
}
