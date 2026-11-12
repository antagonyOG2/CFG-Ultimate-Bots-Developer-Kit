# Loader — injector, installer, player launcher

Paths relative to `Source/original-layout/`. None of this is needed in a native port.

| File | Role / key symbols |
|---|---|
| `EventObserver/IntegrationHarness.cpp` *(withheld from the public kit)* | The injector/loader executable. Modes include `--self-test`, `--install`, `--auto-install` (waits read-only for the player to open Offline Play, then installs the gameplay DLL once), `--capture-all`, `--watch-menu`, `--hosted-match-state`. Verifies pinned DLL SHA-256 before use. |
| `EventObserver/Write-*BuildPin.ps1` *(withheld from the public kit)* | Generate `*.generated.hpp` pins (DLL path, size, SHA-256) at build time. |
| `Release/Launcher.cpp` *(withheld from the public kit)* | Windowless player launcher ("CFG Offline Bots.exe"): checks layout/files/game SHA-256, backs up saves, starts the game, starts the hidden loader. |
| `Release/Install.ps1`, `Uninstall.ps1`, `INSTALL.bat`, `UNINSTALL.bat`, `README.txt` | Player installer/uninstaller (copies to `<CFG>\CFG-Offline-Bots`, shortcuts). |
| `EventObserver/OfflineFrontendBundle.cpp` | The translation unit built (`/LD`) into the gameplay DLL; it includes `OfflineEventObserver.cpp` (the observer / hook entry), which in turn includes `GameplayBundleRuntime.hpp` (line 580) and `FrontendBundleRuntime.hpp` (line 766). |
