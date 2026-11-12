@echo off
setlocal EnableExtensions
cd /d "%~dp0"
call "%~dp0Build-WorkspaceEnvironment.cmd"
if not "%errorlevel%"=="0" exit /b %errorlevel%
call "F:\Program Files\Microsoft Visual Studio\18\Community\Common7\Tools\VsDevCmd.bat" -arch=x64 -host_arch=x64 >nul
if not "%errorlevel%"=="0" exit /b %errorlevel%
if not exist Tests\EventObserver\HTestPort mkdir Tests\EventObserver\HTestPort
if not exist Tests\EventObserver\HTestPort\Temp mkdir Tests\EventObserver\HTestPort\Temp
set "TEMP=%CD%\Tests\EventObserver\HTestPort\Temp"
set "TMP=%CD%\Tests\EventObserver\HTestPort\Temp"
cl /DCFG_H233_TRAP_ROSTER=1 /DCFG_H233_HIDING_ROSTER=1 /DCFG_H233_MORPH_ADMISSION=1 /DCFG_H233_LABEL_TRACE=1 /DCFG_H1_OBSERVE /DCFG_H1_JASON /DCFG_H20_MENU=1 /DCFG_H21_LOBBY_DISCOVERY=1 /DCFG_H22_LOBBY=1 /DCFG_H22_HOTKEYS=1 /DCFG_H21_LOBBY_ROWS=1 /DCFG_H20_FILL=1 /DCFG_J219_FINISHING=1 /DCFG_J218_FINISHING=1 /DCFG_J217_FINISHING=1 /DCFG_J36_CHALLENGE_WALLS=1 /DCFG_J216_FINISHING=1 /DCFG_J21C_RESIDENT_WEAPONS=1 /DCFG_J215_CHALLENGE_WALL_OWNERSHIP=1 /DCFG_J215_PAMELA_MUTE=1 /DCFG_J215_GRAB_MUTE=1 /DCFG_J215_MINIMAL_HUD=1 /DCFG_J215_HARD_MOVEMENT=1 /DCFG_J215_BOT_SWEATER=1 /DCFG_J214_TRAP_PRIORITY=1 /DCFG_J214_INTRO_END_TO_END=1 /DCFG_J214_HUMAN_JASON_OPTIONS=1 /DCFG_H218_HOSTED_SPECTATOR=1 /nologo /std:c++17 /EHsc /O2 /W4 /WX /sdl /guard:cf /MT /DCFG35_URINATION_NATIVE_PORT /DCFG_RETAINED_PRIMARY_JASON /DCFG_RETAINED_GAMEPLAY_BUNDLE /DCFG35_MENU_RETENTION_TEST /DCFG35_NATIVE_COUNSELOR_STARTUP_TEST /DCFG35_PRIMARY_GAMEPLAY_TEST /DCFG35_KNIFE_BOUNDARY_OBSERVE /DCFG35_KNIFE_RELEASE_ADAPTER /LD EventObserver\OfflineFrontendBundle.cpp Tests\EventObserver\nativehook.obj Tests\EventObserver\observerbuffer.obj Tests\EventObserver\buffer.obj Tests\EventObserver\trampoline.obj Tests\EventObserver\hde64.obj /FoTests\EventObserver\HTestPort\Cfg35UrinationPort.obj /FeTests\EventObserver\HTestPort\Cfg35UrinationPort.dll /link /guard:cf bcrypt.lib user32.lib /IMPLIB:Tests\EventObserver\HTestPort\Cfg35UrinationPort.lib /MAP:Tests\EventObserver\HTestPort\Cfg35UrinationPort.map
if not "%errorlevel%"=="0" exit /b %errorlevel%
echo PASS HTest hosted native build; prior outputs untouched.
exit /b 0


