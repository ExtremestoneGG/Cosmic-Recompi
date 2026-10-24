[English](#english) | [PortuguÃªs (Brasil)](#portugues)

<a id="english"></a>

# Building Cosmic Recomp

## Source layout and SDK identity

`launcher/native/` contains the standalone Dear ImGui/SDL3/OpenGL frontend. `launcher/NativeBridge.cs`, `InstalledGame.cs`, `IsoImage.cs` and `GameMenuPatch.cs` implement its process bridge, importer and local menu transformation. Legacy Windows Forms sources are retained for reference, not used by the new frontend. `native-rex/src/` contains title integration, frame pacing, diagnostics and original-game video menu callbacks. Generated game C++ belongs only in ignored `native-rex/generated/default`.

The SDK is pinned to commit `c94f5ebdcb3c9d1a460ca48e04f9758448f8d518` in the 0.10.0 series. A window title such as `v0.10.0-dev.unknown` is not a precise source revision; use the commit and patch.

## Required tools

Use a Windows x64 developer terminal with Git, CMake, Ninja, LLVM/Clang targeting the MSVC ABI, and the Microsoft C++/Windows SDK headers and libraries. Obtain these tools from their upstream providers listed in SOURCES.md; they are not bundled with the launcher.

## Reference native-build sequence

The commands below describe the SDK workflow. A complete clean-machine native build has **not** been verified for this test release. Local development used an existing portable toolchain; do not treat its private benchmark scripts as a universal SDK installer.

```powershell
$root = (Get-Location).Path
git clone https://github.com/rexglue/rexglue-sdk.git third_party/rexglue-sdk
git -C third_party/rexglue-sdk checkout c94f5ebdcb3c9d1a460ca48e04f9758448f8d518
git -C third_party/rexglue-sdk submodule update --init --recursive
./tools/apply_sdk_patch.ps1

Push-Location third_party/rexglue-sdk
cmake --preset win-amd64 "-DCMAKE_INSTALL_PREFIX=$root/toolchain/rexglue-sdk"
cmake --build --preset win-amd64-release --parallel 2
cmake --install out/build/win-amd64 --config Release
Pop-Location

# Place assets from your own supported copy in originals/ first.
& "$root/toolchain/rexglue-sdk/bin/rexglue.exe" codegen native-rex/ben10_manifest.toml
python tools/apply_game_integration.py --generated native-rex/generated/default
cmake -S native-rex -B build/native -G Ninja -DCMAKE_BUILD_TYPE=Release `
  -DCMAKE_CXX_COMPILER=clang-cl "-DCMAKE_PREFIX_PATH=$root/toolchain/rexglue-sdk"
cmake --build build/native --parallel 2
./tools/build_launcher.ps1
```

The patch helper runs `git apply --check` and relocates diagnostic include paths to your checkout. Do not apply the patch to a different SDK revision without reviewing conflicts. SDK build details are in `CMakePresets.json` and `thirdparty/CMakeLists.txt`, also preserved in the corresponding-source archive.

## Building from the complete SDK source archive

Use `CosmicRecomp-test-0.2.1-sdk-source-fix1.zip` when rebuilding the published SDK. It supersedes the earlier 0.2.1 SDK source archive, which omitted the project-owned direct-resolve bytecode header and retained an include into a private research directory. The shader bytes and published game binaries are unchanged.

After extracting the archive, run this from its root:

```powershell
python prepare_sdk_source.py --sdk rexglue-sdk --consumer consumer
```

The helper validates and relocates the SDK's diagnostic includes to the consumer headers bundled in that archive. The SDK patch is **already applied** in the archive; do not apply it again. For a pristine Git checkout, use the normal patch sequence above instead. Include closure and relocation were checked; a complete clean-machine build remains unverified.

## Native frontend and bridge

Check out the UI dependency before configuring the frontend:

```powershell
git clone https://github.com/ExtremestoneGG/recomp-ui.git third_party/full-emerald-ui
git -C third_party/full-emerald-ui checkout 047bc82d8679e96d1dec19ce2cd8ab677003465f
cmake -S launcher/native -B build/frontend -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build/frontend --parallel 2
$root = (Get-Location).Path
$csc = "$env:WINDIR/Microsoft.NET/Framework64/v4.0.30319/csc.exe"
& $csc /nologo /optimize+ /target:winexe /platform:x64 /reference:System.Windows.Forms.dll `
  "/out:$root/build/frontend/CosmicBridge.exe" `
  "$root/launcher/NativeBridge.cs" "$root/launcher/InstalledGame.cs" `
  "$root/launcher/IsoImage.cs" "$root/launcher/GameMenuPatch.cs"
```

Use an x64 MSVC developer environment. CMake builds a separate SDL3 copy with OpenGL enabled; the game SDK's SDL configuration is not changed. Keep the dependency's MIT/ImGui/stb notices and SDL license with redistributions. A GIF background is optional and not provided.

The bridge requires the matching native executable and `runtime/pc-menu-api.txt` containing `1`, plus the runtime hash manifest. Do not mix this frontend with the first test release's older native executable.

## Launcher and importer tests

`tools/build_launcher.ps1` configures the native frontend with CMake and builds the C# bridge with the Windows .NET Framework compiler, writing to `build/launcher`. It requires the pinned UI and SDL checkouts and an x64 C++ developer environment. It creates a project-local temporary directory and does not install tools. The native frontend and bridge were built locally; a clean-machine full-project build remains unverified.

```powershell
$root = (Get-Location).Path
$csc = "$env:WINDIR/Microsoft.NET/Framework64/v4.0.30319/csc.exe"
New-Item -ItemType Directory -Force build/launcher | Out-Null
& $csc /nologo /target:exe /platform:x64 "/out:$root\build\launcher\IsoReaderTests.exe" `
  "$root\launcher\IsoImage.cs" "$root\tests\IsoReaderTests.cs"
./build/launcher/IsoReaderTests.exe "$root/build/launcher/parser-fixtures"
```

These tests use small synthetic images. `tools/build_iso_import_core.ps1` builds `InspectIso.exe`: pass an ISO path to inspect its supported edition; add `--extract <new-folder>` for full extraction without the GUI. Never select an existing folder or your saves as an extraction destination.

## Runtime integrity and modification

The launcher expects `runtime/ben10.exe`, `rexruntime.dll`, `rexgpu-xenos.dll`, and dependencies listed in `runtime/SHA256SUMS.txt`. This manifest detects incomplete or changed copies; it is not a digital signature. Developers replacing SDK DLLs may recalculate its hashes using the same filenames and `sha256`, two spaces, filename format.

`BEN10_PORTABLE_ROOT` separates original assets, user saves, cache, metadata, and logs from the development checkout. Never commit assets, generated game sources, saves, or caches. Build release ZIPs from an explicit allowlist, never recursively from an installed test directory.

---

<a id="portugues"></a>

# Desenvolvimento e reproduÃ§Ã£o

## OrganizaÃ§Ã£o

- `launcher/native/`: frontend Dear ImGui/SDL3/OpenGL; `launcher/`: ponte C# e importador XDVDFS. O antigo host Windows Forms fica apenas como referÃªncia.
- `native-rex/src/`: integraÃ§Ã£o do tÃ­tulo com ReXGlue, controle de cadÃªncia, overlay e ferramentas de diagnÃ³stico.
- `native-rex/ben10_manifest.toml`: manifesto de codegen e limites de funÃ§Ãµes corrigidos.
- `sdk-patches/rex-sdk-local.diff`: alteraÃ§Ãµes locais do SDK em um Ãºnico diff.
- `native-rex/generated/`: somente integraÃ§Ã£o CMake. As fontes PPC de `generated/default` sÃ£o produzidas localmente e devem permanecer ignoradas.
- `tests/`: testes sintÃ©ticos do importador; nÃ£o contÃªm arquivos do jogo.

O SDK utilizado nesta primeira build corresponde Ã  sÃ©rie 0.10.0, fixado no commit `c94f5ebdcb3c9d1a460ca48e04f9758448f8d518`. O tÃ­tulo da janela do SDK pode exibir `v0.10.0-dev.unknown`; nÃ£o interpretar esse texto como identificaÃ§Ã£o precisa do cÃ³digo-fonte. O commit e o diff identificam a base efetiva.

## Launcher

Obtenha o fork recomp-ui no commit indicado na seÃ§Ã£o em inglÃªs e os submÃ³dulos do SDK. Num terminal de desenvolvimento C++ x64, execute `tools/build_launcher.ps1`: CMake compila o frontend nativo, e o compilador C# do .NET Framework compila a ponte. Os resultados ficam em `build/launcher`; ferramentas nÃ£o sÃ£o instaladas. O SDL da interface Ã© compilado separadamente, com OpenGL, sem alterar o SDL do jogo.

O launcher procura `runtime/ben10.exe`, `runtime/rexruntime.dll`, `runtime/rexgpu-xenos.dll` e dependÃªncias declaradas em `runtime/SHA256SUMS.txt`. O manifesto verifica integridade de cÃ³pia; nÃ£o Ã© uma assinatura. Para testar DLLs do SDK modificadas, recalcule o manifesto local, preservando nomes e o formato `sha256`, dois espaÃ§os, nome do arquivo.

## ExecutÃ¡vel nativo

Requisitos de compilaÃ§Ã£o: CMake, Ninja, Clang/LLVM com suporte MSVC, Windows SDK, bibliotecas do MSVC e o SDK ReXGlue. Essas ferramentas nÃ£o acompanham o launcher e precisam ser obtidas de suas fontes oficiais.

1. Obtenha o SDK ReXGlue no commit acima, com seus submÃ³dulos nas revisÃµes registradas por esse commit.
2. Aplique `sdk-patches/rex-sdk-local.diff` na raiz do SDK. O script tools/apply_sdk_patch.ps1 ajusta os includes de diagnÃ³stico do diff para a pasta do checkout atual.
3. Compile e instale o SDK conforme sua documentaÃ§Ã£o, usando configuraÃ§Ã£o Release e suporte a Direct3D 12. As modificaÃ§Ãµes do SDK nÃ£o incluem recursos do jogo.
4. Importe sua ISO com o launcher ou disponibilize seus recursos locais em `originals/`. O XEX precisa ser da ediÃ§Ã£o suportada.
5. Execute `rexglue codegen native-rex/ben10_manifest.toml`. Isso cria `native-rex/generated/default` localmente.
6. Configure o projeto `native-rex` com CMake, indicando a instalaÃ§Ã£o do SDK em `CMAKE_PREFIX_PATH`, e compile em Release. A integraÃ§Ã£o gerada deve corresponder ao codegen do mesmo SDK.

### SequÃªncia de referÃªncia

Execute num terminal de desenvolvimento x64 com Git, CMake, Ninja, LLVM/Clang para ABI MSVC e os includes/libs do Windows SDK disponÃ­veis. Exemplo de checkout em uma pasta de sua escolha; estes comandos descrevem o fluxo do SDK, mas **nÃ£o foram executados de ponta a ponta numa mÃ¡quina limpa** nesta release:

```powershell
$root = (Get-Location).Path
git clone https://github.com/rexglue/rexglue-sdk.git third_party/rexglue-sdk
git -C third_party/rexglue-sdk checkout c94f5ebdcb3c9d1a460ca48e04f9758448f8d518
git -C third_party/rexglue-sdk submodule update --init --recursive
./tools/apply_sdk_patch.ps1

Push-Location third_party/rexglue-sdk
cmake --preset win-amd64 "-DCMAKE_INSTALL_PREFIX=$root/toolchain/rexglue-sdk"
cmake --build --preset win-amd64-release --parallel 2
cmake --install out/build/win-amd64 --config Release
Pop-Location

# Disponibilize os recursos da sua cÃ³pia em originals/ antes do codegen.
& "$root/toolchain/rexglue-sdk/bin/rexglue.exe" codegen native-rex/ben10_manifest.toml
cmake -S native-rex -B build/native -G Ninja -DCMAKE_BUILD_TYPE=Release `
  -DCMAKE_CXX_COMPILER=clang-cl "-DCMAKE_PREFIX_PATH=$root/toolchain/rexglue-sdk"
cmake --build build/native --parallel 2
./tools/build_launcher.ps1
```

O script de patch usa `git apply --check` antes de alterar o SDK e ajusta os includes de integraÃ§Ã£o para o checkout atual. NÃ£o aplique o patch sobre outra revisÃ£o do SDK sem revisar os conflitos. As configuraÃ§Ãµes e dependÃªncias detalhadas do SDK estÃ£o no seu `CMakePresets.json` e `thirdparty/CMakeLists.txt`, incluÃ­dos no arquivo de fontes correspondente.

### Testar o importador

```powershell
$csc = "$env:WINDIR/Microsoft.NET/Framework64/v4.0.30319/csc.exe"
New-Item -ItemType Directory -Force build/launcher | Out-Null
& $csc /nologo /target:exe /platform:x64 "/out:$root\build\launcher\IsoReaderTests.exe" `
  "$root\launcher\IsoImage.cs" "$root\tests\IsoReaderTests.cs"
./build/launcher/IsoReaderTests.exe "$root/build/launcher/parser-fixtures"
```

Esses testes criam somente imagens sintÃ©ticas pequenas no diretÃ³rio fornecido. O teste com a ISO real Ã© separado: `tools/build_iso_import_core.ps1` gera `InspectIso.exe`, que aceita o caminho da ISO para inspecionar a ediÃ§Ã£o. A opÃ§Ã£o `--extract <pasta-nova>` executa uma importaÃ§Ã£o completa sem interface grÃ¡fica. NÃ£o aponte a extraÃ§Ã£o para uma pasta existente ou para seus saves.

O pacote portÃ¡til fornece `BEN10_PORTABLE_ROOT` para separar `game`, `user`, `cache`, `metadata` e logs da pasta de desenvolvimento. NÃ£o copiar os caminhos absolutos dos scripts privados de benchmark para a configuraÃ§Ã£o de uma distribuiÃ§Ã£o.

Os scripts histÃ³ricos de compilaÃ§Ã£o do ambiente de desenvolvimento usam caminhos locais e nÃ£o constituem um instalador universal de toolchain. NÃ£o foi verificada uma compilaÃ§Ã£o em mÃ¡quina limpa nesta primeira versÃ£o; o pacote binÃ¡rio e o importador foram testados no ambiente de desenvolvimento.

Nunca envie ao Git arquivos de `originals`, `installation`, fontes PPC geradas, caches ou saves. NÃ£o use um ZIP recursivo da pasta de testes para montar uma release: a distribuiÃ§Ã£o precisa de uma lista explÃ­cita de arquivos permitidos.

Para recompilar a partir do arquivo completo de fontes do SDK, use `CosmicRecomp-test-0.2.1-sdk-source-fix1.zip`. Ele corrige um header de shader prÃ³prio ausente no pacote anterior, sem alterar os bytes do shader ou os binÃ¡rios do jogo. ApÃ³s extrair, execute `python prepare_sdk_source.py --sdk rexglue-sdk --consumer consumer` na raiz do arquivo extraÃ­do. O diff jÃ¡ estÃ¡ aplicado nesse pacote. Os includes foram verificados; uma compilaÃ§Ã£o completa em mÃ¡quina limpa continua sem verificaÃ§Ã£o.


## Public test profile, v0.3.0-test.1

The public bridge is compiled with `COSMIC_DISABLE_RATH`; the authored program is generated with `authored(disable_rath=True)`. Run `python tools/build_release_bridge.py` to produce `build/rath-disabled-release/CosmicBridge.exe` and its embedded authored-only program. `tools/build_launcher.ps1` builds the standalone frontend and copies that bridge. Do not substitute a private Rath-enabled bridge. The runtime API marker is `3`.

After codegen, `tools/apply_game_integration.py --generated native-rex/generated/default` applies two idempotent title changes: a 64 MiB DB File Heap request and an RT filter at the original game input wrapper. It contains no generated game function bodies and modifies only locally generated files. No SDK kernel/renderer replacement is involved. The failed animation-lookup experiment is excluded. Run the exclusion-policy and installer tests before packaging. The repository must not receive the generated directory or the embedded user's scripts.

SDK/GPU DLLs retain the accepted v0.2.1 hashes recorded in PROVENANCE.json. Their complete corresponding source is provided with the release; it includes the pinned SDK and dependencies plus the matching local patch. The v0.3 consumer snapshot replaces its consumer sources without silently changing SDK files. A full clean-machine rebuild remains unverified.

### Português

O perfil público compila a ponte com `COSMIC_DISABLE_RATH`, gera somente o programa próprio e exclui o Rath. Execute `python tools/build_release_bridge.py`; `tools/build_launcher.ps1` usa essa ponte. Depois do codegen, execute apply_game_integration.py para aplicar as duas alterações próprias ao código gerado localmente. O código gerado e os dados do usuário nunca entram no repositório. Os fontes completos do SDK acompanham a versão, com os hashes dos DLLs registrados. A compilação completa em máquina limpa continua sem verificação.
