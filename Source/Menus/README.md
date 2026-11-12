# Menus — custom setup rows, hosted-lobby rows, settings model

Paths relative to `Source/original-layout/`.

| File | Role / key symbols |
|---|---|
| `EventObserver/FrontendBundleRuntime.hpp` | Builds the offline setup menu. Around line 691 it assembles the row list: Counselor skill, Counselors (count up to `cfglimits::MaximumCounselors`), Jason difficulty, match length (30/60/90 min), starting weapon, starting inventory, plus practice rows ("Mute Pamela (keep sweater voice)", "Mute Distant Bot Grab Sounds", "Minimal HUD (keep objectives)", "Hard AI Jason Movement (2x)", "Bot Sweater Starts Convergence"). |
| `Core/ModeSettingsPolicy.hpp` | `cfgport::Settings`, `Validate(settings, catalogue)`: modes (OfflineCounselor, OfflineJason, Challenge, Online), skill/difficulty/weapon/inventory enums, minutes (30/60/90, 0 = unlimited only in offline Jason mode), challenge options. |
| `EventObserver/NativeSetupRows.hpp`, `NativeSettingsMenuOptions.hpp`, `NativeMenuRowAccess.hpp`, `NativeMenuDispatch.hpp`, `NativeMenuLaunchRuntime.hpp` | The row engine (`rows::Spec`, `rows::Runtime`): adds option rows to the game's existing settings widget, reads back choices. |
| `EventObserver/HostedSettingsSpecs.hpp`, `Core/HostedOptions.hpp` | Hosted-lobby rows: "Online: Jason Bot" (Off/On), "Online: Jason Bot Difficulty" (Off/Easy/Normal/Hard), "Online: Fill Empty Slots with Counselor Bots" (Off/On), "Online: Number of Jasons" (1/2). `cfghostedoptions::Store`, one immutable `Match` snapshot per hosted world. |
| `EventObserver/HostedLobbyRowsRuntime.hpp`, `HostedLobbyRuntime.hpp`, `HostedLobbyLayout.hpp`, `HostedLobbyPlacement.hpp` | Lobby panel placement/rows. |
| `Core/MenuCompanionPolicy.hpp`, `MenuReentryPolicy.hpp`, `MenuPreparationSchedule.hpp`, `SetupMenuModel.hpp`, `OfflineSettingsSelection.hpp`, `RestrictedInventoryMenu.hpp` | Menu preparation/re-entry, setup menu model, selection, restricted inventory options. |
| `EventObserver/FrontendPickerRuntime.hpp`, `FrontendSetupRuntime.hpp`, `FrontendEpochRuntime.hpp`, `FrontendRetentionRuntime.hpp` | Frontend (menu) epoch, picker and retention. |
| `EventObserver/NativeHumanJasonMenu.hpp`, `NativeHumanJasonOptionsRuntime.hpp`, `NativeChallengeSettingsSpecs.hpp` | Human-as-Jason and Challenges option rows. |

Port note: in CFG source these rows belong in the existing UMG settings widgets as normal entries bound to a `UDataAsset` / `USaveGame`;
see `../../UnrealIntegration/UI/`.
