#include <cstdio>
#include "../Core/HostedEndFreezePolicy.hpp"
#include "HostedH1Report.hpp"

int main(){
    struct Reader {bool Read(std::uintptr_t at,std::uintptr_t& value){
        value=at==2?0:0x24000;return at!=3;
    }} reader;
    cfgh1::OutroEvidence outro{};
    outro.Read(reader,1,outro.localState,1);outro.Read(reader,2,outro.localPawn,0);
    outro.Read(reader,3,outro.killerState,5);
    if(outro.reads!=3||outro.localPawn||outro.localState!=0x24000||outro.killerState)return 5;
    bool frozen=false,adopted=true,restorePending=true;unsigned restoreCalls=0;
    unsigned freezes=0;unsigned long long frozenAt=0,watchAt=10;
    if(!cfghostedend::DriverActive(frozen,adopted)||
       !cfghostedend::Freeze(frozen,frozenAt,freezes,20)||
       cfghostedend::DriverActive(frozen,adopted)||frozenAt!=20||freezes!=1)return 1;
    // Freeze gates only adopted driving. Existing end-phase restoration still runs.
    if(restorePending){++restoreCalls;restorePending=false;}
    if(restoreCalls!=1||restorePending)return 2;
    // Existing world retirement clears the freeze and permits the next adoption.
    adopted=false;cfghostedend::ClearForRetirement(frozen,watchAt);
    if(frozen||watchAt||cfghostedend::DriverActive(frozen,adopted))return 3;
    cfghostedend::ClearForActivation(frozen,watchAt);adopted=true;
    if(!cfghostedend::DriverActive(frozen,adopted)||watchAt)return 4;
    std::puts("PASS end freeze preserves class restore and new-world second adoption");return 0;
}
