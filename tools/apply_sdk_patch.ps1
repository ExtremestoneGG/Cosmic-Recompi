param([string]$Sdk)
$ErrorActionPreference = 'Stop'
$root = Split-Path $PSScriptRoot
if (!$Sdk) { $Sdk = Join-Path $root 'third_party\rexglue-sdk' }
$Sdk = [IO.Path]::GetFullPath($Sdk)
$patch = Join-Path $root 'sdk-patches\rex-sdk-local.diff'
if (!(Test-Path -LiteralPath $patch)) { throw 'Diff do SDK ausente.' }
& git -C $Sdk apply --check $patch
if ($LASTEXITCODE -ne 0) { throw 'O SDK nao corresponde a base do diff, ou o patch ja foi aplicado.' }
& git -C $Sdk apply $patch
if ($LASTEXITCODE -ne 0) { throw 'Falha ao aplicar o diff.' }
$prefix = ($root -replace '\\','/') + '/native-rex/src/'
foreach ($line in [IO.File]::ReadAllLines($patch)) {
    if (!$line.StartsWith('+++ b/')) { continue }
    $relative = $line.Substring(6)
    $path = [IO.Path]::GetFullPath((Join-Path $Sdk $relative))
    if (!$path.StartsWith($Sdk + [IO.Path]::DirectorySeparatorChar, [StringComparison]::OrdinalIgnoreCase)) { throw 'Caminho invalido no diff.' }
    if (!(Test-Path -LiteralPath $path -PathType Leaf)) { continue }
    $text = [IO.File]::ReadAllText($path)
    if ($text.Contains('D:/Ben 10 recomp/native-rex/src/')) {
        [IO.File]::WriteAllText($path,$text.Replace('D:/Ben 10 recomp/native-rex/src/',$prefix),[Text.UTF8Encoding]::new($false))
    }
}
Write-Output 'Diff aplicado e includes de integracao ajustados para este checkout.'
