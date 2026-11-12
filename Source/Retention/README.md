# Retention (GC safety) — exists only because the mod is an external DLL

Paths relative to `Source/original-layout/`.

An external DLL cannot register `UPROPERTY()` references, so any `UObject*` it keeps across a frame can be garbage collected or
reused. These files keep objects alive and prove object identity (object-array index + serial number) before every use.
**In a native C++ port all of this is deleted**: use `UPROPERTY()` / `TObjectPtr` / `TWeakObjectPtr` and `IsValid()`.

| File | Role / key symbols |
|---|---|
| `EventObserver/CallbackRootLease.hpp` | `cfgretain::CallbackRootLease<Bank,Limit>`: lifetime of ONE completed engine callback. `Retain`, `PrepareCall`/`EnterNative`/`CompleteCall`, `Close`; `ProvenSinceLastNative` (H2.92 skip of re-proof windows). |
| `EventObserver/ResidentRootBank.hpp`, `PermanentRootPool.hpp`, `RootSlotLedger.hpp`, `SynchronizedRootOwner.hpp`, `ResidentRootRegistration.hpp`, `RootPoisonDiagnostic.hpp`, `RootPublicationProbe.hpp` | The resident root bank (slots registered with the collector) and diagnostics. |
| `EventObserver/BundleCallbackRootsRuntime.hpp`, `BundleRetentionRuntimeMembers.hpp`, `SoftClassRetentionRuntimeMembers.hpp`, `PrimaryJasonRetentionRuntime.hpp` | Bundle-level use of the lease inside each Jason tick (`BundleCallbackRootScope`). |
| `EventObserver/GcReferenceOwner.hpp`, `GcRegistryProbe.hpp`, `GcRegistryProbeRuntime.hpp`, `GcOwnerDiagnosticRegistry.hpp` | Collector registry access/probe. |
| `RuntimeProbe/ObjectLifetime.hpp` | `cfgprobe::lifetime::SlotObservation` (object, index, serial) — the identity token used in every `Same*`/`SameSlot` check. |
| `Core/WorldRootRetirement.hpp`, `EventObserver/WorldRootRetirementRuntime.hpp`, `WorldRootInventory.hpp` | Release of roots at world end. |
| Tests: `Tests/CallbackRootLeaseTests.cpp` | Verifies lease state machine. |
