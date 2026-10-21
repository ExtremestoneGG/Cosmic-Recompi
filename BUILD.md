# Desenvolvimento e reprodução

## Organização

- `launcher/`: launcher Windows Forms e leitor/importador XDVDFS, sem bibliotecas externas.
- `native-rex/src/`: integração do título com ReXGlue, controle de cadência, overlay e ferramentas de diagnóstico.
- `native-rex/ben10_manifest.toml`: manifesto de codegen e limites de funções corrigidos.
- `sdk-patches/rex-sdk-local.diff`: alterações locais do SDK em um único diff.
- `native-rex/generated/`: somente integração CMake. As fontes PPC de `generated/default` são produzidas localmente e devem permanecer ignoradas.
- `tests/`: testes sintéticos do importador; não contêm arquivos do jogo.

O SDK utilizado nesta primeira build corresponde à série 0.10.0, fixado no commit `c94f5ebdcb3c9d1a460ca48e04f9758448f8d518`. O título da janela do SDK pode exibir `v0.10.0-dev.unknown`; não interpretar esse texto como identificação precisa do código-fonte. O commit e o diff identificam a base efetiva.

## Launcher

Em Windows PowerShell, execute `tools/build_launcher.ps1`. Usa o compilador C# do .NET Framework já presente no Windows; grava os resultados em `build/launcher`. Não instala ferramentas.

O launcher procura `runtime/ben10.exe`, `runtime/rexruntime.dll`, `runtime/rexgpu-xenos.dll` e dependências declaradas em `runtime/SHA256SUMS.txt`. O manifesto verifica integridade de cópia; não é uma assinatura. Para testar DLLs do SDK modificadas, recalcule o manifesto local, preservando nomes e o formato `sha256`, dois espaços, nome do arquivo.

## Executável nativo

Requisitos de compilação: CMake, Ninja, Clang/LLVM com suporte MSVC, Windows SDK, bibliotecas do MSVC e o SDK ReXGlue. Essas ferramentas não acompanham o launcher e precisam ser obtidas de suas fontes oficiais.

1. Obtenha o SDK ReXGlue no commit acima, com seus submódulos nas revisões registradas por esse commit.
2. Aplique `sdk-patches/rex-sdk-local.diff` na raiz do SDK. Os includes de diagnóstico do diff original apontam para `D:/Ben 10 recomp/native-rex/src`; ajuste esse prefixo para sua pasta de checkout se necessário.
3. Compile e instale o SDK conforme sua documentação, usando configuração Release e suporte a Direct3D 12. As modificações do SDK não incluem recursos do jogo.
4. Importe sua ISO com o launcher ou disponibilize seus recursos locais em `originals/`. O XEX precisa ser da edição suportada.
5. Execute `rexglue codegen native-rex/ben10_manifest.toml`. Isso cria `native-rex/generated/default` localmente.
6. Configure o projeto `native-rex` com CMake, indicando a instalação do SDK em `CMAKE_PREFIX_PATH`, e compile em Release. A integração gerada deve corresponder ao codegen do mesmo SDK.

### Sequência de referência

Execute num terminal de desenvolvimento x64 com Git, CMake, Ninja, LLVM/Clang para ABI MSVC e os includes/libs do Windows SDK disponíveis. Exemplo de checkout no D:; estes comandos descrevem o fluxo do SDK, mas **não foram executados de ponta a ponta numa máquina limpa** nesta release:

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
