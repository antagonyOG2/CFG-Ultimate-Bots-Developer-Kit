@echo off
rem G: is the only permitted project/build storage location.
rem Called without setlocal so child build scripts inherit these settings.
if /I not "%~d0"=="G:" exit /b 1
set "TEMP=%~dp0Tests\BuildTemp\tmp"
set "TMP=%TEMP%"
if not exist "%TEMP%" mkdir "%TEMP%"
if not exist "%TEMP%" exit /b 1
if not exist "%~dp0Tests\EventObserver" mkdir "%~dp0Tests\EventObserver"
if not exist "%~dp0Tests\RuntimeProbe" mkdir "%~dp0Tests\RuntimeProbe"
where /q pwsh.exe
if not errorlevel 1 exit /b 0
if exist "%ProgramFiles%\PowerShell\7\pwsh.exe" (
 set "PATH=%ProgramFiles%\PowerShell\7;%PATH%"
 exit /b 0
)
if exist "%USERPROFILE%\.cache\codex-runtimes\codex-primary-runtime\dependencies\native\powershell\pwsh.exe" (
 set "PATH=%USERPROFILE%\.cache\codex-runtimes\codex-primary-runtime\dependencies\native\powershell;%PATH%"
 exit /b 0
)
echo PowerShell 7 is required to build the package.
exit /b 1
