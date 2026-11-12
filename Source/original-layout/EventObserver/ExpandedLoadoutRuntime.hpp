#pragma once
#include "LoadedItemClassSurvey.hpp"
namespace cfgloadout::expanded {
enum class Failure:unsigned {None,Deadline,Owner,ClassUnavailable,Readiness,Grant,Verification};
struct Diagnostics {
    unsigned slot=2,choice=0,phase=0,failure=0,nativeStage=0,slices=0,visited=0,cursor=0,total=0,readDeferrals=0,restarts=0,lastReads=0;
    unsigned preparations=0,grantAttempts=0,verifications=0;
    std::uintptr_t type=0;
};
class Runtime {
    cfgofflinesettings::Request owner_{};Selection selection_{};Slot slot_=Slot::Complete;
    restricted::State state_=restricted::State::Empty;Evidence before_{};ClassSurvey survey_{};
    std::uint64_t next_=0,deadline_=0;std::uintptr_t type_=0;unsigned attempts_=0;
    bool prepared_=false,servicing_=false,failed_=false,mutated_=false;
    unsigned Choice()const{return slot_==Slot::Weapon?selection_.weapon:selection_.inventory;}
    void Advance(std::uint64_t now,Failure failure){
        failed_|=failure!=Failure::None;if(failure!=Failure::None)report.failure=static_cast<unsigned>(failure);
        slot_=slot_==Slot::Weapon?Slot::Inventory:Slot::Complete;
        if(slot_==Slot::Inventory&&!selection_.inventory)slot_=Slot::Complete;
        state_=slot_==Slot::Complete?(failed_?(mutated_?restricted::State::Indeterminate:restricted::State::Unavailable):restricted::State::Verified):restricted::State::Waiting;
        type_=0;attempts_=0;prepared_=false;before_={};survey_={};next_=now+1000;deadline_=now+90000;
    }
    void SurveyReport(){report.phase=survey_.phase;report.slices=survey_.slices;report.visited=survey_.visited;
        report.cursor=survey_.cursor;report.total=survey_.total;report.readDeferrals=survey_.readDeferrals;
        report.restarts=survey_.restarts;report.lastReads=survey_.lastReads;report.type=survey_.found;}
public:
    Diagnostics report{};
    bool Begin(cfgofflinesettings::Request owner,Selection selected,std::uint64_t now){
        if(state_!=restricted::State::Empty||!owner.Valid()||!Valid(selected)||selected.weapon==1||selected.inventory==1)return false;
        owner_=owner;selection_=selected;slot_=selected.weapon?Slot::Weapon:selected.inventory?Slot::Inventory:Slot::Complete;
        state_=slot_==Slot::Complete?restricted::State::Preserved:restricted::State::Waiting;next_=now+3000;deadline_=now+90000;
        report.slot=static_cast<unsigned>(slot_);report.choice=slot_==Slot::Complete?0:Choice();return true;
    }
    template<class Reader,class Host> bool Step(Reader& reader,Host& host,std::uintptr_t module,std::uint64_t now){
        if(servicing_||!Active()||now<next_)return false;
        struct Guard{bool& flag;Guard(bool& f):flag(f){flag=true;}~Guard(){flag=false;}} guard(servicing_);
        next_=now+25;report.slot=static_cast<unsigned>(slot_);report.choice=Choice();
        if(now>deadline_){Advance(now,Failure::Deadline);return false;}
        // A transient read budget/owner observation failure is not a grant and
        // must not consume a slot. The same finite deadline still bounds it.
        if(!host.Current(owner_)){++report.readDeferrals;report.failure=static_cast<unsigned>(Failure::Owner);return false;}
        if(!type_){
            const bool progressed=survey_.Step(reader,module,owner_.world,slot_,Choice());SurveyReport();
            if(survey_.phase==2){type_=survey_.found;report.failure=0;}
            else if(survey_.phase==3)Advance(now,Failure::ClassUnavailable);
            return progressed;
        }
        NativeAccess<Reader,Host> access(reader,host,{owner_,slot_,Choice()},module,type_);next_=now+1000;
        if(state_==restricted::State::Dispatched){++report.verifications;const bool okay=access.Verify(before_);
            report.nativeStage=access.lastStage;report.lastReads=access.lastReads;Advance(now,okay?Failure::None:Failure::Verification);return true;}
        if(!prepared_){if(++attempts_>15){Advance(now,Failure::Readiness);return false;}
            ++report.preparations;prepared_=access.Observe(before_,true);report.nativeStage=access.lastStage;report.lastReads=access.lastReads;return prepared_;}
        const bool okay=access.Grant(before_);report.nativeStage=access.lastStage;report.lastReads=access.lastReads;
        if(!access.grantDispatched){prepared_=false;++report.readDeferrals;report.failure=static_cast<unsigned>(Failure::Readiness);return false;}
        state_=restricted::State::Dispatched;++report.grantAttempts;mutated_=true;
        if(!okay)Advance(now,Failure::Grant);
        else report.failure=0;
        return true; // native ambiguity is consumed; preflight refusal is not a grant
    }
    bool Active()const{return state_==restricted::State::Waiting||state_==restricted::State::Dispatched;}
    restricted::State CurrentState()const{return state_;}
};
}
