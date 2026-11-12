#pragma once
#include "SettingsStartRouter.hpp"
#include "PendingSandboxStart.hpp"
namespace cfgmenu::startslots {
enum class State {Empty,Publishing,Installed,Restored,Refused,Indeterminate};
// Publication/retirement transaction, not lifetime adoption. Thunks remain
// resident and must forward unrelated receivers to the saved native executor.
// No shared interpreter/ProcessEvent patch; no wrapper-only successful claim.
class Slots {
    State state_=State::Empty;
    std::uintptr_t original_=0,wrapper_=0,graph_=0;
public:
    State Current()const noexcept{return state_;}
    // Shared class slots already belong to these resident thunks. A new native
    // widget may adopt them only after the same full metadata/owner proof.
    template<class Host> bool Adopt(Host& host,std::uintptr_t original,std::uintptr_t wrapper,std::uintptr_t graph){
        if(state_!=State::Empty||!host.ThunksResident(wrapper,graph)||!host.Current(wrapper,graph))return false;
        original_=original;wrapper_=wrapper;graph_=graph;state_=State::Installed;return true;
    }
    template<class Host> bool Install(Host& host,std::uintptr_t original,std::uintptr_t wrapper,std::uintptr_t graph){
        if(state_!=State::Empty)return false;
        state_=State::Publishing;original_=original;wrapper_=wrapper;graph_=graph;
        if(!cfgprobe::ObjectPointer(original)||!cfgprobe::ObjectPointer(wrapper)||!cfgprobe::ObjectPointer(graph)||
            original==wrapper||original==graph||wrapper==graph||!host.ThunksResident(wrapper,graph)||!host.Current(original,original)){
            state_=State::Refused;return false;}
        using W=sandboxstart::WriteResult;
        const auto first=host.Exchange(startroute::SlotKind::Wrapper,original,wrapper);
        if(first!=W::Changed){state_=first==W::Refused?State::Refused:State::Indeterminate;return false;}
        if(!host.Current(wrapper,original)){state_=State::Indeterminate;return false;}
        const auto second=host.Exchange(startroute::SlotKind::Graph,original,graph);
        if(second!=W::Changed){
            // Restore only a definitely unmodified second slot and an intact
            // live menu. An exception/lost owner is not rollback permission.
            if(second==W::Refused&&host.Current(wrapper,original)&&
                host.Exchange(startroute::SlotKind::Wrapper,wrapper,original)==W::Changed&&host.Current(original,original))state_=State::Refused;
            else state_=State::Indeterminate;
            return false;
        }
        if(!host.Current(wrapper,graph)){state_=State::Indeterminate;return false;}
        state_=State::Installed;return true;
    }
    template<class Host> bool Restore(Host& host){
        if(state_!=State::Installed)return state_==State::Restored;
        state_=State::Publishing;
        if(!host.Current(wrapper_,graph_)){state_=State::Indeterminate;return false;}
        using W=sandboxstart::WriteResult;
        if(host.Exchange(startroute::SlotKind::Graph,graph_,original_)!=W::Changed||!host.Current(wrapper_,original_)||
            host.Exchange(startroute::SlotKind::Wrapper,wrapper_,original_)!=W::Changed||!host.Current(original_,original_)){
            state_=State::Indeterminate;return false;}
        state_=State::Restored;return true;
    }
};
}
