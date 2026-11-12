#pragma once
#include <array>
#include <cstdint>
namespace cfgmenu::continuation {
constexpr unsigned MaximumCells=65;
enum class Write {Changed,Refused,Indeterminate};
enum class State {Empty,Installing,Ready,Restoring,Refused,Detached,Indeterminate};
struct Cell {std::uintptr_t address=0,previous=0;};
struct Plan {std::uintptr_t settingsClass=0;std::array<Cell,MaximumCells> cells{};unsigned count=0;};
// One current Sandbox menu only. The native host proves exact typed ownership;
// this transaction never edits pending mode/map, possession, a CDO or scripts.
// Publish pending-settings first, then each populated map button's authored
// continuation. No native calls or allocations, no catch-up burst on a hitch.
class Transaction {
    Plan plan_{};State state_=State::Empty;unsigned index_=0;std::uint64_t last_=0;bool paced_=false;
public:
    template<class Host> bool Prepare(Host& host){
        if(state_!=State::Empty)return false;Plan candidate;
        if(!host.Observe(candidate)||!candidate.settingsClass||candidate.settingsClass%8||candidate.count<2||candidate.count>MaximumCells){state_=State::Refused;return false;}
        for(unsigned i=0;i<candidate.count;++i){const auto& cell=candidate.cells[i];
            if(!cell.address||cell.address%8||cell.previous%8){state_=State::Refused;return false;}
            for(unsigned j=0;j<i;++j)if(candidate.cells[j].address==cell.address){state_=State::Refused;return false;}
        }
        plan_=candidate;state_=State::Installing;return true;
    }
    template<class Host> bool Step(Host& host,std::uint64_t now){
        if(state_!=State::Installing&&state_!=State::Restoring)return false;
        if(paced_&&(now<last_||now-last_<25))return false;last_=now;paced_=true;
        if(!host.Current(plan_)){state_=State::Detached;return false;}
        if(state_==State::Restoring){
            if(!index_){state_=State::Refused;return true;}const auto& cell=plan_.cells[index_-1];
            const auto result=host.Exchange(cell,plan_.settingsClass,cell.previous);
            if(result!=Write::Changed){state_=result==Write::Indeterminate?State::Indeterminate:State::Detached;return false;}
            --index_;return true;
        }
        const auto& cell=plan_.cells[index_];const auto result=host.Exchange(cell,cell.previous,plan_.settingsClass);
        if(result==Write::Indeterminate){state_=State::Indeterminate;return false;}
        if(result==Write::Refused){state_=State::Restoring;return false;}
        if(++index_==plan_.count){
            // A successful final CAS does not prove earlier cells stayed ours.
            // Verify all published cells while the same native menu is current.
            state_=host.Published(plan_)?State::Ready:State::Restoring;
        }return true;
    }
    State CurrentState()const noexcept{return state_;}
    unsigned Published()const noexcept{return index_;}
    const Plan& Evidence()const noexcept{return plan_;}
};
}
