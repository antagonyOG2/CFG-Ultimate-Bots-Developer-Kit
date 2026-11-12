# BT_JasonHunt — suggested tree layout

**EXAMPLE — not compiled or tested.** The mod drives Jason with code, not a BT; this layout is how the same priority order could be
expressed as a Behavior Tree. Priority order is taken from `Core/DonorJasonControllerTick.hpp`
(`DonorRunOfflineBotsControllerTickOnGameThread`) and `Core/DonorJasonChase.hpp`.

```
Root (Selector)
|- [Service] BTService_JasonFindTarget          (every 0.5 s: target, held/water flags; mirrors decision frame)
|- Sequence: StartupTraps            [Decorator: StartupTrapSetupActive]            -> Task: PlaceStartupTraps
|- Sequence: DoorBreak               [Decorator: DoorBreakInProgress]               -> Task: TickDoorBreak
|- Sequence: KnifeSequence           [Decorator: KnifeSequenceActive]               -> Task: ContinueKnifeSequence
|- Sequence: HeldGrabKill            [Decorator: HeldVictim set]                    -> Task: DriveGrabKill   (cfgdonorgrab::State::Drive)
|- Sequence: Unstick                 [Decorator: UnstickDue (water>=8s / frozen>=12s)] -> Task: MorphNearCounselor (cut-off ring)
|- Sequence: Burst                   [Decorator: BurstWanted (>=4 m, ready)]        -> Task: BTTask_JasonSpeedBurst
|- Sequence: Interact                [Decorator: Blocked door / occupied hiding spot in reach] -> Task: OpenOrBreakDoor / SearchHidingSpot
|- Sequence: Chase                                                                  -> Task: MoveTo(TargetLocation, acceptance 85)
                                                                                         (Task: DistanceMorph when far / cooldown over)
```

Notes grounded in the source:

* Order matters: the real tick runs traps, door break, knife, then the chase update, then combat (which "owns owed attack release and
  held-counselor input even when a preceding action changed state").
* The stock driver call (stock AI behavior) runs last when none of the owned sequences hold the controller (`SafeJasonDriverCall`). In a
  native build this is simply "fall through to the existing stock tree".
* Re-evaluate `Chase` every 500 ms while bursting, otherwise honor the 3 s path lock.
* Cars (primary only) are a separate branch above `Chase` and are not part of the 1.0.0 verified F1 behavior.
