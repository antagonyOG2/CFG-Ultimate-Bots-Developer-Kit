# CFG Offline Bots installer. Run by INSTALL.bat (double-click). Never changes Camp Forest Green's own files or saves.
# It (1) finds Camp Forest Green on ANY drive, (2) puts this mod folder inside it, (3) checks every file,
# (4) unblocks the downloaded files, (5) creates "CFG Offline Bots" shortcuts on the Desktop and in the Start menu.
$ErrorActionPreference='Stop'
$ModName='CFG-Offline-Bots'
$Here=Split-Path -Parent $MyInvocation.MyCommand.Path
function Say($t,$c='Gray'){Write-Host $t -ForegroundColor $c}
function Fail($t){Say '' ;Say ('PROBLEM: '+$t) 'Red';Say '';Read-Host 'Press Enter to close' | Out-Null;exit 1}
function IsCfg($dir){$dir -and (Test-Path -LiteralPath (Join-Path $dir 'Start_CampForestGreen.exe')) -and
    (Test-Path -LiteralPath (Join-Path $dir 'CampForestGreen\Binaries\Win64\SummerCamp-Win64-Shipping.exe'))}
Say '=== CFG Offline Bots installer ===' 'Cyan'
$version='__CFG_RELEASE_VERSION__'
Say ('Version: '+$version)

# 1) Where is Camp Forest Green?
$parent=Split-Path -Parent $Here
$cfg=$null
if(IsCfg $parent){$cfg=$parent;Say ('Camp Forest Green found here: '+$cfg) 'Green'}
else{
    Say 'Looking for Camp Forest Green on all drives (Steam libraries and the CFG launcher folder)...'
    $found=[System.Collections.Generic.List[string]]::new()
    $add={param($d) if((IsCfg $d) -and -not ($found -contains $d)){$found.Add((Resolve-Path -LiteralPath $d).Path)}}
    # a running game tells us exactly where it is
    Get-Process -Name 'SummerCamp-Win64-Shipping' -ErrorAction SilentlyContinue | ForEach-Object {
        try{& $add (Split-Path -Parent (Split-Path -Parent (Split-Path -Parent (Split-Path -Parent $_.Path))))}catch{}}
    # Steam libraries, wherever Steam is installed
    $steamRoots=@()
    foreach($key in 'HKCU:\Software\Valve\Steam','HKLM:\SOFTWARE\WOW6432Node\Valve\Steam','HKLM:\SOFTWARE\Valve\Steam'){
        try{$v=Get-ItemProperty -LiteralPath $key -ErrorAction Stop;foreach($n in 'SteamPath','InstallPath'){if($v.$n){$steamRoots+=($v.$n -replace '/','\')}}}catch{}}
    $libraries=@()
    foreach($s in $steamRoots|Select-Object -Unique){
        $libraries+=$s
        $vdf=Join-Path $s 'steamapps\libraryfolders.vdf'
        if(Test-Path -LiteralPath $vdf){foreach($m in [regex]::Matches((Get-Content -LiteralPath $vdf -Raw),'"path"\s+"([^"]+)"')){$libraries+=($m.Groups[1].Value -replace '\\\\','\')}}}
    foreach($lib in $libraries|Select-Object -Unique){
        $common=Join-Path $lib 'steamapps\common'
        if(Test-Path -LiteralPath $common){Get-ChildItem -LiteralPath $common -Directory -ErrorAction SilentlyContinue|ForEach-Object{& $add $_.FullName}}}
    # The Camp Forest Green launcher folder on every fixed drive (any drive letter, any top-level folder)
    foreach($drive in [IO.DriveInfo]::GetDrives()|Where-Object{$_.DriveType -eq 'Fixed' -and $_.IsReady}){
        $r=$drive.RootDirectory.FullName
        foreach($top in @($r)+@(Get-ChildItem -LiteralPath $r -Directory -ErrorAction SilentlyContinue|ForEach-Object FullName)){
            $games=Join-Path $top 'CampForestGreenLauncher\Games'
            if(Test-Path -LiteralPath $games){Get-ChildItem -LiteralPath $games -Directory -ErrorAction SilentlyContinue|ForEach-Object{& $add $_.FullName}}}}
    if($found.Count -eq 1){$cfg=$found[0];Say ('Camp Forest Green found: '+$cfg) 'Green'}
    elseif($found.Count -gt 1){
        Say 'More than one Camp Forest Green install was found:' 'Yellow'
        for($i=0;$i -lt $found.Count;$i++){Say ('  '+($i+1)+') '+$found[$i])}
        $pick=Read-Host 'Type the number of the one you play'
        if($pick -match '^\d+$' -and [int]$pick -ge 1 -and [int]$pick -le $found.Count){$cfg=$found[[int]$pick-1]}else{Fail 'No install chosen; nothing changed.'}
    }else{
        Say 'Camp Forest Green was not found automatically.' 'Yellow'
        $typed=(Read-Host 'Paste the folder that contains Start_CampForestGreen.exe').Trim('"',' ')
        if(IsCfg $typed){$cfg=(Resolve-Path -LiteralPath $typed).Path}else{Fail 'That folder does not contain Start_CampForestGreen.exe and CampForestGreen\Binaries\Win64. Nothing changed.'}
    }
}

# 2) Put the mod folder inside Camp Forest Green (copy if it is elsewhere; never touches the game's own files)
$target=Join-Path $cfg $ModName
if(-not [string]::Equals((Resolve-Path -LiteralPath $Here).Path.TrimEnd('\'),$target.TrimEnd('\'),[StringComparison]::OrdinalIgnoreCase)){
    if(Get-Process -Name 'SummerCamp-Win64-Shipping' -ErrorAction SilentlyContinue){Fail 'Close Camp Forest Green first, then run INSTALL.bat again.'}
    if(Test-Path -LiteralPath $target){Say ('Updating the existing mod folder (save backups kept): '+$target)}
    New-Item -ItemType Directory -Force -Path $target | Out-Null
    Get-ChildItem -LiteralPath $Here -File | ForEach-Object {Copy-Item -LiteralPath $_.FullName -Destination (Join-Path $target $_.Name) -Force}
    Say ('Mod copied into: '+$target) 'Green'
}

# 3) Every file present and intact (antivirus or a partial download are the usual causes of trouble)
$sums=Join-Path $target 'CHECKSUMS-SHA256.txt'
if(-not (Test-Path -LiteralPath $sums)){Fail 'CHECKSUMS-SHA256.txt is missing. Re-download the mod.'}
$bad=@()
foreach($line in Get-Content -LiteralPath $sums){
    if($line -notmatch '^([0-9A-F]{64})\s+\*?(.+)$'){continue}
    $file=Join-Path $target $Matches[2].Trim()
    if(-not (Test-Path -LiteralPath $file)){$bad+=($Matches[2]+' (missing)');continue}
    if((Get-FileHash -LiteralPath $file -Algorithm SHA256).Hash -ne $Matches[1]){$bad+=($Matches[2]+' (changed)')}
}
if($bad.Count){Fail ("These files are missing or damaged: "+($bad -join ', ')+". Re-extract the download; if a file keeps disappearing, your antivirus removed it - allow the CFG-Offline-Bots folder and extract again.")}
Say 'All mod files verified.' 'Green'

# 4) Downloaded files carry a "blocked" mark that can stop the launcher or DLLs from loading
Get-ChildItem -LiteralPath $target -File | ForEach-Object {try{Unblock-File -LiteralPath $_.FullName}catch{}}

# 5) Game version check (warn only; the launcher refuses to load the mod into an unsupported version)
$expected='__CFG_GAME_SHA256__'
$gameExe=Join-Path $cfg 'CampForestGreen\Binaries\Win64\SummerCamp-Win64-Shipping.exe'
if($expected -and (Get-FileHash -LiteralPath $gameExe -Algorithm SHA256).Hash -ne $expected.Trim()){
    Say 'WARNING: your Camp Forest Green version is not the one this mod version supports (the game was probably updated).' 'Yellow'
    Say 'The shortcut will still start the game, but the mod will not load until a matching mod update is installed.' 'Yellow'}

# 6) Shortcuts
$launcher=Join-Path $target 'CFG Offline Bots.exe'
$shell=New-Object -ComObject WScript.Shell
$made=@()
foreach($dir in @([Environment]::GetFolderPath('Desktop'),(Join-Path ([Environment]::GetFolderPath('Programs')) ''))){
    if(-not $dir){continue}
    $lnk=Join-Path $dir 'CFG Offline Bots.lnk'
    $s=$shell.CreateShortcut($lnk);$s.TargetPath=$launcher;$s.WorkingDirectory=$target;$s.IconLocation=($gameExe+',0')
    $s.Description='Camp Forest Green with CFG Offline Bots';$s.Save();$made+=$lnk
}
[pscustomobject]@{Version=$version;GameFolder=$cfg;ModFolder=$target;Shortcuts=$made;Utc=[DateTime]::UtcNow.ToString('o')} |
    ConvertTo-Json | Set-Content -LiteralPath (Join-Path $target 'install.json') -Encoding UTF8
Say ''
Say 'INSTALLED.' 'Green'
Say 'Start the game with the "CFG Offline Bots" shortcut on your Desktop (or Start menu).'
Say 'Open Offline Play once after the game starts - the mod is loaded when the extra setup rows (Counselor Bot Skill, Jason Difficulty, Match Length) appear.'
Say 'Then play Offline, Online (host a lobby) or Challenges as usual.'
if((Resolve-Path -LiteralPath $Here).Path.TrimEnd('\') -ne $target.TrimEnd('\')){Say ('You can delete the folder you extracted: '+$Here) 'DarkGray'}
Say ''
Read-Host 'Press Enter to close' | Out-Null
