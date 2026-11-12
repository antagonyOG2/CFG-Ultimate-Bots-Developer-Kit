#pragma once
#include "../RuntimeProbe/ActiveMenuObservation.hpp"
namespace cfgmenu::entryreturn {
enum class State {Empty,Ready,AwaitEntry,Relabeling,Retired};
struct Ticket {std::uint64_t generation=0;active::Context entry{};};
// This policy does not discover a widget, publish callbacks, or write labels.
// Begin is called only after the coordinator's positively owned Back guard and
// cancellation. Host::ReturnCurrent must re-observe the actual native top and
// prove the unchanged class/source graphs, descriptors and owned executors.
// In particular it must NOT use the old widget's equality as return authority.
class Policy {
    State state_=State::Empty;active::Context source_{};std::uintptr_t counselor_=0,departing_=0;
    std::uint64_t generation_=0;Ticket pending_{};
    static bool Positive(const active::Context& c)noexcept{
        const std::array<std::uintptr_t,4> objects={c.world,c.controller,c.widget,c.hud};
        if(!c.thread||!c.array.Sane())return false;
        for(unsigned i=0;i<objects.size();++i)if(!cfgprobe::ObjectPointer(objects[i])||
            c.identities[i].object!=objects[i]||!c.identities[i].HasAssignedSerial())return false;
        return true;
    }
    bool Infrastructure(const active::Context& c)const noexcept{
        if(!Positive(c)||c.module!=source_.module||c.thread!=source_.thread||c.engine!=source_.engine||
           c.viewport!=source_.viewport||c.world!=source_.world||c.mode!=source_.mode||c.gameInstance!=source_.gameInstance||
           c.players!=source_.players||c.player!=source_.player||c.controller!=source_.controller||
           c.hud!=source_.hud||c.stack!=source_.stack)return false;
        for(const auto i:{0U,1U,3U})if(!(c.identities[i]==source_.identities[i]))return false;
        return true;
    }
    bool Candidate(const active::Context& c)const noexcept{
        return Infrastructure(c)&&c.type==source_.type&&c.widget!=departing_&&c.array.count==source_.array.count;
    }
public:
    State CurrentState()const noexcept{return state_;}
    const active::Context& Source()const noexcept{return source_;}
    bool NeedsRelabel()const noexcept{return state_==State::Relabeling;}
    bool Bind(const active::Context& source,std::uintptr_t counselor)noexcept{
        if(state_!=State::Empty||!Positive(source)||source.array.count<2||!cfgprobe::ObjectPointer(source.type)||
           !cfgprobe::ObjectPointer(counselor)||counselor==source.type)return false;
        source_=source;counselor_=counselor;state_=State::Ready;return true;
    }
    bool Begin(const active::Context& actual,const cfgprobe::lifetime::SlotObservation& admitted,
        const cfgprobe::lifetime::SlotObservation& owningController)noexcept{
        if(state_!=State::Ready||!Infrastructure(actual)||actual.type!=counselor_||
           !(actual.identities[2]==admitted)||!admitted.HasAssignedSerial()||
           !(actual.identities[1]==owningController)||!owningController.HasAssignedSerial())return false;
        ++generation_;departing_=actual.widget;pending_={};state_=State::AwaitEntry;return true;
    }
    template<class Host> bool Observe(Host& host,const active::Context& actual,Ticket& out)noexcept{
        static_assert(noexcept(host.ReturnCurrent(source_,actual)));
        out={};if(state_!=State::AwaitEntry||!Candidate(actual)||!host.ReturnCurrent(source_,actual))return false;
        pending_={generation_,actual};out=pending_;state_=State::Relabeling;return true;
    }
    // Customization or map/settings Back can pop directly to Offline Play
    // without invoking the admitted counselor's Back callback. Observe still
    // requires the full native proof before any fresh labels become ready.
    bool BeginSelectionReturn(const active::Context& actual)noexcept{
        if(state_!=State::Ready||!Candidate(actual))return false;
        ++generation_;departing_=0;pending_={};state_=State::AwaitEntry;return true;
    }
    // The host must finish fresh child/label binding and label readback before
    // committing; a ticket alone never means labels are ready or enables input.
    template<class Host> bool CommitRelabeled(Host& host,const Ticket& ticket)noexcept{
        static_assert(noexcept(host.ReturnCurrent(source_,ticket.entry)));
        static_assert(noexcept(host.RelabeledCurrent(ticket.entry)));
        if(state_!=State::Relabeling||ticket.generation!=generation_||!(ticket.entry==pending_.entry)||
           !Candidate(ticket.entry)||!host.ReturnCurrent(source_,ticket.entry)||!host.RelabeledCurrent(ticket.entry))return false;
        source_=ticket.entry;pending_={};departing_=0;state_=State::Ready;return true;
    }
    void Retire()noexcept{++generation_;pending_={};state_=State::Retired;}
};
}
