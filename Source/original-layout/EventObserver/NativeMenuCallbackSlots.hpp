#pragma once
#include "PendingSandboxStart.hpp"
#include <array>
namespace cfgmenu::callbackslots {
constexpr unsigned Maximum=10;
enum class State {Empty,Publishing,Installed,Restoring,Restored,Refused,Indeterminate};
struct Cell {std::uintptr_t address=0,original=0,replacement=0;};
struct Plan {std::array<Cell,Maximum> cells{};unsigned count=0;};
inline bool Shape(const Plan& p)noexcept{
    if(!p.count||p.count>Maximum)return false;
    for(unsigned i=0;i<p.count;++i){const auto& c=p.cells[i];if(!cfgprobe::ObjectPointer(c.address)||!cfgprobe::ObjectPointer(c.original)||
        !cfgprobe::ObjectPointer(c.replacement)||c.original==c.replacement)return false;
        for(unsigned j=0;j<i;++j)if(p.cells[j].address==c.address)return false;}
    return true;
}
// Exact class-local executor cells only. The host owns metadata/source proof,
// resident replacement thunks and pointer exchange. Publish at most ONE cell
// per invocation; no reader/callback work is hidden in the admission policy.
class Transaction {
    Plan plan_{};State state_=State::Empty;unsigned installed_=0;bool rollback_=false;
public:
    State CurrentState()const noexcept{return state_;}
    unsigned Installed()const noexcept{return installed_;}
    const Plan& BoundPlan()const noexcept{return plan_;}
    template<class Host> bool PrepareInherited(Host& host,const Plan& p,const Plan& prior,unsigned prefix){
        if(state_!=State::Empty||!prefix||prefix>=p.count||!Shape(p)||!Shape(prior)||prefix>prior.count)return false;
        for(unsigned i=0;i<prefix;++i){const auto& a=p.cells[i];const auto& b=prior.cells[i];
            if(a.address!=b.address||a.original!=b.original||a.replacement!=b.replacement)return false;}
        if(!host.Resident(p)||!host.Current(p,prefix)){state_=State::Refused;return false;}
        plan_=p;installed_=prefix;state_=State::Publishing;return true;
    }
    template<class Host> bool Prepare(Host& host,const Plan& p){
        if(state_!=State::Empty)return false;
        if(!Shape(p)||!host.Resident(p)||!host.Current(p,0)){state_=State::Refused;return false;}
        plan_=p;state_=State::Publishing;return true;
    }
    template<class Host> bool Step(Host& host){
        using W=sandboxstart::WriteResult;
        if(state_!=State::Publishing&&state_!=State::Restoring)return false;
        if(!host.Current(plan_,installed_)){state_=State::Indeterminate;return false;}
        if(state_==State::Restoring){
            if(!installed_){state_=rollback_?State::Refused:State::Restored;return false;}
            const auto& c=plan_.cells[installed_-1];
            if(host.Exchange(c.address,c.replacement,c.original)!=W::Changed){state_=State::Indeterminate;return false;}
            --installed_;if(!host.Current(plan_,installed_)){state_=State::Indeterminate;return false;}
            if(!installed_)state_=rollback_?State::Refused:State::Restored;return true;
        }
        const auto& c=plan_.cells[installed_];const auto result=host.Exchange(c.address,c.original,c.replacement);
        if(result!=W::Changed){
            if(result==W::Refused&&host.Current(plan_,installed_)){rollback_=true;state_=installed_?State::Restoring:State::Refused;}
            else state_=State::Indeterminate;return false;
        }
        ++installed_;if(!host.Current(plan_,installed_)){state_=State::Indeterminate;return false;}
        if(installed_==plan_.count)state_=State::Installed;return true;
    }
    template<class Host> bool Restore(Host& host){
        if(state_!=State::Installed||!host.Current(plan_,installed_)){state_=State::Indeterminate;return false;}
        rollback_=false;state_=State::Restoring;return true;
    }
};
}
