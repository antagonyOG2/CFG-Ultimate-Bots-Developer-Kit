#pragma once
#include <cstdint>
#include <array>
#include "../Core/ChallengeBoundaryWallCensusPolicy.hpp"
#include "../Core/ChallengeEdgeWallsPolicy.hpp"
#include "../RuntimeProbe/PickerCallbackDiagnostic.hpp"
#include "../Core/NativeStartupDiagnostic.hpp"
#include "../Core/MenuWarmDiagnostic.hpp"
namespace cfgmenu {
constexpr std::uint32_t FrontendReportVersion=CFG_J218_FINISHING?97:CFG_J215_CHALLENGE_WALL_OWNERSHIP?96:95;
constexpr std::size_t FrontendReportReadCap=CFG_J215_CHALLENGE_WALL_OWNERSHIP?65536:32768;
constexpr unsigned FrontendEpochCapacity=64; // per-epoch cached widget views; NOT number of match/menu epochs
struct FrontendReport {
    std::uint32_t bytes=sizeof(FrontendReport),version=FrontendReportVersion,state=0,ownerCallbacks=0;
    std::uint32_t epochs=0,epochStage=0,epochReads=0,epochFailure=0;
    std::uint32_t instanceReentryChecks=0,instanceReentries=0,instanceReentryFailure=0;
    std::uintptr_t epochActualType=0,epochExpectedType=0,epochActualWidget=0;
    std::uint32_t epochActualDepth=0,epochExpectedDepth=0;
    std::array<char,96> epochActualTypeName{};
    std::uint32_t reads=0,error=0,hudSamples=0,hudPhases=0;
    std::uint32_t arrivalChecks=0,arrivalReads=0,arrivalFailure=0,resourceStep=0,spawnQueued=0,continuationCells=0;
    std::uintptr_t world=0,pawn=0;
    std::uint32_t setupStage=0,maximumSetupReads=0;
    std::uint32_t warmStage=0,rowViewState=0,rowPrepareStage=0;
    std::uint32_t freshStep=0,birthStep=0,defaultIndex=0,birthFlags=0,activeNonempty=0,birthReads=0,nativeActiveVerified=0;
    std::uintptr_t birthPawn=0,birthState=0,birthType=0,birthSpawned=0;
    std::uint32_t handoffFailure=0;
    std::uint32_t entryStage=0,entryReads=0,entryMaximumReads=0,entryFailure=0;
    std::uint32_t entryLabelFailure=0,entryCurrentFailure=0;
    std::uint64_t entrySetupStarted=0,entrySetupElapsedMs=0;
    std::uint32_t entrySetupCallbacks=0;
    std::uint32_t pickerPreparationStage=0,callbackState=0,callbackCells=0,routeState=0;
    std::uint32_t routeReads=0,routeMaximumReads=0,constructorAdmissions=0,ownerGetterCalls=0;
    std::uint32_t entryRedirects=0,acceptRedirects=0,routeFailure=0;
    std::uint32_t nativeMetadataSerialAssignments=0;
    std::uint32_t routeArrivalStage=0,routeArrivalReads=0,entryOrderState=0,entryOrderFailure=0;
    std::uint32_t entryOrderCaptureStage=0,handoffBirthStep=0,handoffBirthReads=0,handoffRelationship=0;
    std::uintptr_t handoffExpectedCounselor=0,handoffActualCounselor=0;
    std::uint32_t acceptedChoiceStage=0,acceptedChoiceReads=0,acceptedChoiceIndex=0,acceptedChoiceCommitted=0;
    std::array<char16_t,160> acceptedChoicePath{};
    std::uint32_t constructState=0,constructStage=0,constructReads=0,constructCurrentChecks=0,constructOwnedChecks=0;
    std::uint32_t constructWidenResult=3,constructRestoreResult=3,constructError=0,constructBudgetHit=0;
    std::uint32_t constructStateStage=0,constructMetadataStage=0,constructSourceStage=0;
    std::uint32_t mapResetStage=0,mapResetPropertyFailure=0;std::int32_t mapResetOffset=0;
    std::uint32_t counselorHudArrival=0,pickerArrivalStage=0,pickerArrivalReads=0,pickerArrivalError=0,pickerArrivalBudgetHit=0;
    std::uint32_t pickerArrivalEntryStage=0,pickerArrivalResetStage=0;
    std::uint32_t pickerArrivalMetadataStage=0,pickerArrivalSourceStage=0,pickerArrivalRefreshStage=0;
    std::uint32_t prebirthState=0,prebirthStage=0,prebirthOwnerStage=0,prebirthEvents=0,prebirthAttempts=0,prebirthReturns=0,prebirthReads=0,prebirthError=0;
    std::uintptr_t prebirthWorld=0,prebirthPlayerState=0;
    std::uint64_t mapResetFlags=0;std::uintptr_t mapResetField=0,mapResetSavedNext=0,mapResetCurrentNext=0;
    std::array<std::uint32_t,8> pickerCallbacks{},pickerGuardPasses{},pickerGuardFailures{};
    // Last failed guard per cell is retained across later successful callbacks.
    std::array<pickerdiagnostic::Trace,8> pickerGuard{};
    std::uint32_t nativeHookState=0;std::int32_t nativeHookError=0;
    std::uint32_t nativeStartupStage=0,nativeStartupFailure=0,nativeStartupBots=0,nativeStartupReads=0;
    std::uint32_t counselorRestartAttempts=0,counselorRestartReturns=0,counselorRestartFailure=0;
    std::uint32_t challengeRestarts=0,challengeRestartFailure=0;
    std::uintptr_t nativeStartupWorld=0,nativeStartupHuman=0;
    std::uint32_t nativeIntroCalled=0,nativeMatchCalled=0,nativeCleanupAttempted=0,nativeCleanupRestored=0;
    NativeStartupDiagnostic nativeStartupDetail{};
    MenuWarmDiagnostic menuWarmDetail{};
    std::uint32_t humanJasonMenuStage=0,humanJasonMenuFailure=0,humanJasonMenuPeakReads=0,humanJasonGamePeakReads=0;
    std::uint32_t humanJasonBound=0,humanJasonRetired=0,humanJasonOptionFailure=0,humanJasonSkillState=0,humanJasonDurationState=0;
    std::uint32_t humanJasonKnivesWrites=0,humanJasonMovementWrites=0,humanJasonRechargeWrites=0;
    std::uint32_t challengePageStage=0,challengePageFailure=0,challengePageAttempts=0,challengePageSchemaMask=0,challengePagePeakReads=0;
    std::uintptr_t challengePageClass=0;
    std::uint32_t challengeRouteState=0,challengeRouteFailure=0,challengeRoutePublished=0,challengeRoutePeakReads=0;
    std::uint32_t challengeRouteOpens=0,challengeRouteBacks=0,challengeRouteStarts=0,challengeRowsState=0;
    std::uint32_t challengeStartGuard=0,challengeEventGuard=0,challengeEventRole=0,challengePageRefreshFailure=0,challengePageRefreshes=0;
    std::uint32_t challengeReacquireFailure=0,challengeRouteWin32Error=0,challengeRouteBudgetHit=0;
    std::uint32_t challengePageGuard=0,challengeReturnGuard=0,challengeNativeBackFallbacks=0;
    std::uint32_t challengeRowsWarmStage=0,challengeRowsPrepareStage=0;
    std::uint32_t challengeGameBound=0,challengeGameRetired=0,challengeGameFailure=0,challengeGamePeakReads=0;
    std::array<std::uint32_t,5> challengeGameWrites{};
    std::uint32_t challengeIntroArmed=0,challengeIntroEdges=0,challengeIntroAttempts=0,challengeIntroReturns=0;
    std::uint32_t challengeIntroFailure=0,challengeIntroWarmAttempts=0,challengeIntroWarmReturns=0,challengeIntroPeakReads=0;
    std::uint32_t challengeIntroLastRefusal=0,challengeIntroOwnerFailure=0;
    std::uint64_t challengeEscapeGeneration=0;
    std::uint32_t challengeEscapeArmed=0,challengeEscapeHookState=0,challengeEscapeErrors=0;
    std::uint32_t challengeEscapeWarm=0,challengeEscapeCensus=0,challengeEscapeVolumes=0;
    std::uint32_t challengeEscapeBlocks=0,challengeEscapeRecoveries=0,challengeEscapePeakReads=0,challengeEscapePeakCalls=0;
    std::uint32_t challengeEscapeCensusFailure=0,challengeEscapeCensusResets=0,challengeEscapeCensusSteps=0,challengeEscapeCells=0;
    std::uintptr_t challengeEscapeRejectedObject=0;
    std::uint64_t challengeBoundaryGeneration=0;
    std::uint32_t challengeBoundaryArmed=0,challengeBoundaryReady=0,challengeBoundaryCleanup=0,challengeBoundaryFailure=0;
    std::uint32_t challengeBoundaryApplied=0,challengeBoundaryRestored=0,challengeBoundaryCells=0,challengeBoundaryAttempts=0;
    std::uint32_t challengeBoundaryPeakReads=0,challengeBoundaryErrors=0,challengeBoundaryProtectionPending=0;
    std::uint32_t challengeBoundaryGuard=0,challengeBoundaryScopeFailure=0,challengeBoundaryRefreshes=0,challengeBoundaryDeferrals=0;
    std::uint32_t challengeSettingsServices=0,challengeSettingsRefusals=0,challengeRowsCurrent=0;
    std::uintptr_t challengeRowsWidget=0;
    std::array<std::uint32_t,5> challengeEscapeGateAttempts{},challengeEscapeGateBlocks{},challengeEscapeGateForwards{},challengeEscapeGateBudgets{};
    std::array<std::uint32_t,5> challengeEscapeGateStage{},challengeEscapeGateRemaining{};
    std::array<std::uint32_t,5> challengeEscapeGateCurrent{},challengeEscapeGateEligibility{};
    std::array<std::uintptr_t,5> challengeEscapeGateActor{};
    std::uint32_t challengeEscapeRecoveryStage=0,challengeEscapeRecoveryRemaining=0,challengeEscapeRecoveryReserve=0;
    std::uint32_t challengeEscapeOwnershipHits=0,challengeEscapeOwnershipMisses=0;
    std::uint32_t challengeBoundaryVerifyStage=0,challengeBoundaryVerifyRecord=0,challengeBoundaryVerifyDifference=0;
    std::uint32_t challengeBoundaryVerifyRetries=0,challengeBoundaryVerifyRecoveries=0;
    std::uint32_t introPresentationReady=0,introPresentationStage=0;
    std::uintptr_t introPresentationSkin=0;
    std::uint32_t worldRootRetirementStage=0,worldRootRefusals=0,worldRootBeforeLoadChecks=0,worldRootAfterLoadChecks=0;
    std::uint64_t failedStartWorldRecords=0,failedStartWorldRecordFailures=0,failedStartWorldRetirements=0,failedStartWorldGeneration=0;
    std::uint32_t failedStartRootsAtRecord=0;
    std::uint32_t worldRootLastObservation=0; // 0=none 1=alive 2=not-current 3=unproven (diagnostic only)
    std::uintptr_t worldRootFailureOwner=0;
    std::array<char,48> worldRootFailureName{};
    std::uint32_t rootFirstPoisonSite=0,rootFirstPoisonClaims=0,rootFirstPoisonFlags=0,rootFirstPoisonWindowStage=0,rootFirstPoisonWindowGate=0;
    std::uint32_t rootFirstPoisonContention=0,rootFirstPoisonUntouched=0;
    std::uintptr_t rootFirstPoisonPc=0;
    std::uint32_t refusalSite=0,firstRefusalSite=0,launchPreparationFailure=0;
    std::uint32_t launchSelectionPhase=0,launchSelectionMapFailure=0;
    std::array<char,64> launchSelectionProfileStage{};
    std::uint32_t jasonPathStage=0,jasonPathLength=0,jasonPathMatches=0,jasonPathRosterCount=0;
    std::array<char16_t,512> selectedJasonPath{};
    std::array<std::array<char16_t,160>,32> jasonRosterPaths{};
    std::array<unsigned,32> jasonRosterPathLengths{};
    std::uint32_t selectedJasonRoute=0;std::int32_t selectedJasonClassSerial=0;
    std::uintptr_t pickerRosterModule=0,pickerRosterWidget=0;
    std::uint32_t pickerRosterCount=0,pickerRosterStage=0;
    std::array<std::array<unsigned char,40>,64> pickerRosterPackets{};
    std::uint32_t challengePendingQueued=0,challengePendingCoalesced=0,challengePendingDropped=0,challengePendingExpired=0,challengePendingConsumed=0;
    std::uint32_t challengeInspectStage=0;
    std::uint32_t challengeRouteSub=0; // appended: 1=invoke,2=output-pre,3=hook,4=output-post,5=bad-result
    std::uint32_t challengeIntroScopeFailure=0; // J29: 1=owner/veto,2=occupied scope,3=scope reproof
    // Task005 additions ONLY at the tail: preserve all earlier report offsets.
    std::int32_t challengeEscapeLastHookError=0;
    std::uint32_t challengeEscapeArmFailures=0;
    cfgj29walls::Census challengeBoundaryWallClasses{};
    std::uint32_t challengeIntroValidateStage=0; // J30: 1..16 failing function proof, 0=none/pass
    std::uint32_t challengeIntroDispatchStage=0,challengeIntroDispatchReason=0; // J31: request handoff
    std::uint32_t challengeIntroDispatchPosted=0,challengeIntroDispatchClaimed=0,challengeIntroDispatchReturned=0,challengeIntroDispatchRefused=0;
    std::uint32_t challengeIntroNativeScopeEntered=0,challengeIntroNativeScopeRefused=0,challengeIntroNativeScopeReason=0;
    std::uint32_t challengeIntroNativeProofStage=0,challengeIntroNativeConsumerServiced=0,challengeIntroNativeConsumerDropped=0;
    cfgj36walls::Diagnostics challengeWalls{};
};
}
