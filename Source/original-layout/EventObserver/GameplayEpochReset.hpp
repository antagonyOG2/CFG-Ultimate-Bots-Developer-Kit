// Fresh native frontend epoch only. No old-world calls/frees/writes. Resident
// hooks, originals and process-pinned vtables are deliberately not reset.
bool BundleEpochRetirable()noexcept{
 return spawnRuntime.EpochIdle()&&!bundleKillEligibilityAccess&&!bundleSpectatorViewBusy&&!bundleRBDispatch&&
  !bundleObjectives.PendingRestore()&&!bundleObjectives.Status().restoreRefused&&!bundleObjectives.Status().indeterminate;
}
// Diagnostic only: which BundleEpochRetirable predicate blocks a menu epoch.
unsigned BundleEpochRetireMask()noexcept{
 return (spawnRuntime.EpochIdle()?0u:1u)|(bundleKillEligibilityAccess?2u:0u)|(bundleSpectatorViewBusy?4u:0u)|
  (bundleRBDispatch?8u:0u)|(bundleObjectives.PendingRestore()?16u:0u)|(bundleObjectives.Status().restoreRefused?32u:0u)|
  (bundleObjectives.Status().indeterminate?64u:0u);
}
void BundleResetEpoch()noexcept{
 bundleRootDiagnostics.firstPoison={}; // one FIRST-cause record per match; earlier capture/trace remains saved
 bundleRootDiagnostics.firstPoisonTracePending=false; // #150: no zeroed prior-epoch deferred trace
 BundleAccess::ResetKillEpoch();
 bundleSpectatorInput.Clear();bundleSpectatorInputPending=false;bundleSpectatorInputPendingAt=0;
 bundleSchedule.active=false;bundleSoundScope={};bundleStarted=false;
 bundlePractice={}; // weak presentation bookkeeping only; resident hook/original stay installed
 bundleContextDeferrals=bundleContextDeferralsTotal=bundleContextTraces=0;
 bundlePresentationPending=false;
 bundleConfiguration={};InterlockedExchange(&bundleConfigurationState,0);
#define CFG_EPOCH_CLEAR(value) value={}
 spawnRuntime.ResetEpoch();CFG_EPOCH_CLEAR(spawnAssets);CFG_EPOCH_CLEAR(primaryStartObservation);CFG_EPOCH_CLEAR(primaryStartBootstrap);primaryStartAt=primaryStartNext=0;
 CFG_EPOCH_CLEAR(bundleReport);CFG_EPOCH_CLEAR(bundleSchedule);CFG_EPOCH_CLEAR(bundleJason);CFG_EPOCH_CLEAR(bundleHelpers);CFG_EPOCH_CLEAR(bundleHelperBirth);
 CFG_EPOCH_CLEAR(bundleHelperResidentPass);bundleRBLastTurn=false;bundleJasonLabelLastTurn=false;
 CFG_EPOCH_CLEAR(bundleCounselorStarts);CFG_EPOCH_CLEAR(bundleCounselorStartWarm);CFG_EPOCH_CLEAR(bundleRandomCounselors);CFG_EPOCH_CLEAR(bundleCounselorLoad);
 CFG_EPOCH_CLEAR(bundleCounselorPreload);
 CFG_EPOCH_CLEAR(bundleCounselorCatalogue);CFG_EPOCH_CLEAR(bundleCounselorEntries);CFG_EPOCH_CLEAR(bundleHelperTemplate);CFG_EPOCH_CLEAR(bundleHelperPacket);bundleHelperCatalogueFailures=0;
 bundleHelperEntry=bundleHelperStart=bundleHelperLocationAttempts=bundleHelperStaleRetries=0;bundleHelperSelected=bundleF3Down=bundleHelperReinforcement=false;bundleInputAt=0;CFG_EPOCH_CLEAR(bundleF3Queue);CFG_EPOCH_CLEAR(bundleF3WindowInput);
 CFG_EPOCH_CLEAR(bundlePawnSlots);CFG_EPOCH_CLEAR(bundleControllerSlots);CFG_EPOCH_CLEAR(bundleTreeSlot);CFG_EPOCH_CLEAR(bundleManagerClassSlot);
 bundleManagerClass=bundleCounselorClass=0;bundleSelectIndex=bundleTargetIndex=0;CFG_EPOCH_CLEAR(bundleChoice);bundleMoveAt=bundleTargetLockUntil=0;
 CFG_EPOCH_CLEAR(bundlePathCooldown);CFG_EPOCH_CLEAR(bundleFollowPath);CFG_EPOCH_CLEAR(bundlePursuitProgress);CFG_EPOCH_CLEAR(bundlePursuit);CFG_EPOCH_CLEAR(bundleFootObstacle);CFG_EPOCH_CLEAR(bundleFootObstacleHistory);CFG_EPOCH_CLEAR(bundleFootObstacleCarSlot);bundleFootObstacleTarget=0;bundleFootObstacleCheckAt=0;bundleFootObstacleAlternate=false;
 CFG_EPOCH_CLEAR(bundleSummaryBots);bundleSummaryLastTurn=false;CFG_EPOCH_CLEAR(bundleStuckCars);
 bundleSummaryStackBound=false;
 CFG_EPOCH_CLEAR(bundleStatusBots);CFG_EPOCH_CLEAR(bundleStatusRows);CFG_EPOCH_CLEAR(bundleStatusSurface);bundleSummarySurfaceTurn=0;
 CFG_EPOCH_CLEAR(bundleDoorRecovery);CFG_EPOCH_CLEAR(bundleDoorBreakSelection);bundleTrapDoorAt=0;
 CFG_EPOCH_CLEAR(bundleCompletion);CFG_EPOCH_CLEAR(bundlePostMatchWatch);CFG_EPOCH_CLEAR(bundlePostMatchRecovery);bundleCompletionReady=false;CFG_EPOCH_CLEAR(bundleDeathEnd);
 bundleLateMaskRetryUsed=false;
 CFG_EPOCH_CLEAR(bundleDefeatedOutro);bundleDefeatedOutroCursor=bundleDefeatedOutroFlags=0;CFG_EPOCH_CLEAR(bundleHumanTarget);CFG_EPOCH_CLEAR(bundleNativeTarget);bundleNativeTargetHint=0;bundleNativeTargetHintIndex=0;
 CFG_EPOCH_CLEAR(bundleGrab);bundleGrabWarm=0;bundleGrabUnavailable=false;CFG_EPOCH_CLEAR(bundleBotNames);CFG_EPOCH_CLEAR(bundleEnding);CFG_EPOCH_CLEAR(bundleEndingSkin);
 CFG_EPOCH_CLEAR(bundleAdditional);
 CFG_EPOCH_CLEAR(bundleVehicleProgress);
 CFG_EPOCH_CLEAR(bundleHeldRecovery);
 CFG_EPOCH_CLEAR(bundleChaseRecovery);CFG_EPOCH_CLEAR(bundleHuntTracker);CFG_EPOCH_CLEAR(bundleHuntProgress);CFG_EPOCH_CLEAR(bundleChaseFatigue);bundleInterceptAt=0;bundleEscapeGuard.Clear();
 CFG_EPOCH_CLEAR(bundleSummary);
 CFG_EPOCH_CLEAR(bundleRageDifficulty);
 CFG_EPOCH_CLEAR(bundleSpectator);bundleSpectatorAt=0;bundleSpectatorLast=false;CFG_EPOCH_CLEAR(bundleSpectatorInstalled);CFG_EPOCH_CLEAR(bundleSpectatorList);CFG_EPOCH_CLEAR(bundleSpectatorRoster);
 CFG_EPOCH_CLEAR(bundleSpectatorPawnClasses);CFG_EPOCH_CLEAR(bundleSpectatorStateClasses);bundleSpectatorCursor=0;bundleSpectatorRosterReady=false;
 bundleSpectatorCycleEvents=0;bundleSpectatorNativeCycles=bundleSpectatorCycleRejection=0;bundleSpectatorCycleDirection=0;bundleSpectatorCycleController=0;
 CFG_EPOCH_CLEAR(bundleSpectatorCycleHook);
 CFG_EPOCH_CLEAR(bundleDeadSpectatorRecovery);bundleJasonLabelOwned=false;
 bundleSpectatorLabelPending=false;bundleSpectatorLabelAttempts=0;bundleSpectatorLabelAt=0;
 CFG_EPOCH_CLEAR(bundleSpectatorOtherFunctions);bundleSpectatorOtherCursor=0;bundleSpectatorStatusFunction=bundleSpectatorStatusWidgetHint=0;CFG_EPOCH_CLEAR(bundleSpectatorStatusName);bundleSpectatorStatusNameReady=false;bundleSpectatorStatusAt=0;
 CFG_EPOCH_CLEAR(bundleHiding);bundleHidingRefused=0;CFG_EPOCH_CLEAR(bundleEmotes);CFG_EPOCH_CLEAR(bundleRBEmote);CFG_EPOCH_CLEAR(bundleSoundBlips);
 CFG_EPOCH_CLEAR(bundleEmoteBrains);bundleEmoteMaintainAt=0;bundleEmoteMaintainCursor=bundleEmoteStops=bundleEmoteResumes=0;
 CFG_EPOCH_CLEAR(bundleTrapPriority);CFG_EPOCH_CLEAR(bundleTrapVictimSlot);CFG_EPOCH_CLEAR(bundleTriggeredTrapSlot);bundleSwimNext=0;
 CFG_EPOCH_CLEAR(bundleCarMorph);CFG_EPOCH_CLEAR(bundleCarIntercept);CFG_EPOCH_CLEAR(bundleCarPursuit);CFG_EPOCH_CLEAR(bundleRoads);CFG_EPOCH_CLEAR(bundleRoadMorph);bundleRoadAttemptAt=0;
 CFG_EPOCH_CLEAR(bundleVehicleActions);CFG_EPOCH_CLEAR(bundleCarObstacle);CFG_EPOCH_CLEAR(bundleCarSpeed);CFG_EPOCH_CLEAR(bundleCarActionPending);bundleCarSpeedRetryAt=0;bundleCarIndex=0;CFG_EPOCH_CLEAR(bundleRun);
 CFG_EPOCH_CLEAR(bundleDriverNavigation);
 CFG_EPOCH_CLEAR(bundleKnife);CFG_EPOCH_CLEAR(bundleKnifeRefresh);bundleKnifeProbePending=false;bundleKnifeProbeIndex=0;bundleKnifeProbeAt=0;CFG_EPOCH_CLEAR(bundleCabins);bundleCabinsNext=0;
 bundleKnifeHeldStops=bundleKnifeHeldRefused=bundleKnifeStopMaxReads=0;
 CFG_EPOCH_CLEAR(bundleFootBurst);CFG_EPOCH_CLEAR(bundleFootMovement);bundleFootBurstRequestAt=0;
 CFG_EPOCH_CLEAR(bundleConvergence);CFG_EPOCH_CLEAR(bundleConvergenceAxes);bundleConvergenceFearPending=bundleConvergenceFearWrites=bundleConvergenceFearRefused=0;
 CFG_EPOCH_CLEAR(bundleConvergenceTravel);bundleTravelCursor=bundleTravelAcquires=bundleTravelMoves=bundleTravelResumes=bundleTravelLost=0;bundleTravelAt=0;bundleTravelTurn=false;
 CFG_EPOCH_CLEAR(bundleFeatureTurns);CFG_EPOCH_CLEAR(bundleConvergenceNames);CFG_EPOCH_CLEAR(bundleConvergenceResources);bundleConvergencePrepared=0;CFG_EPOCH_CLEAR(bundleConvergencePublication);
 CFG_EPOCH_CLEAR(bundleConvergenceContinuation);CFG_EPOCH_CLEAR(bundleAxeContinuation);CFG_EPOCH_CLEAR(bundleBlackboardSchemas);CFG_EPOCH_CLEAR(bundleBlackboardClassWarm);bundleBlackboardClassIndex=0;bundleBlackboardClassesRefused=false;CFG_EPOCH_CLEAR(bundleConvergenceBlackboard);
 CFG_EPOCH_CLEAR(bundleSweaterPolicy);CFG_EPOCH_CLEAR(bundleSweaterOwner);CFG_EPOCH_CLEAR(bundleSweaterClass);cfgh241mask::memo.Clear();cfgh241sweater::memo.Clear();bundleSweaterClassAttempted=bundleSweaterClassPending=bundleSweaterFreshPending=bundleSweaterFreshRequested=false;
 bundleSweaterFreshUse=0;bundleSweaterFreshGrantAttempted=false;CFG_EPOCH_CLEAR(bundleSweaterPickup);CFG_EPOCH_CLEAR(bundleSweaterAbility);CFG_EPOCH_CLEAR(bundleSweaterSource);CFG_EPOCH_CLEAR(bundleSweaterRetiredAbility);bundleSweaterCursor=bundleSweaterSettingsCursor=bundleSweaterWarm=0;bundleSweaterSourceIndex=~0u;
 CFG_EPOCH_CLEAR(bundleDuration);CFG_EPOCH_CLEAR(bundleSkill);CFG_EPOCH_CLEAR(bundleLoadout);CFG_EPOCH_CLEAR(bundleExpandedLoadout);CFG_EPOCH_CLEAR(bundleLoadoutResource);
 CFG_EPOCH_CLEAR(bundleTraps);CFG_EPOCH_CLEAR(bundleTrapSources);CFG_EPOCH_CLEAR(bundleTrapInputs);bundleTrapFamily=cfgtrap::MapFamily::Unknown;bundleTrapMapWorld=0;CFG_EPOCH_CLEAR(bundleTrapMapPackage);
 CFG_EPOCH_CLEAR(bundleObjectives);bundleObjectiveRetiring=false;bundleObjectiveNext=bundleObjectiveCheck=0;bundleObjectiveModule=bundleObjectiveWorld=bundleObjectiveViewport=0;bundleObjectiveThread=0;
 CFG_EPOCH_CLEAR(bundleObjectiveRepair);CFG_EPOCH_CLEAR(bundleObjectivePanelBinding);CFG_EPOCH_CLEAR(bundleObjectiveWidget);bundleObjectiveCandidate=bundleObjectiveRepairFailure=0;bundleObjectiveRepairTurn=false;
 bundleObjectiveStackAt=0;CFG_EPOCH_CLEAR(bundleObjectiveStackLast);bundleObjectiveStackTop=0;bundleObjectiveStackVisits=0;
 CFG_EPOCH_CLEAR(bundleObjectiveCarCache);CFG_EPOCH_CLEAR(bundleObjectiveCarPublication);CFG_EPOCH_CLEAR(bundleObjectiveCarProof);CFG_EPOCH_CLEAR(bundleRadioObjective);bundleRadioObjectiveSince=bundleRadioObjectiveNext=0;
 bundleRadioObjectiveBegun=bundleRadioObjectiveAttempted=bundleRadioObjectiveFinished=false;
 CFG_EPOCH_CLEAR(bundleKill);CFG_EPOCH_CLEAR(bundleKillDiagnostics);CFG_EPOCH_CLEAR(bundleDamageGate);bundleKillStanceFunction=0;CFG_EPOCH_CLEAR(bundleKillStanceSlot);bundleKillHookComponent=bundleKillClone=0;
 CFG_EPOCH_CLEAR(bundleKillActorSlot);CFG_EPOCH_CLEAR(bundleKillComponentSlot);CFG_EPOCH_CLEAR(bundleKillInteractSlot);bundleKillAt=0;CFG_EPOCH_CLEAR(bundleKillTicket);CFG_EPOCH_CLEAR(bundleKillWeaponClass);bundleKillWeaponKind=cfgkill::Weapon::Other;
#undef CFG_EPOCH_CLEAR
}
