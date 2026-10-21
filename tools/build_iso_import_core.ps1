$ErrorActionPreference = 'Stop'
$root = Split-Path $PSScriptRoot
$out = Join-Path $root 'build\launcher'
$env:TEMP = Join-Path $root 'data\temp'
$env:TMP = $env:TEMP
[IO.Directory]::CreateDirectory($env:TEMP) | Out-Null
[IO.Directory]::CreateDirectory($out) | Out-Null
$compiler = Join-Path $env:WINDIR 'Microsoft.NET\Framework64\v4.0.30319\csc.exe'
& $compiler /nologo /optimize+ /target:exe /platform:x64 "/out:$out\InspectIso.exe" "$root\launcher\IsoImage.cs" "$root\launcher\InspectIso.cs"
if ($LASTEXITCODE -ne 0) { throw 'Falha ao compilar o leitor de ISO.' }
