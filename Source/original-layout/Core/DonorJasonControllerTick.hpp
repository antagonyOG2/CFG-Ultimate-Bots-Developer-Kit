#pragma once
#include <cstdint>
#include "DonorNavigationYield.hpp"
#include "DonorJasonCombat.hpp"
namespace cfgdonorhunt {
// Active FEAT RunOfflineBotsControllerTickOnGameThread12773-12933.
// Native reads/calls are host seams, resolved by exact3.5 bindings and the
// existing shared identity/root/read/call admission. This is not a hook.
template<class H>auto TrapSlashPass(H& h,int)->decltype(h.TrapSlashTurn(),DonorRunTrapSlash(h)){return CFG_H243_TRAP_SLASH&&h.TrapSlashTurn()&&DonorRunTrapSlash(h);}
template<class H>bool TrapSlashPass(H&,long){return false;}
template<class Host>bool DonorRunOfflineBotsControllerTickOnGameThread(Host& host){
    if(host.StartupTrapSetupActive()){
        // H2.43: slash first (one press); otherwise the trap phase continues
        // exactly as before. A press consumes only this slice.
        if(TrapSlashPass(host,0))return true;
        host.RunJasonAIStartupTrapSetupOnGameThread();return true;
    }
    if(host.TickJasonAIDoorBreakOnGameThread())return true;
    if(host.KnifeSequenceActive()){
        host.RunJasonAICombatOnGameThread();return true;
    }
    const bool chaseOK=host.RunJasonAIChaseUpdateOnGameThread();
    if(!chaseOK)return false;
    if(DonorNavigationPending(host,0))return true;
    const auto jason=host.Jason();
    if(!jason||!host.ReadableActor(jason))return true;
    // This MUST precede special-state tests: it owns owed attack release
    // and held-counselor input even when a preceding action changed state.
    host.RunJasonAICombatOnGameThread();
    if(DonorNavigationPending(host,0))return true;
    std::uint8_t first=0,second=0;
    if(!host.ReadDriverBlockingStates(jason,first,second))return true;
    if(first!=0||second!=0)return true;
    std::uintptr_t driver=0;
    if(!host.ReadVerifiedDriverSlot(jason,driver)||!driver)return true;
    const bool driverCallOK=host.SafeJasonDriverCall(jason,driver);
    if(!host.DriverTailLogged()){
        host.SetDriverTailLogged();host.LogDriverTail(driverCallOK);
    }
    return true;
}
}
