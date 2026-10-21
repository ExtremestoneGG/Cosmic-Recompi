# Origem dos componentes

Este documento distingue dependências efetivamente usadas, ferramentas do ambiente e referências de arquitetura. `PROVENANCE.json` fixa as revisões e hashes; `THIRD_PARTY.md` e a pasta `licenses` da release preservam os termos originais.

## Código próprio

O launcher Windows Forms, a interface de importação, a verificação da edição da ISO, a organização portátil e os testes sintéticos foram implementados neste projeto. As integrações do título, correções de limites de callbacks, cadência e overlay ficam em `native-rex/src` e no manifesto. Não são um novo kernel ou renderizador.

## Base e linhagem

A base direta é [ReXGlue SDK](https://github.com/rexglue/rexglue-sdk), commit `c94f5ebdcb3c9d1a460ca48e04f9758448f8d518`. As alterações locais estão num único diff em `sdk-patches/rex-sdk-local.diff`. O próprio SDK documenta sua origem em [Xenia](https://github.com/xenia-project/xenia) e influências de [XenonRecomp](https://github.com/hedge-dev/XenonRecomp) e [rexdex/recompiler](https://github.com/rexdex/recompiler). Esses créditos de linhagem não significam que todos esses projetos sejam dependências independentes adicionais do launcher.

## Submódulos efetivamente presentes no SDK utilizado

As URLs abaixo vêm dos remotes dos checkouts locais e as revisões vêm dos submódulos usados no build; não são versões escolhidas por memória ou presumidas como a mais recente. As licenças detalhadas estão preservadas nas fontes e no pacote de notices.

| Componente/caminho | Origem | Revisão |
| --- | --- | --- |
| `thirdparty/FFmpeg` | https://github.com/wmarti/FFmpeg.git | `0604b464c7cb4ebc94940cf1f324a3b26b87717c` |
| `thirdparty/cli11` | https://github.com/CLIUtils/CLI11 | `bfffd37e1f804ca4fae1caae106935791696b6a9` |
| `thirdparty/fmt` | https://github.com/fmtlib/fmt.git | `407c905e45ad75fc29bf0f9bb7c5c2fd3475976f` |
| `thirdparty/glslang` | https://github.com/KhronosGroup/glslang.git | `f4f1d8a352ca1908943aea2ad8c54b39b4879080` |
| `thirdparty/imgui` | https://github.com/ocornut/imgui.git | `6d910d5487d11ca567b61c7824b0c78c569d62f0` |
| `thirdparty/inja` | https://github.com/pantor/inja.git | `7d1b4600b68595085a949743331c2e5673f511ea` |
| `thirdparty/libmspack` | https://github.com/kyz/libmspack.git | `305907723a4e7ab2018e58040059ffb5e77db837` |
| `thirdparty/o1heap` | https://github.com/pavel-kirienko/o1heap.git | `388a73fd9007300e5130c5fe352d9ce3288b6dde` |
| `thirdparty/sdl3` | https://github.com/libsdl-org/SDL | `8bf3b7215ad9fc3deb583c6a3a37c6c67f2e24e4` |
| `thirdparty/simde` | https://github.com/simd-everywhere/simde.git | `71fd833d9666141edcd1d3c109a80e228303d8d7` |
| `thirdparty/spdlog` | https://github.com/gabime/spdlog.git | `79524ddd08a4ec981b7fea76afd08ee05f83755d` |
| `thirdparty/spirv-headers` | https://github.com/KhronosGroup/SPIRV-Headers.git | `29981f65241605e08b0ede4cfeb999fe3b723c6a` |
| `thirdparty/spirv-tools` | https://github.com/KhronosGroup/SPIRV-Tools.git | `9a49b0883b9b635689a85b5647dbfcb223268151` |
| `thirdparty/tomlplusplus` | https://github.com/marzer/tomlplusplus.git | `30172438cee64926dc41fdd9c11fb3ba5b2ba9de` |
| `thirdparty/utfcpp` | https://github.com/nemtrif/utfcpp.git | `63d64de49fd6b829f7c8694df5ab2ee625cb7134` |
| `thirdparty/vulkan-headers` | https://github.com/KhronosGroup/Vulkan-Headers.git | `e3b1eec08173d6b825cd3ac88c885a63b621504a` |
| `thirdparty/vulkan-memory-allocator` | https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator.git | `1d8f600fd424278486eade7ed3e877c99f0846b1` |
| `thirdparty/xxHash` | https://github.com/Cyan4973/xxHash.git | `e626a72bc2321cd320e953a0ccf1584cad60f363` |

O SDK também contém código vendorizado fora desses submódulos (por exemplo crypto, disasm, dxbc, pe, stb e cabeçalhos auxiliares). A revisão desse código é a do próprio SDK; seus créditos/licenças por arquivo são mantidos no arquivo de fontes. Não atribuímos a esses diretórios um repositório ou commit externo que não foi registrado separadamente. Nem todo submódulo do SDK é necessariamente utilizado no caminho Direct3D 12 desta build.

## Ferramentas do ambiente de desenvolvimento

| Ferramenta | Origem | Identificação utilizada | Distribuída no ZIP do jogo? |
| --- | --- | --- | --- |
| LLVM/Clang/LLD | [LLVM](https://github.com/llvm/llvm-project) via [llvm-mingw](https://github.com/mstorsjo/llvm-mingw) | pacote `llvm-mingw-20261006-ucrt-x86_64`; Clang 23.1.3, commit LLVM `0d261d1ca552c95a8f007e061c787ac7132fbcbc` | Não |
| MSVC headers/libs e CRT | [Microsoft Visual Studio](https://visualstudio.microsoft.com/downloads/) | MSVC 14.51.36231 | Somente DLLs CRT de execução declarados no manifesto |
| Windows SDK | [Microsoft Windows SDK](https://developer.microsoft.com/windows/downloads/windows-sdk/) | 10.0.28000.0 | Não |
| Compilador do launcher | .NET Framework do Windows/Microsoft | C#/.NET Framework 4.8.9221.0, plataforma x64 | Não; o Windows fornece o framework |
| CMake | [Kitware CMake](https://cmake.org/) | 4.4.4 | Não |
| Ninja | [Ninja](https://github.com/ninja-build/ninja) | 1.13.2.git.kitware.jobserver-pipe-1 | Não |

No ambiente original, os payloads MSVC/SDK foram obtidos dos manifestos Microsoft com auxílio de `portable-msvc.py`, da família do [downloader de mmozeiko](https://gist.github.com/mmozeiko/7f3162ec2988e81e56d5c4e22cde9977), e extração local dos MSIs/CABs. O downloader não acompanha a distribuição nem é necessário para seguir o fluxo com ferramentas Microsoft instaladas normalmente. A revisão externa exata desse script não foi preservada; não alegamos correspondência byte a byte com a versão atual do gist.

SHA-256 da cópia local utilizada do downloader: `1791cccca594ff083ce8d3e0a139e6156d2c312e24332a909e608010f0230a51`.

## Recursos do jogo e evidência

Os recursos vêm exclusivamente da ISO fornecida pelo próprio usuário. O SHA-256 suportado do XEX identifica a edição; não é um recurso redistribuído. Os hashes do runtime publicado estão em `PROVENANCE.json` e `runtime/SHA256SUMS.txt`. Testes e limites conhecidos estão em `ARCHITECTURE.md`; resultados do checkpoint inicial não foram generalizados para a campanha inteira.
