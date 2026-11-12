@echo off
setlocal EnableExtensions
cd /d "%~dp0"
call Build-WorkspaceEnvironment.cmd
call "F:\Program Files\Microsoft Visual Studio\18\Community\Common7\Tools\VsDevCmd.bat" -arch=x64 -host_arch=x64 >nul
if errorlevel 1 exit /b 1
cl /nologo /std:c++17 /EHsc /O2 /W4 /WX /MT EventObserver\DonorHeldGrabKillTests.cpp /FoTests\EventObserver\DonorHeldGrabKillTests.obj /FeTests\EventObserver\DonorHeldGrabKillTests.exe
if errorlevel 1 exit /b 1
Tests\EventObserver\DonorHeldGrabKillTests.exe
if errorlevel 1 exit /b 1
cl /nologo /std:c++17 /EHsc /O2 /W4 /WX /MT EventObserver\J21cHeldRecoveryTests.cpp /FoTests\EventObserver\J21cHeldRecoveryTests.obj /FeTests\EventObserver\J21cHeldRecoveryTests.exe
if errorlevel 1 exit /b 1
Tests\EventObserver\J21cHeldRecoveryTests.exe
if errorlevel 1 exit /b 1
cl /nologo /std:c++17 /EHsc /O2 /W4 /WX /MT EventObserver\J21cReportTests.cpp /FoTests\EventObserver\J21cReportTests.obj /FeTests\EventObserver\J21cReportTests.exe
if errorlevel 1 exit /b 1
Tests\EventObserver\J21cReportTests.exe
if errorlevel 1 exit /b 1
cl /nologo /std:c++17 /EHsc /O2 /W4 /WX /MT EventObserver\AdditionalJasonSelectionRuntimeTests.cpp /FoTests\EventObserver\AdditionalJasonSelectionRuntimeTests.obj /FeTests\EventObserver\AdditionalJasonSelectionRuntimeTests.exe /link user32.lib
if errorlevel 1 exit /b 1
Tests\EventObserver\AdditionalJasonSelectionRuntimeTests.exe
if errorlevel 1 exit /b 1
cl /nologo /std:c++17 /EHsc /O2 /W4 /WX /MT EventObserver\HelperBirthTests.cpp /FoTests\EventObserver\HelperBirthTests.obj /FeTests\EventObserver\HelperBirthTests.exe /link user32.lib
if errorlevel 1 exit /b 1
Tests\EventObserver\HelperBirthTests.exe
if errorlevel 1 exit /b 1
for %%i in (NativeJasonControllerTickObservation NativeChallengeOptions NativeChallengeRows NativeChallengeMenuRoute NativeChallengeIntro NativeChallengeRestart NativeChallengeBoundary ChallengeEscapeBoundaryPolicy) do (
 cl /nologo /std:c++17 /EHsc /O2 /W4 /WX /MT EventObserver\%%iTests.cpp /FoTests\EventObserver\%%iTests.obj /FeTests\EventObserver\%%iTests.exe /link user32.lib
 if errorlevel 1 exit /b 1
 Tests\EventObserver\%%iTests.exe
 if errorlevel 1 exit /b 1
)
cl /nologo /std:c++17 /EHsc /O2 /W4 /WX /MT EventObserver\FrontendMapArrivalPolicyTests.cpp /FoTests\EventObserver\FrontendMapArrivalPolicyTests.obj /FeTests\EventObserver\FrontendMapArrivalPolicyTests.exe
if errorlevel 1 exit /b 1
Tests\EventObserver\FrontendMapArrivalPolicyTests.exe
if errorlevel 1 exit /b 1
pwsh -NoProfile -File Tests\Cfg35J21cChallengesTests.ps1
if errorlevel 1 exit /b 1
exit /b 0