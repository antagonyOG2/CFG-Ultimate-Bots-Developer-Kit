#pragma once
#include "HostedLobbyReport.hpp"
namespace cfghostedlobbyinputs {
// Lexical publisher also runs on warm-up/factory early returns. Once an actual
// provider refusal exists, later empty providers cannot erase or fabricate it.
template<class Provider,class Access> void RecordInputs(::cfghostedlobby::Report& report,const Provider& provider,const Access* access,unsigned scope){
    report.rowInputAttempts+=provider.InputAttempts();report.rowInputFailures+=provider.InputFailures();
    if(!access||report.rowRootReason||!provider.Failure().reason)return;
    const auto& f=provider.Failure();
    report.rowTypeReason=access->typeReason;report.rowCurrentReason=access->currentReason;report.rowInvokeReason=access->invokeReason;
    report.rowRootStage=provider.Stage();report.rowRootReason=f.reason;report.rowRootRole=provider.Role();report.rowRootScopeFailure=scope;
    report.rowRootValue=f.value;report.rowRootSerial=f.slot.serial;report.rowRootInternal=f.internal;report.rowRootFlags=f.flags;
    report.rowOwners=provider.Owners();
}
}
