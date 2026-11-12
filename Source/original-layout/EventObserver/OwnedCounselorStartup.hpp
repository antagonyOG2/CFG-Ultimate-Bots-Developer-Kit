#pragma once
#include "StartingPlayerObservation.hpp"
#include "../RuntimeProbe/FreshGameplayObservation.hpp"
#include "../Core/SetupMenuGameplayHandoff.hpp"
namespace cfgmenu::startup {
enum class State {Empty,Preparing,Installed,Restarted,Refused,Quarantined};
enum class Result {Ignored,Installed,Refused,Quarantined};
// Host registers this persistent route BEFORE passing its compiled gates to
// Before. Installation additionally requires independently established owner
// quiescence; this class cannot infer it from a thread ID or a paused flag.
// No reset, post-birth possession repair, native-path search or retry.
template<class Reader,class Publication,class Calls>class OwnedCounselorStartup {
    Reader& reader_;Publication& publication_;Calls& calls_;GameplayHandoff& handoff_;
    freshgame::Source source_;std::uint64_t frontend_=0,generation_=0;
    promotion::gate::OwnedSandboxPrebirthGate<Reader,Publication> gate_;
    promotion::NativeCounselorPromotion<Reader,Calls> promotion_;
    promotion::Context owner_{};State state_=State::Empty;
    promotion::Result birthResult_=promotion::Result::Refused;
public:
    OwnedCounselorStartup(Reader& reader,Publication& publication,Calls& calls,GameplayHandoff& handoff,
        const freshgame::Source& source,std::uint64_t frontend,std::uint64_t generation):
        reader_(reader),publication_(publication),calls_(calls),handoff_(handoff),source_(source),frontend_(frontend),generation_(generation),
        gate_(reader,publication),promotion_(reader,calls){}
    OwnedCounselorStartup(const OwnedCounselorStartup&)=delete;
    State CurrentState()const noexcept{return state_;}
    promotion::Result BirthResult()const noexcept{return birthResult_;}
    const promotion::Context& Owner()const noexcept{return owner_;}
    template<class ResolverFactory>Result Before(const starting::Event& event,const promotion::gate::Gates& gates,
        bool ownerQuiescent,ResolverFactory&& resolverFactory){
        if(state_!=State::Empty||event.module!=source_.module||event.thread!=source_.thread||
           !handoff_.ArmedFor(frontend_,generation_,source_.frontendWorld))return Result::Ignored;
        // Cheap exact event test first; no reflection work on unrelated actors.
        std::uintptr_t expected=0;
        if(!reader_.Read(event.module+startingdisk::FunctionCache,expected)||expected!=event.function)return Result::Ignored;
        state_=State::Preparing;
        starting::Observation seen;
        if(!ownerQuiescent||!starting::Observe(reader_,event,seen)||seen.owner.world==source_.frontendWorld||
           seen.owner.engine!=source_.engine||seen.owner.viewport!=source_.viewport||
           seen.owner.player!=source_.player||seen.owner.gameInstance!=source_.gameInstance){state_=State::Refused;return Result::Refused;}
        // Resolver constructor is not a call to load/resolve. The supplied
        // resolver may inspect only this already-loaded exact picked value.
        auto resolver=resolverFactory(seen.owner);
        if(!starting::ResolvePicked(reader_,seen.owner,event.thread,resolver,owner_)){state_=State::Refused;return Result::Refused;}
        MenuCatalogueEvidence evidence;Selection selected;std::uint64_t submission=0;
        if(!handoff_.InspectArmed(evidence,selected,submission)||evidence.frontend!=frontend_||
           (!selected.preserveNativeCounselor&&selected.counselorClass!=owner_.selectedClass)){
            state_=State::Refused;return Result::Refused;}
        const auto installed=gate_.Install(owner_,gates,ownerQuiescent);
        if(installed==promotion::gate::InstallResult::Quarantined){state_=State::Quarantined;return Result::Quarantined;}
        if(installed!=promotion::gate::InstallResult::Installed||!gate_.Bind(owner_)){state_=State::Refused;return Result::Refused;}
        state_=State::Installed;return Result::Installed;
    }
    // compiled host gates invoke only these methods on the actual owner thread.
    // handled=true means consume even a refusal; never spawn a stock Jason as
    // a fallback for this counselor route. Unrelated owners forward natively.
    bool Restart(std::uintptr_t mode,std::uintptr_t controller,std::uint32_t currentThread){
        if(mode!=owner_.mode||controller!=owner_.controller||currentThread!=source_.thread)return false;
        if(state_!=State::Installed)return true;
        state_=State::Restarted;birthResult_=promotion_.Restart(owner_);return true;
    }
    promotion::GetterResult PawnClass(std::uintptr_t mode,std::uintptr_t controller,std::uint32_t currentThread){
        if(mode!=owner_.mode||controller!=owner_.controller||currentThread!=source_.thread)return {};
        if(state_!=State::Restarted)return {true,0};
        const auto result=promotion_.PawnClass(mode,controller);
        return result.handled?result:promotion::GetterResult{true,0};
    }
};
}
