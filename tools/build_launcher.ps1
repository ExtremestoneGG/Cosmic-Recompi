$ErrorActionPreference = 'Stop'
$root = Split-Path $PSScriptRoot
$out = Join-Path $root 'build\launcher'
$env:TEMP = Join-Path $root 'data\temp'
$env:TMP = $env:TEMP
[IO.Directory]::CreateDirectory($env:TEMP) | Out-Null
[IO.Directory]::CreateDirectory($out) | Out-Null
$compiler = Join-Path $env:WINDIR 'Microsoft.NET\Framework64\v4.0.30319\csc.exe'
if (-not (Test-Path -LiteralPath "$root\third_party\full-emerald-ui\src\common\launcher_gl.c")) {
    throw 'Check out the pinned recomp-ui dependency first; see BUILD.md.'
}
& cmake -S "$root\launcher\native" -B $out -G Ninja -DCMAKE_BUILD_TYPE=Release
if ($LASTEXITCODE -ne 0) { throw 'Frontend configuration failed.' }
& cmake --build $out --parallel 2
if ($LASTEXITCODE -ne 0) { throw 'Frontend build failed.' }
& python "$root\tools\build_release_bridge.py"
if ($LASTEXITCODE -ne 0) { throw 'Corrected release bridge build failed.' }
Copy-Item -LiteralPath "$root\build\settings-release\CosmicBridge.exe" -Destination "$out\CosmicBridge.exe" -Force
if ($LASTEXITCODE -ne 0) { throw 'Process bridge build failed.' }
Write-Output "Built CosmicRecomp.exe and CosmicBridge.exe in $out"
