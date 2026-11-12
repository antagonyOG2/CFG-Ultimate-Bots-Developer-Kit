# Gameplay — bundle runtime, report struct, feature flags, difficulty

Paths relative to `Source/original-layout/`.

| File | Role / key symbols |
|---|---|
| `EventObserver/GameplayBundleRuntime.hpp` | The gameplay "bundle" runtime (about 5,000 lines): schedules every Jason/counselor feature, owns the per-match state (`bundleJason`, `bundleAdditional`, `bundleKnife`, `bundleFootBurst`, ...), `ResetBundleWorldState(now)`, F1/F3 key handling. |
| `Core/GameplayBundlePolicy.hpp` | `cfgbundle::Report` — the fixed-layout, versioned (`ReportVersion=87`) counter struct the DLL publishes for captures: spawn attempts, knife stages, car stages, convergence, per-Jason rows (`h281Unstick`, `h283Kill`, `h284Cutoff`, `h285Starts`, `h286Lead`, `h288FailedRings`, `h291*` timing, ...). Also `Schedule`, `Bot`, `Lane`, `CameraTurn`. |
| `Core/J22FeatureFlags.hpp` | Every feature flag introduced H2.5x-H2.95 (`CFG_*`), each `#ifndef`-guarded with a comment on why. Other flag headers: `H20FeatureFlags.hpp`, `J214FeatureFlags.hpp`, `J215FeatureFlags.hpp`, `J21cFeatureFlags.hpp`, `J24FeatureFlags.hpp`. See `../../Documentation/FEATURE-FLAGS.md`. |
| `Core/GameplayConfiguration.hpp`, `GameplayLimits.hpp`, `GameplayReportCapture.hpp` | Configuration snapshot, limits (e.g. max counselors), capture. |
| `EventObserver/DifficultyPolicy.hpp`, `DifficultyRuntimeMembers.hpp`, `Core/JasonRageDifficultyPolicy.hpp` | Jason difficulty. |
| `Core/J21FrameBudget.hpp`, `J21WorkPolicy.hpp`, `OptionalWorkAdmission.hpp`, `EventObserver/J21Timing.hpp` | Per-frame work/read budgets (an external-DLL concern; a native port does not need them). |
| `Core/SpectatorInputIntent.hpp`, `SpectatorRefusalPolicy.hpp`, `DeadSpectatorRecoveryPolicy.hpp`, `EventObserver/SpectatorRuntimeMembers.hpp` | F4 spectator view of AI Jason. |
| `Core/EndingMetadataPolicy.hpp`, `EndingSummaryPolicy.hpp`, `SummaryPresentationPolicy.hpp` | Match ending / summary (end-summary replay flags are OFF in 1.0.0). |
