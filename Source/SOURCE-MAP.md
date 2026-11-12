# Source map


## Original-layout folders

| Folder | Files | Contents |
|---|---|---|
| `Core/` | 195 | Pure policy headers (engine-independent decision logic, feature flags, report struct). Most transferable. |
| `EventObserver/` | 932 | In-game runtime: bundle runtime, hosted runtime, F1 birth, frontend/menus, GC retention, injector (IntegrationHarness.cpp) and about 490 fixture tests. |
| `RuntimeProbe/` | 232 | Address/binding pins for CFG 3.5 (*.generated.hpp), object-lifetime types, observation probes and their tests. Delete in a native port. |
| `Release/` | 6 | Player launcher and installer scripts. |

## Subsystem index

### JasonAI  (`Source/JasonAI/README.md`)

- `EventObserver/NativeJasonControllerTickRuntime.hpp`
- `Core/DonorJasonControllerTick.hpp`
- `Core/DonorJasonChase.hpp`
- `Core/DonorJasonTargeting.hpp`
- `EventObserver/DonorJasonDecisionFrame.hpp`
- `Core/ChaseLead.hpp`
- `Core/DonorJasonPursuit.hpp`
- `Core/DonorJasonMorph.hpp`
- `Core/DonorNativeMorphCompletion.hpp`
- `Core/DonorJasonMovement.hpp`
- `Core/DonorJasonCombat.hpp`
- `Core/DonorOwedAttackRelease.hpp`
- `Core/DonorJasonRoster.hpp`
- `Core/DonorJasonStartup.hpp`
- `Core/DonorJasonBusyCause.hpp`
- `Core/DonorNavigationYield.hpp`
- `Core/J22TargetPolicy.hpp`
- `Core/JasonHuntPolicy.hpp`
- `Core/JasonRunPolicy.hpp`
- `Core/JasonApproachPolicy.hpp`
- `Core/JasonRageDifficultyPolicy.hpp`
- `Core/PursuitSpeedPolicy.hpp`
- `Core/JasonEscapeGuardPolicy.hpp`
- `EventObserver/DonorJasonHuntingService.hpp`
- `EventObserver/DonorJasonHuntingHostMembers.hpp`
- `EventObserver/DonorJasonHuntingNativeMembers.hpp`
- `EventObserver/DonorJasonRuntimeState.hpp`
- `EventObserver/DonorJasonActionRuntimeMembers.hpp`
- `EventObserver/DonorJasonMovementNativeMembers.hpp`

### CounselorAI  (`Source/CounselorAI/README.md`)

- `Core/DonorCounselorConvergence.hpp`
- `Core/CounselorConvergencePolicy.hpp`
- `Core/ConvergenceTravelPolicy.hpp`
- `Core/ConvergencePublication.hpp`
- `Core/ConvergenceArrivalReport.hpp`
- `Core/LeanConvergencePolicy.hpp`
- `Core/SweaterLifecyclePolicy.hpp`
- `Core/SweaterOwnerRefreshPolicy.hpp`
- `Core/SweaterPublication.hpp`
- `Core/SweaterRetention.hpp`
- `Core/H248SweaterTarget.hpp`
- `Core/CounselorPursuitPolicy.hpp`
- `Core/CounselorPracticePolicy.hpp`
- `Core/CounselorPracticeSettings.hpp`
- `EventObserver/DonorCounselorObservation.hpp`
- `EventObserver/DonorCounselorContextObservation.hpp`
- `Core/EmoteSyncPolicy.hpp`
- `Core/NativeEmoteMirrorQueue.hpp`
- `Core/NativeEmoteCompletionPolicy.hpp`
- `EventObserver/DonorCounselorEmotes.hpp`
- `EventObserver/NativeEmoteMirror.hpp`
- `EventObserver/EmoteBrainRuntimeMembers.hpp`
- `EventObserver/RBEmoteRuntime.hpp`
- `Core/RandomCounselorPolicy.hpp`
- `Core/CounselorLoadPolicy.hpp`
- `Core/CounselorStatsPolicy.hpp`
- `Core/BotNamePolicy.hpp`
- `EventObserver/DonorCounselorConvergenceNativeMembers.hpp`
- `EventObserver/ConvergenceAxeRuntime.hpp`
- `EventObserver/ConvergenceTravelRuntimeMembers.hpp`
- `EventObserver/LeanConvergenceNativeMembers.hpp`
- `EventObserver/SweaterBindRuntimeMembers.hpp`
- `EventObserver/SweaterClassCaptureRuntimeMembers.hpp`
- `EventObserver/SweaterFreshRuntimeMembers.hpp`
- `EventObserver/SweaterHudNativeRuntimeMembers.hpp`
- `EventObserver/SweaterKillArmRuntimeMembers.hpp`
- `EventObserver/SweaterOwnerRuntimeMembers.hpp`
- `EventObserver/SweaterPhaseRuntimeMembers.hpp`
- `EventObserver/SweaterRuntimeMembers.hpp`

### Spawning  (`Source/Spawning/README.md`)

- `Core/AdditionalJasonPolicy.hpp`
- `EventObserver/AdditionalJasonBirthMembers.hpp`
- `EventObserver/HostedJasonMembers.hpp`
- `EventObserver/AdditionalJasonSelectionMembers.hpp`
- `EventObserver/AdditionalJasonLifecycleMembers.hpp`
- `EventObserver/AdditionalJasonRegistryMembers.hpp`
- `EventObserver/AdditionalJasonRuntimeMembers.hpp`
- `Core/AdditionalJasonJournal.hpp`
- `Core/DonorAdditionalJasonHunt.hpp`
- `Core/NativeSpawnParameters.hpp`
- `Core/NativeDeferredSpawnParameters.hpp`
- `Core/OneCounselorSpawnPolicy.hpp`
- `Core/HelperSpawnPolicy.hpp`
- `Core/UnifiedAISpawnCommands.hpp`
- `Core/J24SpawnRefusal.hpp`
- `Core/KillerStartBootstrap.hpp`
- `EventObserver/SpawnRuntime.hpp`
- `EventObserver/SpawnBindings.hpp`
- `EventObserver/HelperBirthRuntimeMembers.hpp`
- `EventObserver/NativeOfflineCounselorBirth.hpp`
- `EventObserver/NativePreparedCounselorConstruct.hpp`
- `EventObserver/PrimarySpawnObservation.hpp`
- `EventObserver/OfflineCounselorSpawn.cpp`

### Navigation  (`Source/Navigation/README.md`)

- `Core/DonorJasonChase.hpp`
- `Core/DonorJasonNavContinuation.hpp`
- `Core/ChaseLead.hpp`
- `Core/DonorUnstick.hpp`
- `Core/ChaseRecoveryPolicy.hpp`
- `Core/DriverNavigationPolicy.hpp`
- `Core/DonorMoveRetentionPolicy.hpp`
- `Core/DonorNavigationYield.hpp`
- `Core/DoorRecoveryPolicy.hpp`
- `Core/CabinBoundsPolicy.hpp`
- `EventObserver/CabinBoundsRuntime.hpp`
- `Core/H246BurstPolicy.hpp`
- `EventObserver/ChaseRecoveryRuntimeMembers.hpp`
- `EventObserver/DriverNavigationRuntimeMembers.hpp`

### Combat  (`Source/Combat/README.md`)

- `Core/OnFootBurstPolicy.hpp`
- `Core/H246BurstPolicy.hpp`
- `Core/DonorHeldGrabKill.hpp`
- `Core/J23GrabTiming.hpp`
- `Core/GrabCombatPolicy.hpp`
- `Core/HeldRecoveryPolicy.hpp`
- `Core/KillPolicy.hpp`
- `Core/H247CombatPolicy.hpp`
- `Core/JasonDeathCompletion.hpp`
- `EventObserver/DonorHeldGrabKillNativeMembers.hpp`
- `EventObserver/HeldRecoveryRuntimeMembers.hpp`
- `EventObserver/GrabRuntimeMembers.hpp`
- `Core/KnifeAimGeometry.hpp`
- `EventObserver/KnifeRuntime.hpp`
- `EventObserver/KnifeRuntimeMembers.hpp`
- `EventObserver/DonorKnifeContinuationMembers.hpp`
- `EventObserver/KnifeNotifyCompletion.hpp`
- `Core/ObjectiveTrapPolicy.hpp`
- `Core/TrapPriorityPolicy.hpp`
- `Core/ShackTrapPolicy.hpp`
- `Core/DonorTrapPriorityBridge.hpp`
- `Core/StartupTrapRuntime.hpp`
- `Core/StartupTrapMapScope.hpp`
- `EventObserver/NativeStartupTraps.hpp`
- `EventObserver/TrapEventObservationMembers.hpp`
- `EventObserver/StartupTrapIntegration.txt`
- `Core/DoorRecoveryPolicy.hpp`
- `EventObserver/DoorReadOnlySnapshot.cpp`
- `EventObserver/SelectedDoorObservation.hpp`
- `Core/HidingInteractionPolicy.hpp`
- `Core/DonorHidingTargetPolicy.hpp`
- `EventObserver/HidingRuntimeMembers.hpp`
- `Core/VehicleActionPolicy.hpp`
- `Core/VehicleContinuationPolicy.hpp`
- `Core/VehicleExitBlockPolicy.hpp`
- `Core/VehicleProgressPolicy.hpp`
- `Core/VehiclePursuitPolicy.hpp`
- `Core/VehicleRoadChasePolicy.hpp`
- `Core/VehicleRoadInterceptPolicy.hpp`
- `Core/VehicleRoadPathTrace.hpp`
- `Core/StuckOccupiedCarPolicy.hpp`
- `Core/OnFootCarObstaclePolicy.hpp`
- `EventObserver/VehicleActionRuntimeMembers.hpp`
- `EventObserver/VehicleObstacleRuntimeMembers.hpp`
- `EventObserver/VehicleProgressRuntimeMembers.hpp`
- `EventObserver/VehicleRoadRuntimeMembers.hpp`
- `EventObserver/VehicleStopRuntimeMembers.hpp`
- `EventObserver/CarPendingActionRuntimeMembers.hpp`

### Gameplay  (`Source/Gameplay/README.md`)

- `EventObserver/GameplayBundleRuntime.hpp`
- `Core/GameplayBundlePolicy.hpp`
- `Core/J22FeatureFlags.hpp`
- `Core/H20FeatureFlags.hpp`
- `Core/J214FeatureFlags.hpp`
- `Core/J215FeatureFlags.hpp`
- `Core/J21cFeatureFlags.hpp`
- `Core/J24FeatureFlags.hpp`
- `Core/GameplayConfiguration.hpp`
- `Core/GameplayLimits.hpp`
- `Core/GameplayReportCapture.hpp`
- `EventObserver/DifficultyPolicy.hpp`
- `EventObserver/DifficultyRuntimeMembers.hpp`
- `Core/JasonRageDifficultyPolicy.hpp`
- `Core/J21FrameBudget.hpp`
- `Core/J21WorkPolicy.hpp`
- `Core/OptionalWorkAdmission.hpp`
- `EventObserver/J21Timing.hpp`
- `Core/SpectatorInputIntent.hpp`
- `Core/SpectatorRefusalPolicy.hpp`
- `Core/DeadSpectatorRecoveryPolicy.hpp`
- `EventObserver/SpectatorRuntimeMembers.hpp`
- `Core/EndingMetadataPolicy.hpp`
- `Core/EndingSummaryPolicy.hpp`
- `Core/SummaryPresentationPolicy.hpp`

### Menus  (`Source/Menus/README.md`)

- `EventObserver/FrontendBundleRuntime.hpp`
- `Core/ModeSettingsPolicy.hpp`
- `EventObserver/NativeSetupRows.hpp`
- `EventObserver/NativeSettingsMenuOptions.hpp`
- `EventObserver/NativeMenuRowAccess.hpp`
- `EventObserver/NativeMenuDispatch.hpp`
- `EventObserver/NativeMenuLaunchRuntime.hpp`
- `EventObserver/HostedSettingsSpecs.hpp`
- `Core/HostedOptions.hpp`
- `EventObserver/HostedLobbyRowsRuntime.hpp`
- `EventObserver/HostedLobbyRuntime.hpp`
- `EventObserver/HostedLobbyLayout.hpp`
- `EventObserver/HostedLobbyPlacement.hpp`
- `Core/MenuCompanionPolicy.hpp`
- `Core/MenuReentryPolicy.hpp`
- `Core/MenuPreparationSchedule.hpp`
- `Core/SetupMenuModel.hpp`
- `Core/OfflineSettingsSelection.hpp`
- `Core/RestrictedInventoryMenu.hpp`
- `EventObserver/FrontendPickerRuntime.hpp`
- `EventObserver/FrontendSetupRuntime.hpp`
- `EventObserver/FrontendEpochRuntime.hpp`
- `EventObserver/FrontendRetentionRuntime.hpp`
- `EventObserver/NativeHumanJasonMenu.hpp`
- `EventObserver/NativeHumanJasonOptionsRuntime.hpp`
- `EventObserver/NativeChallengeSettingsSpecs.hpp`

### MatchManagement  (`Source/MatchManagement/README.md`)

- `EventObserver/GameplayBundleRuntime.hpp`
- `EventObserver/GameplayEpochReset.hpp`
- `EventObserver/HostedH1Runtime.hpp`
- `Core/HostedOfflineEpoch.hpp`
- `EventObserver/HostedRoutineRuntime.hpp`
- `EventObserver/WorldRootRetirementRuntime.hpp`
- `Core/WorldRootRetirement.hpp`
- `Core/NativeOfflineLifecycle.hpp`
- `Core/FrontendEpochService.hpp`
- `Core/FrontendMapArrivalPolicy.hpp`
- `Core/FrontendArrivalWindow.hpp`
- `EventObserver/NativeOfflineLifecycleHooks.hpp`
- `EventObserver/NativeFrontendEpochSource.hpp`
- `EventObserver/PostMatchRecoveryObservation.hpp`
- `Core/HostedEndFreezePolicy.hpp`
- `Core/HostedClassRestorePolicy.hpp`
- `Core/JasonDeathCompletion.hpp`

### Hosted  (`Source/Hosted/README.md`)

- `EventObserver/HostedH1Runtime.hpp`
- `EventObserver/HostedAdoption.hpp`
- `EventObserver/HostedJasonTakeover.hpp`
- `EventObserver/HostedJasonState.hpp`
- `EventObserver/HostedJasonMembers.hpp`
- `EventObserver/HostedRoutineRuntime.hpp`
- `EventObserver/HostedRoutineRuntimeMembers.hpp`
- `EventObserver/HostedCounselorMembers.hpp`
- `EventObserver/HostedCounselorBirthBrainMembers.hpp`
- `EventObserver/HostedCounselorBrainMembers.hpp`
- `EventObserver/HostedFillRuntime.hpp`
- `EventObserver/HostedDeferredFill.hpp`
- `EventObserver/HostedLobbyDiscovery.hpp`
- `EventObserver/HostedLobbyInputEvidence.hpp`
- `EventObserver/HostedLobbyLayout.hpp`
- `EventObserver/HostedLobbyObservation.hpp`
- `EventObserver/HostedLobbyOutputSlotObservation.hpp`
- `EventObserver/HostedLobbyPlacement.hpp`
- `EventObserver/HostedLobbyReport.hpp`
- `EventObserver/HostedLobbyRetainedRuntime.hpp`
- `EventObserver/HostedLobbyRowsRuntime.hpp`
- `EventObserver/HostedLobbyRuntime.hpp`
- `EventObserver/HostedLobbyStaticInputs.hpp`
- `EventObserver/HostedSettingsSpecs.hpp`
- `Core/HostedOnlinePolicy.hpp`
- `Core/HostedOptions.hpp`
- `Core/HostedFillPolicy.hpp`
- `Core/HostedRolePreference.hpp`
- `Core/HostedCounselorBirth.hpp`
- `Core/HostedCounselorCandidatePolicy.hpp`
- `Core/HostedKillerPresentation.hpp`
- `Core/HostedMovementFallbackPolicy.hpp`
- `Core/HostedMaskTriggerMemo.hpp`
- `Core/HostedMaskTriggerPolicy.hpp`
- `EventObserver/HostedMaskTriggerMembers.hpp`
- `Core/HostedSpectatorHintPolicy.hpp`
- `Core/HostedObservationBudget.hpp`
- `Core/DonorAdditionalJasonHunt.hpp`
- `Core/J22FeatureFlags.hpp`
- `RuntimeProbe/HostedAuthority35.generated.hpp` *(withheld from the public kit)*
- `RuntimeProbe/HostedBirth35.generated.hpp` *(withheld from the public kit)*
- `RuntimeProbe/HostedFill35.generated.hpp` *(withheld from the public kit)*
- `RuntimeProbe/HostedLobby35.generated.hpp` *(withheld from the public kit)*
- `RuntimeProbe/HostLocalView35.generated.hpp` *(withheld from the public kit)*

### Challenges  (`Source/Challenges/README.md`)

- `Core/ChallengeEdgeWallsPolicy.hpp`
- `Core/ChallengeEscapeBoundaryPolicy.hpp`
- `Core/ChallengeEscapeStartPolicy.hpp`
- `Core/ChallengeBoundaryWallCensusPolicy.hpp`
- `Core/ChallengeNativeScopePolicy.hpp`
- `Core/ChallengeStrongChildrenPolicy.hpp`
- `Core/ChallengeIntroDispatchPolicy.hpp`
- `Core/ChallengeIntroSkipPolicy.hpp`
- `EventObserver/NativeChallengeBoundaryFrontend.hpp`
- `EventObserver/NativeChallengeBoundaryRuntime.hpp`
- `EventObserver/NativeChallengeBoundaryScript.hpp`
- `EventObserver/NativeChallengeBoundaryWriter.hpp`
- `EventObserver/NativeChallengeEdgeWallsFrontend.hpp`
- `EventObserver/NativeChallengeEdgeWallsRuntime.hpp`
- `EventObserver/NativeChallengeEscapeAccess.hpp`
- `EventObserver/NativeChallengeEscapeActors.hpp`
- `EventObserver/NativeChallengeEscapeDispatch.hpp`
- `EventObserver/NativeChallengeEscapeFrontend.hpp`
- `EventObserver/NativeChallengeEscapeGeometry.hpp`
- `EventObserver/NativeChallengeEscapeHooks.hpp`
- `EventObserver/NativeChallengeEscapeResources.hpp`
- `EventObserver/NativeChallengeEscapeRuntime.hpp`
- `EventObserver/NativeChallengeEscapeScope.hpp`
- `EventObserver/NativeChallengeEscapeSeams.hpp`
- `EventObserver/NativeChallengeSettingsService.hpp`
- `EventObserver/NativeChallengeOptions.hpp`
- `RuntimeProbe/ChallengeEdgeWallsBindings.generated.hpp` *(withheld from the public kit)*
- `RuntimeProbe/ChallengeEscapeRecovery.generated.hpp` *(withheld from the public kit)*
- `RuntimeProbe/ChallengeIntro.generated.hpp` *(withheld from the public kit)*

### Retention  (`Source/Retention/README.md`)

- `EventObserver/CallbackRootLease.hpp`
- `EventObserver/ResidentRootBank.hpp`
- `EventObserver/PermanentRootPool.hpp`
- `EventObserver/RootSlotLedger.hpp`
- `EventObserver/SynchronizedRootOwner.hpp`
- `EventObserver/ResidentRootRegistration.hpp`
- `EventObserver/RootPoisonDiagnostic.hpp`
- `EventObserver/RootPublicationProbe.hpp`
- `EventObserver/BundleCallbackRootsRuntime.hpp`
- `EventObserver/BundleRetentionRuntimeMembers.hpp`
- `EventObserver/SoftClassRetentionRuntimeMembers.hpp`
- `EventObserver/PrimaryJasonRetentionRuntime.hpp`
- `EventObserver/GcReferenceOwner.hpp`
- `EventObserver/GcRegistryProbe.hpp`
- `EventObserver/GcRegistryProbeRuntime.hpp`
- `EventObserver/GcOwnerDiagnosticRegistry.hpp`
- `RuntimeProbe/ObjectLifetime.hpp`
- `Core/WorldRootRetirement.hpp`
- `EventObserver/WorldRootRetirementRuntime.hpp`
- `EventObserver/WorldRootInventory.hpp`

### Loader  (`Source/Loader/README.md`)

- `EventObserver/IntegrationHarness.cpp` *(withheld from the public kit)*
- `Core/AutomaticInstallPolicy.hpp`
- `EventObserver/Write-EmoteTraceBuildPin.ps1`
- `EventObserver/Write-JasonApproachBuildPin.ps1`
- `EventObserver/Write-ObserverBuildPin.ps1`
- `EventObserver/Write-ReleasePortBuildPin.ps1`
- `EventObserver/Write-SpawnBuildPin.ps1`
- `EventObserver/Write-StatsBuildPin.ps1`
- `EventObserver/RBEmoteBindings.generated.hpp` *(withheld from the public kit)*
- `EventObserver/UrinationBindings.generated.hpp` *(withheld from the public kit)*
- `RuntimeProbe/ActorRotationBindings.generated.hpp` *(withheld from the public kit)*
- `RuntimeProbe/ActorTeleportBindings.generated.hpp` *(withheld from the public kit)*
- `RuntimeProbe/AdditionalJasonBindings.generated.hpp` *(withheld from the public kit)*
- `RuntimeProbe/AdditionalLifecycleBindings.generated.hpp` *(withheld from the public kit)*
- `RuntimeProbe/AiActionBindings.generated.hpp` *(withheld from the public kit)*
- `RuntimeProbe/ArrayReflection.generated.hpp` *(withheld from the public kit)*
- `RuntimeProbe/BlackboardSchemaBindings.generated.hpp` *(withheld from the public kit)*
- `RuntimeProbe/BotNameBindings.generated.hpp` *(withheld from the public kit)*
- `RuntimeProbe/CabinBoundsBindings.generated.hpp` *(withheld from the public kit)*
- `RuntimeProbe/CameraBindings.generated.hpp` *(withheld from the public kit)*
- `RuntimeProbe/ChallengeEdgeWallsBindings.generated.hpp` *(withheld from the public kit)*
- `RuntimeProbe/ChallengeEscapeRecovery.generated.hpp` *(withheld from the public kit)*
- `RuntimeProbe/ChallengeGeometryInputs.generated.hpp` *(withheld from the public kit)*
- `RuntimeProbe/ChallengeIntro.generated.hpp` *(withheld from the public kit)*
- `RuntimeProbe/ChallengeMenuCalls.generated.hpp` *(withheld from the public kit)*
- `RuntimeProbe/ChallengeWallOwnershipBindings.generated.hpp` *(withheld from the public kit)*
- `RuntimeProbe/CollectorSubmitBindings.generated.hpp` *(withheld from the public kit)*
- `RuntimeProbe/ConstructBoundary.generated.hpp` *(withheld from the public kit)*
- `RuntimeProbe/ConvergenceBindings.generated.hpp` *(withheld from the public kit)*
- `RuntimeProbe/ConvergenceTravelBindings.generated.hpp` *(withheld from the public kit)*
- `RuntimeProbe/CounselorBirthBindings.generated.hpp` *(withheld from the public kit)*
- `RuntimeProbe/CounselorLoadBindings.generated.hpp` *(withheld from the public kit)*
- `RuntimeProbe/CounselorPickerCast.generated.hpp` *(withheld from the public kit)*
- `RuntimeProbe/CounselorPracticeBindings.generated.hpp` *(withheld from the public kit)*
- `RuntimeProbe/CounselorStartBindings.generated.hpp` *(withheld from the public kit)*
- `RuntimeProbe/CrowdFallbackBindings.generated.hpp` *(withheld from the public kit)*
- `RuntimeProbe/DifficultyBindings.generated.hpp` *(withheld from the public kit)*
- `RuntimeProbe/DonorHeldBindings.generated.hpp` *(withheld from the public kit)*
- `RuntimeProbe/DoorBindings.generated.hpp` *(withheld from the public kit)*
- `RuntimeProbe/DoorBreakBindings.generated.hpp` *(withheld from the public kit)*
- `RuntimeProbe/EmoteMirrorBindings.generated.hpp` *(withheld from the public kit)*
- `RuntimeProbe/EndingMetadataBindings.generated.hpp` *(withheld from the public kit)*
- `RuntimeProbe/EscapeBindings.generated.hpp` *(withheld from the public kit)*
- `RuntimeProbe/FeatureSnapshotBindings.generated.hpp` *(withheld from the public kit)*
- `RuntimeProbe/FreshAbilityBindings.generated.hpp` *(withheld from the public kit)*
- `RuntimeProbe/FrontendCatalogues.generated.hpp` *(withheld from the public kit)*
- `RuntimeProbe/FrontendTransport.generated.hpp` *(withheld from the public kit)*
- `RuntimeProbe/GcRetentionBindings.generated.hpp` *(withheld from the public kit)*
- `RuntimeProbe/GrabBindings.generated.hpp` *(withheld from the public kit)*
- `RuntimeProbe/HostedAuthority35.generated.hpp` *(withheld from the public kit)*
- `RuntimeProbe/HostedBirth35.generated.hpp` *(withheld from the public kit)*
- `RuntimeProbe/HostedCounselorBrainBindings.generated.hpp` *(withheld from the public kit)*
- `RuntimeProbe/HostedFill35.generated.hpp` *(withheld from the public kit)*
- `RuntimeProbe/HostedLobby35.generated.hpp` *(withheld from the public kit)*
- `RuntimeProbe/HostedSpectator35.generated.hpp` *(withheld from the public kit)*
- `RuntimeProbe/HostLocalView35.generated.hpp` *(withheld from the public kit)*
- `RuntimeProbe/HunterBindings.generated.hpp` *(withheld from the public kit)*
- `RuntimeProbe/InventoryBindings.generated.hpp` *(withheld from the public kit)*
- `RuntimeProbe/JasonApproachBindings.generated.hpp` *(withheld from the public kit)*
- `RuntimeProbe/KillBindings.generated.hpp` *(withheld from the public kit)*
- `RuntimeProbe/KillContactBindings.generated.hpp` *(withheld from the public kit)*
- `RuntimeProbe/KillerStartBindings.generated.hpp` *(withheld from the public kit)*
- `RuntimeProbe/KnifeSequenceBindings.generated.hpp` *(withheld from the public kit)*
- `RuntimeProbe/LoadedCounselorBindings.generated.hpp` *(withheld from the public kit)*
- `RuntimeProbe/MenuCopy.generated.hpp` *(withheld from the public kit)*
- `RuntimeProbe/MenuExecution.generated.hpp` *(withheld from the public kit)*
- `RuntimeProbe/MenuNativeCaches.generated.hpp` *(withheld from the public kit)*
- `RuntimeProbe/MenuPush.generated.hpp` *(withheld from the public kit)*
- `RuntimeProbe/MenuStackParent.generated.hpp` *(withheld from the public kit)*
- `RuntimeProbe/MenuTransition.generated.hpp` *(withheld from the public kit)*
- `RuntimeProbe/NameBindings.generated.hpp` *(withheld from the public kit)*
- `RuntimeProbe/NativeCounselorTransition.generated.hpp` *(withheld from the public kit)*
- `RuntimeProbe/NativeJasonBindings.generated.hpp` *(withheld from the public kit)*
- `RuntimeProbe/NativePickerSource.generated.hpp` *(withheld from the public kit)*
- `RuntimeProbe/NativePopulationBindings.generated.hpp` *(withheld from the public kit)*
- `RuntimeProbe/NativeSoftClassLoad.generated.hpp` *(withheld from the public kit)*
- `RuntimeProbe/OfflineIntroMetadataBindings.generated.hpp` *(withheld from the public kit)*
- `RuntimeProbe/OfflineLifecycleBindings.generated.hpp` *(withheld from the public kit)*
- `RuntimeProbe/OfflineRecoveryBindings.generated.hpp` *(withheld from the public kit)*
- `RuntimeProbe/OfflineSettingsBindings.generated.hpp` *(withheld from the public kit)*
- `RuntimeProbe/PositionBindings.generated.hpp` *(withheld from the public kit)*
- `RuntimeProbe/PostMatchRecoveryBindings.generated.hpp` *(withheld from the public kit)*
- `RuntimeProbe/PrimaryJasonBindings.generated.hpp` *(withheld from the public kit)*
- `RuntimeProbe/RadarObjectiveBindings.generated.hpp` *(withheld from the public kit)*
- `RuntimeProbe/RadarWorldBindings.generated.hpp` *(withheld from the public kit)*
- `RuntimeProbe/ReflectionLayout.generated.hpp` *(withheld from the public kit)*
- `RuntimeProbe/RestrictedLoadoutBindings.generated.hpp` *(withheld from the public kit)*
- `RuntimeProbe/SandboxGateBindings.generated.hpp` *(withheld from the public kit)*
- `RuntimeProbe/ScriptLayout.generated.hpp` *(withheld from the public kit)*
- `RuntimeProbe/SettingsStart.generated.hpp` *(withheld from the public kit)*
- `RuntimeProbe/SetupSelections.generated.hpp` *(withheld from the public kit)*
- `RuntimeProbe/SoftClassMarshaling.generated.hpp` *(withheld from the public kit)*
- `RuntimeProbe/SoundBlipBindings.generated.hpp` *(withheld from the public kit)*
- `RuntimeProbe/SpawnBindings.generated.hpp` *(withheld from the public kit)*
- `RuntimeProbe/SpawnRuntimeBindings.generated.hpp` *(withheld from the public kit)*
- `RuntimeProbe/SpectatorNativeBindings.generated.hpp` *(withheld from the public kit)*
- `RuntimeProbe/StartingLoadoutBindings.generated.hpp` *(withheld from the public kit)*
- `RuntimeProbe/StartingPlayerBindings.generated.hpp` *(withheld from the public kit)*
- `RuntimeProbe/StartupEntryBindings.generated.hpp` *(withheld from the public kit)*
- `RuntimeProbe/StartupTrapBindings.generated.hpp` *(withheld from the public kit)*
- `RuntimeProbe/SummaryBindings.generated.hpp` *(withheld from the public kit)*
- `RuntimeProbe/SummaryFilterBindings.generated.hpp` *(withheld from the public kit)*
- `RuntimeProbe/SweaterBindings.generated.hpp` *(withheld from the public kit)*
- `RuntimeProbe/SweaterHudBindings.generated.hpp` *(withheld from the public kit)*
- `RuntimeProbe/SweaterPickupBindings.generated.hpp` *(withheld from the public kit)*
- `RuntimeProbe/SweaterRetentionBindings.generated.hpp` *(withheld from the public kit)*
- `RuntimeProbe/TickCallbackBindings.generated.hpp` *(withheld from the public kit)*
- `RuntimeProbe/TommyEndRaceBindings.generated.hpp` *(withheld from the public kit)*
- `RuntimeProbe/TrapPriorityBindings.generated.hpp` *(withheld from the public kit)*
- `RuntimeProbe/VehicleBindings.generated.hpp` *(withheld from the public kit)*
- `RuntimeProbe/VehicleInteractionBindings.generated.hpp` *(withheld from the public kit)*
- `RuntimeProbe/VehicleOccupancyBindings.generated.hpp` *(withheld from the public kit)*
- `RuntimeProbe/VehicleRoadBindings.generated.hpp` *(withheld from the public kit)*
- `RuntimeProbe/WorldMapBindings.generated.hpp` *(withheld from the public kit)*
- `RuntimeProbe/Compatibility/CfgAdditional35.generated.hpp` *(withheld from the public kit)*
- `RuntimeProbe/Compatibility/CfgCameraDiagnostic35.generated.hpp` *(withheld from the public kit)*
- `RuntimeProbe/Compatibility/CfgEnding35.generated.hpp` *(withheld from the public kit)*
- `RuntimeProbe/Compatibility/CfgJasonHunting35.generated.hpp` *(withheld from the public kit)*
- `RuntimeProbe/Compatibility/CfgJasonMovement35.generated.hpp` *(withheld from the public kit)*
- `RuntimeProbe/Compatibility/CfgPoliceExit35.generated.hpp` *(withheld from the public kit)*
- `RuntimeProbe/Compatibility/CfgRBConfirmation35.generated.hpp` *(withheld from the public kit)*
- `RuntimeProbe/Compatibility/CfgSummary35.generated.hpp` *(withheld from the public kit)*
- `RuntimeProbe/Compatibility/CfgSweaterHud35.generated.hpp` *(withheld from the public kit)*
- `RuntimeProbe/Compatibility/CfgSweaterPickup35.generated.hpp` *(withheld from the public kit)*
- `RuntimeProbe/Compatibility/CfgSweaterWorldSettings35.generated.hpp` *(withheld from the public kit)*
- `RuntimeProbe/Compatibility/CfgTrapPriority35.generated.hpp` *(withheld from the public kit)*
- `RuntimeProbe/Compatibility/CfgUrination35.generated.hpp` *(withheld from the public kit)*
- `Release/Launcher.cpp` *(withheld from the public kit)*
- `Release/Install.ps1`
- `Release/Uninstall.ps1`
- `Release/INSTALL.bat`
- `Release/UNINSTALL.bat`
- `Release/README.txt`
- `EventObserver/OfflineFrontendBundle.cpp`
- `EventObserver/OfflineEventObserver.cpp`
- `EventObserver/GameplayBundleRuntime.hpp`
- `EventObserver/FrontendBundleRuntime.hpp`



## Not included

- `SourceBaseline/` (the original donor project): not published here pending the author's decision; many comments cite its line numbers (for example `FrozenJasonBridge.cpp`, `FEAT`).
- `Diagnostics/`, `Staging/`, `Experimental/`, `Tests/` outputs, `Older mods/`, `Research/`, `TechnicalReviews/`, `HookDiagnostic/`, `OfflineBootstrap/`, `Config/` and the hundreds of per-package `Build-*.cmd` / `Run-*.cmd` scripts.
