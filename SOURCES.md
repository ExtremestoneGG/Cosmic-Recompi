[English](#english) | [Português (Brasil)](#portugues)

<a id="english"></a>

# Component provenance

This guide separates direct runtime dependencies, development tools, upstream lineage, and project-owned code. PROVENANCE.json records revisions/hashes; THIRD_PARTY.md and the release notices preserve license terms. No personal saves, logs, Windows account paths, or attachment data are included. Public GitHub repository/account identity and required upstream copyright notices remain public.

## Project-owned code and upstream lineage

The Cosmic-specific frontend host, process bridge, import UI, supported-edition checks, portable folder layout and tests were implemented in this project. The frontend's reused UI dependency is credited below. Title integration, callback-boundary corrections, pacing and overlay live in native-rex/src and the manifest; they do not replace the SDK kernel or GPU renderer.

The direct base is [ReXGlue SDK](https://github.com/rexglue/rexglue-sdk), commit `c94f5ebdcb3c9d1a460ca48e04f9758448f8d518`, plus the single local SDK diff. ReXGlue itself credits [Xenia](https://github.com/xenia-project/xenia), [XenonRecomp](https://github.com/hedge-dev/XenonRecomp), and [rexdex/recompiler](https://github.com/rexdex/recompiler). Lineage credits do not imply each is a separate direct launcher dependency.

## Frontend dependency

The frontend uses [Full Emerald's recomp-ui fork](https://github.com/ExtremestoneGG/recomp-ui), pinned to `047bc82d8679e96d1dec19ce2cd8ab677003465f` (MIT, Matthew Stanley). [Full Emerald](https://github.com/ExtremestoneGG/Full-Emerald) provides the visual reference. Cosmic Recomp compiles Dear ImGui core/backends and `src/common/launcher_gl.c` from this dependency, using its own C++ host. It uses SDL3 from the recorded SDK dependency rather than Full Emerald's SDL2 host. This does not incorporate Pokémon images, audio or game data.

Optional background animation is an external GIF supplied by the user, decoded with the upstream loader. No background recording is included. Segoe UI is loaded from Windows; the font is not redistributed.

## Recorded SDK submodules

Origins below are the local checkouts' upstream remotes, and revisions are those recorded for the SDK build. They are not guessed latest versions. Not every SDK submodule is necessarily exercised by this Direct3D 12 build.

| Component/path | Origin | Revision |
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


The SDK also vendors code outside these submodules, including crypto, disasm, dxbc, pe, stb, and auxiliary headers. Their recorded revision is the SDK commit; per-file notices are preserved in the complete source archive. We do not invent independent external revisions where none was separately recorded.

## Development toolchain

| Tool | Upstream | Recorded version | Included in the game ZIP? |
| --- | --- | --- | --- |
| LLVM/Clang/LLD via llvm-mingw | [LLVM](https://github.com/llvm/llvm-project), [llvm-mingw](https://github.com/mstorsjo/llvm-mingw) | Package 20261006; Clang 23.1.3; LLVM commit `0d261d1ca552c95a8f007e061c787ac7132fbcbc` | No |
| Microsoft C++ headers/libraries and CRT | [Visual Studio](https://visualstudio.microsoft.com/downloads/) | MSVC 14.51.36231 | Only listed execution CRT DLLs |
| Windows SDK | [Microsoft](https://developer.microsoft.com/windows/downloads/windows-sdk/) | 10.0.28000.0 | No |
| Launcher C# compiler | Windows .NET Framework | 4.8.9221.0, x64 | No |
| CMake | [Kitware](https://cmake.org/) | 4.4.4 | No |
| Ninja | [Ninja](https://github.com/ninja-build/ninja) | 1.13.2.git.kitware.jobserver-pipe-1 | No |

The original development setup retrieved Microsoft payloads using a local portable-msvc.py downloader from the [mmozeiko downloader family](https://gist.github.com/mmozeiko/7f3162ec2988e81e56d5c4e22cde9977), with local MSI/CAB extraction. That downloader is not distributed or required by the normal developer-tool workflow. Its exact external revision was not preserved, so no byte-for-byte match with today's gist is claimed. The local script fingerprint is retained in the Portuguese section below.

Original assets come exclusively from the player's ISO. The supported XEX hash identifies the edition; it does not redistribute that executable. Runtime hashes are recorded in PROVENANCE.json and runtime/SHA256SUMS.txt; validation scope and limits are in ARCHITECTURE.md.

---

<a id="portugues"></a>

# Origem dos componentes

O frontend usa Dear ImGui e o carregador GIF do fork [recomp-ui](https://github.com/ExtremestoneGG/recomp-ui), commit `047bc82d8679e96d1dec19ce2cd8ab677003465f`, sob MIT. O Full Emerald é a referência visual; o host C++ é próprio e usa SDL3, enquanto o host original usa SDL2. Não são distribuídos recursos de Pokémon. O GIF de fundo é opcional e ainda não acompanha o pacote. Segoe UI é lida da instalação Windows, sem redistribuir a fonte.


Este documento distingue dependências efetivamente usadas, ferramentas do ambiente e referências de arquitetura. `PROVENANCE.json` fixa as revisões e hashes; `THIRD_PARTY.md` e a pasta `licenses` da release preservam os termos originais.

## Código próprio

O host específico do frontend, a ponte de processos, o importador, a verificação da edição, a organização portátil e os testes foram implementados neste projeto. As dependências de interface são creditadas acima. As integrações do título, correções de callbacks, cadência e overlay ficam em `native-rex/src` e no manifesto. Não são um novo kernel ou renderizador.

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

## Alpha additions maintained by this project

The resident-to-streamed animation converter, owned reset lifecycle, original-menu wrappers and SDL presentation extension are project-authored code. Animation format details were established from local supported-game data and checked with independent C#/Python transformations; no external game's implementation or asset is imported. The conversion source is `launcher/AllAliensPatch.cs` and `tools/merge_alien_streams.py`.

`native-rex/src/sdk_presentation_extension.inc` is maintained here and carried into the SDK through the single diff. It calls the pinned SDL dependency's window/display APIs inside ReXGlue's existing window unit. SDL and ReXGlue license notices and corresponding sources remain included. No new renderer or kernel dependency was introduced.

Português: as conversões de animação, ciclo de reset, extensões de menus e integração SDL foram implementadas neste projeto. Os dados são derivados localmente da cópia do usuário. Nenhum patch de tradução ou recurso de outro jogo é distribuído. As origens, revisões e licenças das dependências continuam registradas acima.
