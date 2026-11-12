@echo off
setlocal EnableExtensions
cd /d "%~dp0"
call Build-WorkspaceEnvironment.cmd
if errorlevel 1 exit /b 1
call "F:\Program Files\Microsoft Visual Studio\18\Community\Common7\Tools\VsDevCmd.bat" -arch=x64 -host_arch=x64 >nul
if errorlevel 1 exit /b 1
if not exist Tests\EventObserver\HTestFixtures mkdir Tests\EventObserver\HTestFixtures
for %%t in (BundleObservation HostedMovementFallback H228HostedLane DoorRecovery HidingRuntimeMembers DonorJasonNavContinuation HostedJasonPresentationSource HostedWidgetDispatcherPolicy DonorJasonPursuit DonorJasonMorph AdditionalJasonSelectionRuntime) do (
  cl /nologo /std:c++17 /EHsc /O2 /W4 /WX /sdl /guard:cf /MT EventObserver\%%tTests.cpp /FoTests\EventObserver\HTestFixtures\%%t.obj /FeTests\EventObserver\HTestFixtures\%%t.exe /link /guard:cf bcrypt.lib user32.lib
  if errorlevel 1 exit /b 1
  Tests\EventObserver\HTestFixtures\%%t.exe
  if errorlevel 1 exit /b 1
)
pwsh -NoProfile -File Tests\Cfg35H231CaptureLoopTests.ps1
if errorlevel 1 exit /b 1
pwsh -NoProfile -File Tests\HostedPresentationRootsTests.ps1
if errorlevel 1 exit /b 1
pwsh -NoProfile -File Tests\HostedH242EvidenceTests.ps1
exit /b %errorlevel%
