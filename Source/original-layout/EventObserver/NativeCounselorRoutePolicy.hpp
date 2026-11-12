#pragma once
#include "../RuntimeProbe/ActiveMenuObservation.hpp"
namespace cfgmenu::counselorroute {
enum class State {Empty,EntryReady,EntryRequested,AwaitCounselor,CounselorReady,AcceptRequested,AwaitMap,MapReady,SettingsReady,Submitted,Retired};
enum class Step {Entry,AcceptedCounselor};
struct Token {
    std::uint64_t generation=0;Step step=Step::Entry;
    std::uintptr_t receiver=0,previous=0,destination=0;
    cfgprobe::lifetime::SlotObservation identity{};
};
// Admission policy only. The host must independently prove actual borrowed
// callback, OwningPlayer, unchanged source references and real destination
// consumer before using a token. This never writes metadata, frames or saves.
class Transaction {
    State state_=State::Empty;active::Context source_{};
    std::uintptr_t counselor_=0,map_=0,settings_=0,sandboxMap_=0;
    cfgprobe::lifetime::SlotObservation receiver_{};
    std::uint64_t generation_=0;
    bool Infrastructure(const active::Context& current)const noexcept{
        if(current.module!=source_.module||current.thread!=source_.thread||current.world!=source_.world||
           current.mode!=source_.mode||current.controller!=source_.controller||current.hud!=source_.hud||
           current.stack!=source_.stack||current.gameInstance!=source_.gameInstance)return false;
        for(const auto i:{0U,1U,3U})if(!current.identities[i].HasAssignedSerial()||!(current.identities[i]==source_.identities[i]))return false;
        return true;
    }
    bool Receiver(const active::Context& current)const noexcept{
        return Infrastructure(current)&&current.widget==receiver_.object&&current.identities[2]==receiver_&&receiver_.HasAssignedSerial();
    }
public:
    State CurrentState()const noexcept{return state_;}
    bool Bind(const active::Context& source,std::uintptr_t counselor,std::uintptr_t map,std::uintptr_t settings,std::uintptr_t sandboxMap){
        if(state_!=State::Empty||!source.thread||!source.identities[2].HasAssignedSerial()||source.widget!=source.identities[2].object||
           !cfgprobe::ObjectPointer(counselor)||!cfgprobe::ObjectPointer(map)||!cfgprobe::ObjectPointer(settings)||!cfgprobe::ObjectPointer(sandboxMap)||
           counselor==map||counselor==settings||counselor==sandboxMap||map==settings||map==sandboxMap||settings==sandboxMap)return false;
        source_=source;if(source.identities[0].object!=source.world||source.identities[1].object!=source.controller||
            source.identities[3].object!=source.hud||!Infrastructure(source))return false;
        counselor_=counselor;map_=map;settings_=settings;sandboxMap_=sandboxMap;receiver_=source.identities[2];state_=State::EntryReady;return true;
    }
    bool RequestEntry(const active::Context& current){
        if(state_!=State::EntryReady||!(current==source_)||!Receiver(current))return false;
        ++generation_;state_=State::EntryRequested;return true;
    }
    bool RequestAccept(const active::Context& current){
        if(state_!=State::CounselorReady||current.type!=counselor_||!Receiver(current))return false;
        ++generation_;state_=State::AcceptRequested;return true;
    }
    bool Borrow(const active::Context& current,Step step,std::uintptr_t previous,Token& out)const noexcept{
        out={};if(!Receiver(current))return false;
        if(step==Step::Entry){if(state_!=State::EntryRequested||current.type!=source_.type||previous!=sandboxMap_)return false;}
        else if(step==Step::AcceptedCounselor){if(state_!=State::AcceptRequested||current.type!=counselor_||previous)return false;}
        else return false;
        out={generation_,step,current.widget,previous,step==Step::Entry?counselor_:map_,receiver_};return true;
    }
    // Consume before native forwarding so nested or repeated callbacks cannot
    // redirect twice. A failed native forward retires, never auto-retries.
    bool Consume(const active::Context& current,const Token& token){
        Token expected;if(!Borrow(current,token.step,token.previous,expected)||token.generation!=expected.generation||
           token.receiver!=expected.receiver||token.destination!=expected.destination||!(token.identity==expected.identity))return false;
        state_=token.step==Step::Entry?State::AwaitCounselor:State::AwaitMap;return true;
    }
    bool Arrive(const active::Context& current){
        if(!Infrastructure(current)||!current.identities[2].HasAssignedSerial()||current.widget!=current.identities[2].object)return false;
        State next;
        if(state_==State::AwaitCounselor&&current.type==counselor_)next=State::CounselorReady;
        else if((state_==State::AwaitMap||state_==State::MapReady||state_==State::SettingsReady)&&current.type==map_)next=State::MapReady;
        else if((state_==State::MapReady||state_==State::SettingsReady)&&current.type==settings_)next=State::SettingsReady;
        else return false;
        receiver_=current.identities[2];state_=next;return true;
    }
    bool RevisitCounselor(const active::Context& current,bool guardedConstruct=false){
        if((state_!=State::MapReady&&state_!=State::SettingsReady)||!Infrastructure(current)||
           (!guardedConstruct&&current.type!=counselor_)||!current.identities[2].HasAssignedSerial()||current.identities[2].object!=current.widget)return false;
        ++generation_;receiver_={};state_=State::AwaitCounselor;return true;
    }
    bool ReturnToEntry(const active::Context& current){
        // Customization can replace the counselor widget and pop directly to
        // Offline Play without the admitted instance's Back callback. Only the
        // actual positive entry top may cancel that pre-launch route; this is
        // not permission to adopt a foreign counselor receiver or submitted game.
        if((state_!=State::MapReady&&state_!=State::SettingsReady&&state_!=State::AwaitMap&&
            state_!=State::AwaitCounselor&&state_!=State::CounselorReady)||!Infrastructure(current)||
           (state_==State::AwaitCounselor&&current.widget==source_.widget)||
           current.type!=source_.type||current.array.count!=source_.array.count||!current.identities[2].HasAssignedSerial()||current.identities[2].object!=current.widget)return false;
        ++generation_;receiver_=current.identities[2];state_=State::EntryReady;return true;
    }
    bool Submit(const active::Context& current){
        if(state_!=State::SettingsReady||current.type!=settings_||!Receiver(current))return false;
        state_=State::Submitted;return true;
    }
    bool Cancel(const active::Context& current){
        if(!Receiver(current)||(state_!=State::EntryRequested&&state_!=State::CounselorReady&&state_!=State::AcceptRequested))return false;
        ++generation_;receiver_=source_.identities[2];state_=State::EntryReady;return true;
    }
    bool CancelAccept(const active::Context& current){
        if(state_!=State::AcceptRequested||current.type!=counselor_||!Receiver(current))return false;
        ++generation_;state_=State::CounselorReady;return true;
    }
    // Caller owns a completed Back-return proof and fresh native labels. Do not
    // reset generation: old return/accept tokens must stay permanently stale.
    bool RebindEntry(const active::Context& current){
        if(state_!=State::EntryReady||!Infrastructure(current)||current.type!=source_.type||current.array.count!=source_.array.count||
           !current.identities[2].HasAssignedSerial()||current.widget!=current.identities[2].object)return false;
        ++generation_;source_=current;receiver_=current.identities[2];return true;
    }
    void Retire()noexcept{++generation_;state_=State::Retired;}
};
}
