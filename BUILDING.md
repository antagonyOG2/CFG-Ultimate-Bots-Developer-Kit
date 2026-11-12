# Building

**The kit is a reference. It is not expected to build standalone** — the full repository (including generated pin headers, the original donor
project sources it was transcribed from, vendor libraries and build scripts) and the exact game binary are required to produce the working mod.
This document records how the author builds it so the structure can be understood. Paths in the scripts are the author's development layout
(for example `F:\Program Files\Microsoft Visual Studio\18\Community`, `G:\Codex\...`) and are not portable. Nothing here was run while assembling the kit.

## Toolchain
* Windows x64, Visual Studio 2022 or newer toolset (the author uses VS "18" Community), C++17.
* Compiler flags seen across the scripts: `cl /nologo /std:c++17 /EHsc /O2 /W4 /WX /sdl /guard:cf /MT` (static CRT, warnings as errors, control-flow guard), linking `bcrypt.lib psapi.lib advapi32.lib` and `/STACK:8388608,65536` for the loader.
* A call to `VsDevCmd.bat -arch=x64 -host_arch=x64` precedes every build; `Build-WorkspaceEnvironment.cmd` points `TEMP` at a project folder.
* PowerShell 7 (`pwsh`) is used by the package/pin scripts.

## Build flow (author's repo)

Reference copies are in `Documentation/BuildScripts-Reference/`:

| Script | Role |
|---|---|
| `Build-Cfg35-HTestPortHarness.cmd` *(withheld from the public kit)* | Builds the **loader/harness** (`EventObserver/IntegrationHarness.cpp` *(withheld from the public kit)*) with the feature defines for the current "HTest" line: `CFG_H1_OBSERVE`, `CFG_H1_JASON`, `CFG_H20_MENU=1`, `CFG_J219_FINISHING=1` (and J218..J216 finishing flags), `CFG_J215_*` practice options, `CFG_FRONTEND_BUNDLE`, `CFG_SPAWN_ADAPTER`, `CFG_JASON_APPROACH`, `CFG_PRIMARY_JASON`, `CFG_GAMEPLAY_BUNDLE`, `CFG_RETAINED_PRIMARY_JASON`, `CFG_RETAINED_GAMEPLAY_BUNDLE`, `CFG35_HTest_PIN`. Writes the build pins (`Write-JasonApproachBuildPin.ps1`) and runs the self-test. |
| `Build-Cfg35-HTestNativeProduction.cmd` | Builds the **gameplay DLL** (`/LD`, source `EventObserver/OfflineFrontendBundle.cpp`) as `Cfg35UrinationPort.dll`, with the same defines plus the hosted/trap/hiding roster flags; the harness script copies it to `Cfg35PrimaryGameplay.dll`. |
| `Build-Cfg35-HTestFocused.cmd` | The focused fixture gate that runs before packaging. |
| `Build-Cfg35-HTestPackage.ps1` | The "Run-HTestPackage" flow: runs the gates in order (`focused` -> `hook` -> `native-primary` -> `reader-provider` -> `receipt`), refusing on any `error C...`, `Assertion failed`, or `FAIL` in a log; archives the previous rolling package by hash; writes a test receipt. |
| `Build-Cfg35-J2HeldKillTests.cmd`, `Build-Cfg35-J21cCoreTests.cmd` | Examples of how individual fixtures are compiled and run (`cl ... EventObserver\DonorHeldGrabKillTests.cpp`). |

The author's full repository has about 460 build scripts, one per development package; only a representative few are included. The player-release build script is available from the author.

## Compiling one policy fixture
See `Tests/README.md`. A fixture needs only `cl`, the `Source/original-layout` headers and `/I Source\original-layout\EventObserver`.

## What you cannot reproduce from this kit alone
* The gameplay DLL (missing: generated `*BuildPin.generated.hpp`, `vendor/` libraries, the donor source tree the code was transcribed from, local test fixtures).
* A runnable mod (needs the exact CFG 3.5 executable; all native addresses are pinned to it).

## Player install flow (for reference)
`Source/original-layout/Release/`: `Launcher.cpp`, `Install.ps1`, `Uninstall.ps1`, `INSTALL.bat`, `UNINSTALL.bat`, `README.txt` — the installer copies the mod folder into the
CFG folder and creates shortcuts; the launcher checks the game SHA-256, backs up saves, starts the game and the loader.
