param(
    [string]$Python = 'python',
    [string]$SdkPrefix,
    [int]$Jobs = 2,
    [switch]$Regenerate
)
$ErrorActionPreference = 'Stop'
$root = Split-Path $PSScriptRoot
if (!$SdkPrefix) { $SdkPrefix = Join-Path $root 'toolchain/rexglue-sdk' }
$SdkPrefix = [IO.Path]::GetFullPath($SdkPrefix)
$env:TEMP = Join-Path $root 'data/temp'
$env:TMP = $env:TEMP
$env:PYTHONDONTWRITEBYTECODE = '1'
[IO.Directory]::CreateDirectory($env:TEMP) | Out-Null
Push-Location $root
try {
    $codegen = Join-Path $SdkPrefix 'bin/rexglue.exe'
    if (!(Test-Path -LiteralPath $codegen)) { throw 'Install the pinned patched SDK first. See docs/CONTRIBUTOR_BUILD_RUNBOOK.md.' }
    if ($Regenerate) {
        & $codegen codegen native-rex/ben10_manifest.toml
        if ($LASTEXITCODE -ne 0) { throw 'Code generation failed; stop here.' }
    }
    if (!(Test-Path -LiteralPath 'native-rex/generated/default/ben10_init.h')) {
        throw 'Generated sources absent. Extract your supported ISO to originals, then rerun with -Regenerate.'
    }
    & $Python -B tools/apply_game_integration.py --generated native-rex/generated/default
    if ($LASTEXITCODE -ne 0) { throw 'Game integration failed; do not stage an older executable.' }
    & cmake -S native-rex -B build/native -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_COMPILER=clang-cl "-DCMAKE_PREFIX_PATH=$SdkPrefix" "-Drexglue_DIR=$SdkPrefix/lib/cmake/rexglue"
    if ($LASTEXITCODE -ne 0) { throw 'Consumer configuration failed.' }
    & cmake --build build/native --parallel $Jobs
    if ($LASTEXITCODE -ne 0) { throw 'Consumer compilation failed.' }
    & "$PSScriptRoot/build_launcher.ps1" -Python $Python
    & $Python -B tools/stage_development.py --native build/native --frontend build/launcher --output build/development-app
    if ($LASTEXITCODE -ne 0) { throw 'Development staging failed.' }
    Write-Output 'Run build/development-app/CosmicRecomp.exe. This is a local development app, not a published installer.'
} finally { Pop-Location }
