#include "../Core/DonorJasonControllerTick.hpp"
#include <cassert>
#include <cstdio>
#include <string>
struct Host {
    std::string order;
    bool traps=false,door=false,knife=false,chase=true,readable=true,states=true,slot=true,call=true,logged=false;
    std::uintptr_t jason=1,driver=2;
    std::uint8_t first=0,second=0;
    unsigned driverCalls=0,logs=0;
    bool StartupTrapSetupActive(){order+='T';return traps;}
    void RunJasonAIStartupTrapSetupOnGameThread(){order+='t';}
    bool TickJasonAIDoorBreakOnGameThread(){order+='D';return door;}
    bool KnifeSequenceActive(){order+='K';return knife;}
    bool RunJasonAIChaseUpdateOnGameThread(){order+='C';return chase;}
    std::uintptr_t Jason(){order+='J';return jason;}
    bool ReadableActor(std::uintptr_t){order+='R';return readable;}
    void RunJasonAICombatOnGameThread(){order+='M';}
    bool ReadDriverBlockingStates(std::uintptr_t,std::uint8_t& a,std::uint8_t& b){order+='S';a=first;b=second;return states;}
    bool ReadVerifiedDriverSlot(std::uintptr_t,std::uintptr_t& out){order+='V';out=driver;return slot;}
    bool SafeJasonDriverCall(std::uintptr_t,std::uintptr_t){order+='N';++driverCalls;return call;}
    bool DriverTailLogged(){return logged;}
    void SetDriverTailLogged(){logged=true;}
    void LogDriverTail(bool){order+='L';++logs;}
};
int main(){unsigned checks=0;const auto check=[&](bool x){++checks;assert(x);};
    {Host h;check(cfgdonorhunt::DonorRunOfflineBotsControllerTickOnGameThread(h)&&h.order=="TDKCJRMSVNL");
        h.order.clear();check(cfgdonorhunt::DonorRunOfflineBotsControllerTickOnGameThread(h)&&h.order=="TDKCJRMSVN"&&h.logs==1);}
    {Host h;h.traps=true;check(cfgdonorhunt::DonorRunOfflineBotsControllerTickOnGameThread(h)&&h.order=="Tt"&&!h.driverCalls);}
    {Host h;h.door=true;check(cfgdonorhunt::DonorRunOfflineBotsControllerTickOnGameThread(h)&&h.order=="TD");}
    {Host h;h.knife=true;check(cfgdonorhunt::DonorRunOfflineBotsControllerTickOnGameThread(h)&&h.order=="TDKM");}
    {Host h;h.chase=false;check(!cfgdonorhunt::DonorRunOfflineBotsControllerTickOnGameThread(h)&&h.order=="TDKC");}
    for(unsigned fault=0;fault<8;++fault){Host h;
        switch(fault){case 0:h.jason=0;break;case 1:h.readable=false;break;case 2:h.states=false;break;
            case 3:h.first=1;break;case 4:h.second=1;break;case 5:h.slot=false;break;case 6:h.driver=0;break;case 7:h.call=false;break;}
        check(cfgdonorhunt::DonorRunOfflineBotsControllerTickOnGameThread(h));
        check(h.driverCalls==(fault==7?1u:0u));
        if(fault>=2)check(h.order.find('M')<h.order.find('S')); // combat/release must not be hidden by a busy state
    }
    {struct PendingHost:Host {bool NavigationPending(){return true;}} h;
        check(cfgdonorhunt::DonorRunOfflineBotsControllerTickOnGameThread(h)&&h.order=="TDKC"&&!h.driverCalls);}
    // Cadence test only: driver wrapper runs on each admitted callback; no
    // port lane timing/backoff is inserted. Full chase/60s gameplay sim pending.
    {Host h;for(unsigned frame=0;frame<3600;++frame){h.order.clear();check(cfgdonorhunt::DonorRunOfflineBotsControllerTickOnGameThread(h));}
        check(h.driverCalls==3600&&h.logs==1);}
    std::printf("J1a active donor controller order: %u checks PASS; trap/door/knife priority, chase/combat/busy/native tail, each admitted callback. Production host pending.\n",checks);
}
