[CmdletBinding()]
param([Parameter(Mandatory)][ValidatePattern('^[0-9a-f]{40}$')][string]$NativeCommit,
    [ValidateSet('focused','hook','native-primary','reader-provider','receipt','ValidateOnly-Hosted')][string]$ResumeAt='focused')
$ErrorActionPreference='Stop';$root=$PSScriptRoot;$diagnostics=Join-Path $root 'Diagnostics\HTest'
[void][IO.Directory]::CreateDirectory($diagnostics)
$started=[DateTime]::UtcNow
$script:reachedResume=$false
function Gate([string]$name,[string]$file,[string[]]$arguments=@()){
    if(!$script:reachedResume){if($name -ne $ResumeAt){Write-Host ('SKIP already passed '+$name);return};$script:reachedResume=$true}
    if(!(Test-Path -LiteralPath 'G:\CFG-Shared\MAILBOX.md')){throw 'Shared mailbox unavailable before package gate'}
    $log=Join-Path $diagnostics ($name+'.log')
    Push-Location $root
    try{
        if($file.EndsWith('.cmd')){& $env:ComSpec /d /c $file @arguments *> $log}
        else{& (Get-Command pwsh).Source -NoProfile -File (Join-Path $root $file) @arguments *> $log}
    }finally{Pop-Location}
    $code=$LASTEXITCODE;$body=Get-Content -LiteralPath $log -Raw
    if($code -or $body -match '(?im)error C\d+|fatal error|^FAIL\b|Assertion failed:|\b[1-9]\d* failures\b'){
        throw "Gate refused: $name (exit $code); log=$log"
    }
    Write-Host ('PASS '+$name)
}
$env:CFG35_PACKAGE_PORT='HTestPort';$env:CFG35_PACKAGE_RECEIPT='CFG35-HTest-TestReceipt.json'
Gate 'focused' 'Build-Cfg35-HTestFocused.cmd'
if($ResumeAt -in @('focused','hook')){
    $port=Join-Path $root 'Tests\EventObserver\HTestPort'
    if(Test-Path -LiteralPath $port){
        $primary=Join-Path $port 'Cfg35PrimaryGameplay.dll'
        if(!(Test-Path -LiteralPath $primary)){throw 'Existing rolling port has no primary to identify its archive'}
        $hash=(Get-FileHash -LiteralPath $primary).Hash
        $archive=Join-Path 'G:\Codex\CFG35-HTest-Archive' $hash.Substring(0,8)
        if(!(Test-Path -LiteralPath $archive)){
            [void][IO.Directory]::CreateDirectory($archive)
            Copy-Item -LiteralPath $port -Destination $archive -Recurse
            $oldReceipt=Join-Path $root 'Diagnostics\CFG35-HTest-TestReceipt.json'
            if(Test-Path -LiteralPath $oldReceipt){Copy-Item -LiteralPath $oldReceipt -Destination $archive}
        }
        if((Get-FileHash -LiteralPath (Join-Path $archive 'HTestPort\Cfg35PrimaryGameplay.dll')).Hash -cne $hash){throw 'Rolling port archive hash mismatch'}
    }
}
Gate 'hook' 'Build-Cfg35-HTestHook.cmd'
Gate 'native-primary' 'Build-Cfg35-HTestNativeProduction.cmd'
Gate 'reader-provider' 'Build-Cfg35-HTestPortHarness.cmd'
Gate 'receipt' 'Write-Cfg35HTestReceipt.ps1' @('-NativeCommit',$NativeCommit)
Gate 'ValidateOnly-Hosted' 'Launch-CFG35-ParityTest.ps1' @('-Route','Hosted','-ValidateOnly')
$receiptPath=Join-Path $root 'Diagnostics\CFG35-HTest-TestReceipt.json'
$receipt=Get-Content -LiteralPath $receiptPath -Raw|ConvertFrom-Json
foreach($artifact in $receipt.Artifacts){
    if((Get-FileHash -LiteralPath (Join-Path $root $artifact.File)).Hash -cne $artifact.SHA256){throw "Final artifact hash mismatch: $($artifact.File)"}
    Write-Host ($artifact.File+' '+$artifact.SHA256)
}
Write-Host ('Receipt '+(Get-FileHash -LiteralPath $receiptPath).Hash)
Write-Host ('PASS HTest Tier 1 package; elapsed minutes='+[Math]::Round(([DateTime]::UtcNow-$started).TotalMinutes,1))