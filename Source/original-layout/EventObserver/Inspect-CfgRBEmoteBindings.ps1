$ErrorActionPreference='Stop'
$rbRoot=Split-Path $PSScriptRoot -Parent
$rbPackage=Import-PowerShellDataFile (Join-Path $rbRoot 'CFG-Live-Package.psd1')
$rbExe=Join-Path $rbPackage.GameRoot 'CampForestGreen\Binaries\Win64\SummerCamp-Win64-Shipping.exe'
$rbHash=(Get-FileHash -LiteralPath $rbExe -Algorithm SHA256).Hash
if($rbHash -ne $rbPackage.ShippingSha256){throw 'Unknown CFG image'}
Add-Type -LiteralPath @((Join-Path $rbRoot 'Research/PeDiskImage.cs'),(Join-Path $rbRoot 'Research/FunctionFactories.cs'))
$rbBytes=[IO.File]::ReadAllBytes($rbExe);$rbImage=[CfgPeDiskImage]::new($rbBytes)
$rbClassHelper=[Convert]::ToUInt32($rbPackage.DiskRebindCandidates.ClassFactoryHelperRva.Substring(2),16)
$rbCtorHelper=[Convert]::ToUInt32($rbPackage.DiskRebindCandidates.ClassConstructorHelperRva.Substring(2),16)
$rbFunctionHelper=[Convert]::ToUInt32($rbPackage.DiskRebindCandidates.FunctionFactoryHelperRva.Substring(2),16)
$rbClasses=@($rbImage.ClassNameCandidates($rbClassHelper,$rbCtorHelper,[string[]]@('SCCounselorCharacter')))
if($rbClasses.Count -ne 1){throw 'Counselor class ambiguous'};$rbClass=$rbClasses[0]
if($rbImage.Hex($rbClass.ConstructorRva,10) -ne '4C-8B-DC-48-83-EC-78-48-8B-05'){throw 'Constructor prefix differs'}
$rbCtor=[uint32]([long]$rbClass.ConstructorRva+14+$rbImage.I32($rbClass.ConstructorRva+10))
$rbFunctions=[CfgFunctionFactoryReader]::Inspect($rbBytes,$rbImage,$rbFunctionHelper)
$rbHeader=@('#pragma once','#include "../RuntimeProbe/VehicleBindings.generated.hpp"','namespace cfgrbemote::disk {')
foreach($rbSpec in @(@('OnHideEmoteSelect',0x220B3B0,0x4C96E0),@('OnRequestShowEmoteSelect',0x220B490,0x4C9A20))){
 $rbFn=@($rbFunctions|Where-Object {$_.Name -eq $rbSpec[0] -and $_.OwnerFactoryRva -eq $rbClass.Factory.FactoryRva})
 if($rbFn.Count -ne 1 -or $rbFn[0].ParameterBytes -ne 0 -or $rbFn[0].ParameterCount -ne 0){throw 'RB action zero-parameter schema differs'};$rbFn=$rbFn[0]
 if($rbImage.Ascii($rbImage.FromVa($rbImage.U64($rbSpec[1]))) -ne $rbSpec[0] -or $rbImage.FromVa($rbImage.U64($rbSpec[1]+8)) -ne $rbSpec[2]){throw 'RB native registration differs'}
 # Both are disassembled 26-byte leaf exec thunks without unwind entries:
 # advance empty FFrame, then tailcall the native action. No guessed extent.
 $rbLength=26
 $rbTail=if($rbSpec[0] -eq 'OnHideEmoteSelect'){'46-03-F2-FF'}else{'06-3D-F2-FF'}
 if($rbImage.Hex($rbSpec[2],26) -ne ('48-8B-42-20-45-33-C0-48-85-C0-41-0F-95-C0-4C-03-C0-4C-89-42-20-E9-'+$rbTail) -or $rbImage.Hex($rbSpec[2]+26,6) -ne 'CC-CC-CC-CC-CC-CC'){throw 'RB leaf executor differs'}
 $rbCode=($rbImage.Hex($rbFn.FactoryRva,47).Split('-')|ForEach-Object {'0x'+$_})-join ','
 $rbHeader+=('constexpr cfgvehicle::disk::Binding {0}{{0x{1:X},0x{2:X},0x{3:X},0x{4:X},0,0,{{{{{5}}}}}}};' -f $rbSpec[0],$rbFn.FactoryRva,$rbFn.CacheRva,$rbClass.Factory.CacheRva,$rbCtor,$rbCode)
 $rbHeader+=('constexpr std::uintptr_t {0}Executor=0x{1:X};' -f $rbSpec[0],$rbSpec[2])
 $rbHeader+=('constexpr std::array<unsigned char,{0}> {1}Code{{{{{2}}}}};' -f $rbLength,$rbSpec[0],(($rbImage.Hex($rbSpec[2],$rbLength).Split('-')|ForEach-Object {'0x'+$_})-join ','))
}
$rbHeader+='}';$rbHeader|Set-Content -LiteralPath (Join-Path $PSScriptRoot 'RBEmoteBindings.generated.hpp') -Encoding utf8
'CFG RB emote native registration, zero-frame schemas and code pins PASS; no game access.'
