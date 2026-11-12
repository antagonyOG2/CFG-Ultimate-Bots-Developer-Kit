# CFG Offline Bots uninstaller. Run by UNINSTALL.bat. Removes only what the mod added:
# its shortcuts and its own CFG-Offline-Bots folder. Camp Forest Green's files and saves are never touched
# (the mod never changes them). Save backups can be kept on the Desktop.
$ErrorActionPreference='Stop'
$Here=Split-Path -Parent $MyInvocation.MyCommand.Path
function Say($t,$c='Gray'){Write-Host $t -ForegroundColor $c}
Say '=== CFG Offline Bots uninstaller ===' 'Cyan'
if(Get-Process -Name 'SummerCamp-Win64-Shipping' -ErrorAction SilentlyContinue){Say 'Close Camp Forest Green first, then run UNINSTALL.bat again.' 'Red';Read-Host 'Press Enter to close'|Out-Null;exit 1}
$shortcuts=@((Join-Path ([Environment]::GetFolderPath('Desktop')) 'CFG Offline Bots.lnk'),(Join-Path ([Environment]::GetFolderPath('Programs')) 'CFG Offline Bots.lnk'))
$manifest=Join-Path $Here 'install.json'
if(Test-Path -LiteralPath $manifest){try{$shortcuts+=@((Get-Content -LiteralPath $manifest -Raw|ConvertFrom-Json).Shortcuts)}catch{}}
foreach($l in $shortcuts|Select-Object -Unique){if($l -and (Test-Path -LiteralPath $l)){Remove-Item -LiteralPath $l -Force;Say ('Removed shortcut: '+$l)}}
$backups=Join-Path $Here 'Backups'
if((Test-Path -LiteralPath $backups) -and (Get-ChildItem -LiteralPath $backups -ErrorAction SilentlyContinue)){
    $keep=Read-Host 'Keep your save backups (copied to your Desktop)? Y/N [Y]'
    if($keep -notmatch '^[Nn]'){$dest=Join-Path ([Environment]::GetFolderPath('Desktop')) ('CFG-Offline-Bots-SaveBackups');Copy-Item -LiteralPath $backups -Destination $dest -Recurse -Force;Say ('Save backups kept in: '+$dest) 'Green'}
}
# Delete this folder after this window closes (a running script cannot delete its own folder).
$folder=$Here
Say ''
Say 'UNINSTALLED. Camp Forest Green itself was not changed; start it normally from Steam or its launcher.' 'Green'
Read-Host 'Press Enter to close (the mod folder is deleted a few seconds later)' | Out-Null
Start-Process -FilePath $env:ComSpec -WorkingDirectory $env:TEMP -ArgumentList ('/c timeout /t 4 /nobreak >nul & rmdir /s /q "'+$folder+'"') -WindowStyle Hidden
