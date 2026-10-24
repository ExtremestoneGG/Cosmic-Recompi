[English](#english) | [Português (Brasil)](#portugues)

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

Use `CosmicRecomp-v0.4.0-alpha.1-corresponding-source.zip` when rebuilding the published SDK. It supersedes the earlier 0.2.1 SDK source archive, which omitted the project-owned direct-resolve bytecode header and retained an include into a private research directory. The shader bytes and published game binaries are unchanged.

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

# Desenvolvimento e reprodução

## Organização

- `launcher/native/`: frontend Dear ImGui/SDL3/OpenGL; `launcher/`: ponte C# e importador XDVDFS. O antigo host Windows Forms fica apenas como referência.
- `native-rex/src/`: integração do título com ReXGlue, controle de cadência, overlay e ferramentas de diagnóstico.
- `native-rex/ben10_manifest.toml`: manifesto de codegen e limites de funções corrigidos.
- `sdk-patches/rex-sdk-local.diff`: alterações locais do SDK em um único diff.
- `native-rex/generated/`: somente integração CMake. As fontes PPC de `generated/default` são produzidas localmente e devem permanecer ignoradas.
- `tests/`: testes sintéticos do importador; não contêm arquivos do jogo.

O SDK utilizado nesta primeira build corresponde à série 0.10.0, fixado no commit `c94f5ebdcb3c9d1a460ca48e04f9758448f8d518`. O título da janela do SDK pode exibir `v0.10.0-dev.unknown`; não interpretar esse texto como identificação precisa do código-fonte. O commit e o diff identificam a base efetiva.

## Launcher

Obtenha o fork recomp-ui no commit indicado na seção em inglês e os submódulos do SDK. Num terminal de desenvolvimento C++ x64, execute `tools/build_launcher.ps1`: CMake compila o frontend nativo, e o compilador C# do .NET Framework compila a ponte. Os resultados ficam em `build/launcher`; ferramentas não são instaladas. O SDL da interface é compilado separadamente, com OpenGL, sem alterar o SDL do jogo.

O launcher procura `runtime/ben10.exe`, `runtime/rexruntime.dll`, `runtime/rexgpu-xenos.dll` e dependências declaradas em `runtime/SHA256SUMS.txt`. O manifesto verifica integridade de cópia; não é uma assinatura. Para testar DLLs do SDK modificadas, recalcule o manifesto local, preservando nomes e o formato `sha256`, dois espaços, nome do arquivo.

## Executável nativo

Requisitos de compilação: CMake, Ninja, Clang/LLVM com suporte MSVC, Windows SDK, bibliotecas do MSVC e o SDK ReXGlue. Essas ferramentas não acompanham o launcher e precisam ser obtidas de suas fontes oficiais.

1. Obtenha o SDK ReXGlue no commit acima, com seus submódulos nas revisões registradas por esse commit.
2. Aplique `sdk-patches/rex-sdk-local.diff` na raiz do SDK. O script tools/apply_sdk_patch.ps1 ajusta os includes de diagnóstico do diff para a pasta do checkout atual.
3. Compile e instale o SDK conforme sua documentação, usando configuração Release e suporte a Direct3D 12. As modificações do SDK não incluem recursos do jogo.
4. Importe sua ISO com o launcher ou disponibilize seus recursos locais em `originals/`. O XEX precisa ser da edição suportada.
5. Execute `rexglue codegen native-rex/ben10_manifest.toml`. Isso cria `native-rex/generated/default` localmente.
6. Configure o projeto `native-rex` com CMake, indicando a instalação do SDK em `CMAKE_PREFIX_PATH`, e compile em Release. A integração gerada deve corresponder ao codegen do mesmo SDK.

### Sequência de referência

Execute num terminal de desenvolvimento x64 com Git, CMake, Ninja, LLVM/Clang para ABI MSVC e os includes/libs do Windows SDK disponíveis. Exemplo de checkout em uma pasta de sua escolha; estes comandos descrevem o fluxo do SDK, mas **não foram executados de ponta a ponta numa máquina limpa** nesta release:

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

# Disponibilize os recursos da sua cópia em originals/ antes do codegen.
& "$root/toolchain/rexglue-sdk/bin/rexglue.exe" codegen native-rex/ben10_manifest.toml
cmake -S native-rex -B build/native -G Ninja -DCMAKE_BUILD_TYPE=Release `
  -DCMAKE_CXX_COMPILER=clang-cl "-DCMAKE_PREFIX_PATH=$root/toolchain/rexglue-sdk"
cmake --build build/native --parallel 2
./tools/build_launcher.ps1
```

O script de patch usa `git apply --check` antes de alterar o SDK e ajusta os includes de integração para o checkout atual. Não aplique o patch sobre outra revisão do SDK sem revisar os conflitos. As configurações e dependências detalhadas do SDK estão no seu `CMakePresets.json` e `thirdparty/CMakeLists.txt`, incluídos no arquivo de fontes correspondente.

### Testar o importador

```powershell
$csc = "$env:WINDIR/Microsoft.NET/Framework64/v4.0.30319/csc.exe"
New-Item -ItemType Directory -Force build/launcher | Out-Null
& $csc /nologo /target:exe /platform:x64 "/out:$root\build\launcher\IsoReaderTests.exe" `
  "$root\launcher\IsoImage.cs" "$root\tests\IsoReaderTests.cs"
./build/launcher/IsoReaderTests.exe "$root/build/launcher/parser-fixtures"
```

Esses testes criam somente imagens sintéticas pequenas no diretório fornecido. O teste com a ISO real é separado: `tools/build_iso_import_core.ps1` gera `InspectIso.exe`, que aceita o caminho da ISO para inspecionar a edição. A opção `--extract <pasta-nova>` executa uma importação completa sem interface gráfica. Não aponte a extração para uma pasta existente ou para seus saves.

O pacote portátil fornece `BEN10_PORTABLE_ROOT` para separar `game`, `user`, `cache`, `metadata` e logs da pasta de desenvolvimento. Não copiar os caminhos absolutos dos scripts privados de benchmark para a configuração de uma distribuição.

Os scripts históricos de compilação do ambiente de desenvolvimento usam caminhos locais e não constituem um instalador universal de toolchain. Não foi verificada uma compilação em máquina limpa nesta primeira versão; o pacote binário e o importador foram testados no ambiente de desenvolvimento.

Nunca envie ao Git arquivos de `originals`, `installation`, fontes PPC geradas, caches ou saves. Não use um ZIP recursivo da pasta de testes para montar uma release: a distribuição precisa de uma lista explícita de arquivos permitidos.

Para recompilar a partir do arquivo completo de fontes do SDK, use `CosmicRecomp-v0.4.0-alpha.1-corresponding-source.zip`. Ele corrige um header de shader próprio ausente no pacote anterior, sem alterar os bytes do shader ou os binários do jogo. Após extrair, execute `python prepare_sdk_source.py --sdk rexglue-sdk --consumer consumer` na raiz do arquivo extraído. O diff já está aplicado nesse pacote. Os includes foram verificados; uma compilação completa em máquina limpa continua sem verificação.


## Alpha profile, v0.4.0-alpha.1

The public bridge uses `COSMIC_NATIVE_SETTINGS;COSMIC_ALIEN_STREAMS`. Run `python tools/build_release_bridge.py` to produce `build/settings-release/CosmicBridge.exe`; `tools/build_launcher.ps1` copies this matching profile beside the frontend. It embeds only the project's authored VM program, generated with `authored()`. It does not embed original game scripts. `--rath-disabled` reproduces the older exclusion profile and is not the Alpha default.

Use the pinned SDK plus the single `sdk-patches/rex-sdk-local.diff`, including `Ben10ConfigurePresentation` in the SDK SDL window translation unit. In the corresponding-source archive the diff is already applied; do not apply it again. Run `python prepare_sdk_source.py --sdk rexglue-sdk --consumer consumer` to resolve the consumer include placeholders. The presentation addition uses the runtime's SDL instance, adds no data members or virtual methods, and makes no GPU algorithm change. SDK/kernel/GPU source origins and revisions remain in PROVENANCE.json and SOURCES.md.

After local game codegen, `tools/apply_game_integration.py --generated native-rex/generated/default` applies the 64 MiB DB File Heap request and the RT filter in the original input wrapper. These are idempotent title-specific transformations of locally generated code. Generated files must not enter Git or source archives.

The packaged runtime requires `runtime/pc-menu-api.txt` containing `7` and a `runtime/SHA256SUMS.txt` manifest covering every runtime file. Build the game and the SDK together before staging them; never change only the API marker on an older runtime. Build and runtime validation remain separate requirements. A complete clean-machine rebuild has not been verified.

Run `tools/test_all_aliens_patch.py --native-settings`, the importer/menu suites, and the animation-stream parity and save-reset fixtures against disposable copies. Native gameplay, fullscreen and checkpoint tests have a separate scope recorded in VALIDATION.json. Do not publish private input recordings, screenshots containing user data, original assets, generated functions, caches or saves.

### Português

O perfil Alpha usa `COSMIC_NATIVE_SETTINGS;COSMIC_ALIEN_STREAMS`, API nativa 7 e ponte em `build/settings-release`. O diff único do SDK inclui a extensão SDL de apresentação; o arquivo de fontes já vem com ele aplicado. Execute prepare_sdk_source.py para resolver os includes relocáveis e siga os comandos acima. O SDK continua fornecendo kernel, renderizador e áudio. Código gerado do jogo e dados pessoais não entram no repositório. Os testes locais não comprovam uma compilação completa em máquina limpa.
