# What is withheld from this public kit

To avoid publishing a ready-made recipe for hooking the Camp Forest Green 3.5 executable, the following were removed from this public
repository. None of them is needed for a native integration: in a native build the game calls its own functions by name, and the
GC-safety, address pinning and loader layers are deleted (see PORTING-STATUS.md, class D).

**Removed files**
- The external loader / injector: `EventObserver/IntegrationHarness.cpp`,
  `EventObserver/HarnessOwnedReportFixture.hpp`, `EventObserver/OwnedProcessFixture.cpp`, `RuntimeProbe/OfflineLoadTest.hpp`.
- The player launcher `Release/Launcher.cpp` (it starts the game and the loader). The installer scripts remain for reference.
- All generated address and binding tables (`*.generated.hpp`, 120 files) and the scripts that generate build pins (`Write-*BuildPin.ps1`).

**Redacted values (structure kept)**
- In the remaining source, 268 CFG 3.5 executable addresses (values inside the shipping image range) are replaced by `0x0` with the
  comment `/*CFG3.5 address withheld*/`, and 41 copied game-code byte signatures by `{}` with `/*CFG3.5 code signature withheld*/`.
  Names, call sequences, field meanings and all decision logic are unchanged. Small struct field offsets are kept: they document which
  game data each behavior reads.

**Consequence**
- `Source/original-layout/` is a reading and porting reference; it does not build into a working DLL. The pure policy headers in `Core/`
  and most fixtures in `Tests/` do not depend on the withheld files.

- Integration code for the author's private development tooling was removed. Where it was woven into gameplay code, only the
  tooling side was taken out; in two places the glue was simplified (bundle service pacing, and counselor F3 input now reads the
  keyboard directly). This reference therefore differs slightly from the shipped player DLL in those spots.

**Camp Forest Green developers**
- The complete, unredacted source (including the loader, address tables and the original donor project) is available to the CFG team on
  request from the author, AntagonyOG (https://www.nexusmods.com/profile/antagonyOG), under written terms as required by LICENSE.
