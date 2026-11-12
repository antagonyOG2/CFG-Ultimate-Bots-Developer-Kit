@echo off
setlocal EnableExtensions
cd /d "%~dp0"
call Build-WorkspaceEnvironment.cmd
if errorlevel 1 exit /b 1
call "F:\Program Files\Microsoft Visual Studio\18\Community\Common7\Tools\VsDevCmd.bat" -arch=x64 -host_arch=x64 >nul
if errorlevel 1 exit /b 1
cl /nologo /std:c++17 /EHsc /O2 /W4 /WX /sdl /guard:cf /MT EventObserver\DonorHeldGrabKillTests.cpp /FoTests\EventObserver\DonorHeldGrabKillTests.obj /FeTests\EventObserver\DonorHeldGrabKillTests.exe
if errorlevel 1 exit /b 1
Tests\EventObserver\DonorHeldGrabKillTests.exe
if errorlevel 1 exit /b 1
cl /nologo /std:c++17 /EHsc /O2 /W4 /WX /sdl /guard:cf /MT EventObserver\DonorNativeTurnTests.cpp /FoTests\EventObserver\DonorNativeTurnTests.obj /FeTests\EventObserver\DonorNativeTurnTests.exe
if errorlevel 1 exit /b 1
Tests\EventObserver\DonorNativeTurnTests.exe
if errorlevel 1 exit /b 1
cl /nologo /std:c++17 /EHsc /O2 /W4 /WX /sdl /guard:cf /MT EventObserver\HeldRecoveryRuntimeTests.cpp /FoTests\EventObserver\HeldRecoveryRuntimeTests.obj /FeTests\EventObserver\HeldRecoveryRuntimeTests.exe
if errorlevel 1 exit /b 1
Tests\EventObserver\HeldRecoveryRuntimeTests.exe
exit /b %errorlevel%
