$ErrorActionPreference = 'Stop'
$root = Split-Path $PSScriptRoot
$out = Join-Path $root 'build\launcher'
$env:TEMP = Join-Path $root 'data\temp'
$env:TMP = $env:TEMP
[IO.Directory]::CreateDirectory($env:TEMP) | Out-Null
[IO.Directory]::CreateDirectory($out) | Out-Null
$compiler = Join-Path $env:WINDIR 'Microsoft.NET\Framework64\v4.0.30319\csc.exe'
& $compiler /nologo /optimize+ /target:winexe /platform:x64 /reference:System.Windows.Forms.dll /reference:System.Drawing.dll "/out:$out\CosmicRecompi.exe" "$root\launcher\IsoImage.cs" "$root\launcher\InstalledGame.cs" "$root\launcher\Program.cs"
if ($LASTEXITCODE -ne 0) { throw 'Falha ao compilar Cosmic Recompi.' }
