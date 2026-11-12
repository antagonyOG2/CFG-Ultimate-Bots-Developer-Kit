#pragma once
#include <cstdint>
namespace cfgmenu::retention {
struct StartupEntry {
    std::uint32_t candidates=0,originPassed=0,ownerPassed=0,forwards=0,stage=0,reads=0,error=0;
    std::uintptr_t receiver=0,returnPc=0,expectedPc=0,world=0;
};
struct Report {
    std::uint32_t bytes=sizeof(Report),version=7,generation=0;
    std::uint32_t candidates=0,admitted=0,inputs=0,nativeEntries=0,outputs=0,factories=0;
    std::uint32_t failures=0,lastFailure=0,owners=0,peakClaims=0,poisoned=0;
    std::uint32_t registryStage=0,registryReads=0,registryGate=0,registryTick=0;
    std::uintptr_t receiver=0,function=0,returnPc=0,expectedReceiver=0,expectedFunction=0,expectedPc=0;
    std::uint32_t constructCandidates=0,constructAdmitted=0,constructForwards=0,constructStage=0,constructRoots=0;
    std::uintptr_t constructReturnPc=0,constructExpectedPc=0;
    StartupEntry startup[2]{};
    std::uint32_t startupRoots=0,startupClaims=0,startupPoisoned=0,startupFailures=0,startupLastFailure=0;
    std::uint32_t startupCalls=0,startupReturns=0,startupRegistryReads=0;
    std::uint32_t startupWindowAttempts=0,startupLockRetries=0,startupWindowRefusals=0;
    std::uint32_t startupPrepareStage=0,startupPrepareCall=0,startupWindowStage=0,startupWindowGate=0,startupWindowContended=0;
    std::uint32_t startCandidates=0,startAdmitted=0,startFailure=0,startCalls=0,startReturns=0;
    std::uint32_t startExecutorReturns=0,startRetirements=0,startClaims=0,startLivePoison=0,startCompletionStage=0,startLockRetries=0;
    std::uintptr_t startPc=0,startExpectedPc=0;
};
}
