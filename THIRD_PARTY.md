[English](#english) | [Português (Brasil)](#portugues)

<a id="english"></a>

# Third-party components and licenses

The standalone frontend reuses Dear ImGui and the GIF loader from [Full Emerald's recomp-ui fork](https://github.com/ExtremestoneGG/recomp-ui), revision `047bc82d8679e96d1dec19ce2cd8ab677003465f`. Preserve its MIT notice (Matthew Stanley), Dear ImGui's MIT notice and stb's MIT/public-domain terms. Its separately compiled SDL3 uses the zlib license. No Full Emerald game assets are included. Windows' installed Segoe UI font is read at runtime and is not redistributed.

The runtime uses [ReXGlue SDK](https://github.com/rexglue/rexglue-sdk), commit `c94f5ebdcb3c9d1a460ca48e04f9758448f8d518`, with local patches. Its principal SDK code uses BSD-3-Clause and preserves Xenia credits. The local diff is supplied with the project sources; the original SDK notice is in `LICENSES/ReXGlue.txt`.

SDK dependencies retain their own terms, including SDL, ImGui, fmt, spdlog, xxHash, toml++, SPIR-V components, compression libraries, and codecs. Preserve their original notices supplied under `licenses` in the binary package and in the complete SDK source archive. Component origins and exact recorded revisions are listed in SOURCES.md and PROVENANCE.json.

This software uses code from [FFmpeg](https://ffmpeg.org), licensed under [LGPLv2.1 or later](https://www.gnu.org/licenses/old-licenses/lgpl-2.1.html). Corresponding source is provided in `CosmicRecomp-test-0.2.1-sdk-corresponding-source.zip`, attached to the release. The SDK builds the audio subset described in `thirdparty/CMakeLists.txt` and the codec/parser/filter lists in `thirdparty/ffmpeg-overlay`; this is not a distribution of the ffmpeg command-line application. The LZX decompressor from libmspack also includes LGPL code.

For rebuilding, use the complete corrected snapshot `CosmicRecomp-test-0.2.1-sdk-source-fix1.zip` and its separate SHA256SUMS file. The original 0.2.1 source archive omitted a project-owned direct-resolve shader header and retained a private include path. The corrected archive contains the exact existing bytecode header and a relocation helper for bundled consumer headers; game binaries are unchanged. See BUILD.md and SOURCE_CORRECTION.md inside the archive. Previously published assets remain available for traceability.

The source archive includes the SDK, its dependencies, the current local diff and current consumer integration headers, without game assets. This hotfix changes failure-only runtime diagnostics; the archive includes that source. Application/launcher sources are also in the matching repository tag. No additional restriction is imposed on modifying LGPL libraries or reverse engineering necessary to debug those modifications.

Microsoft Visual C++ Runtime DLLs, when supplied, retain Microsoft's redistribution terms; the project's BSD license does not cover them. See [Microsoft's redistribution documentation](https://learn.microsoft.com/cpp/windows/redistributing-visual-cpp-files). The .NET Framework and Windows system components are prerequisites, not bundled framework/compiler/SDK development packages.

Ben 10 trademarks and original game assets belong to their respective owners. No project license grants rights to those assets; players must supply their own copy.

---

<a id="portugues"></a>

# Componentes de terceiros

O frontend reutiliza Dear ImGui e o carregador GIF do fork recomp-ui do Full Emerald, commit `047bc82d8679e96d1dec19ce2cd8ab677003465f`. As licenças MIT do projeto e ImGui, os termos do stb e a licença zlib do SDL devem acompanhar a distribuição. Não são incluídos recursos do jogo Full Emerald nem o arquivo da fonte Segoe UI do Windows.

O runtime é baseado no [ReXGlue SDK](https://github.com/rexglue/rexglue-sdk), commit `c94f5ebdcb3c9d1a460ca48e04f9758448f8d518`, com patches locais. O SDK mantém créditos ao projeto Xenia e usa BSD-3-Clause para seu código principal. O diff local acompanha o código deste projeto.

As dependências do SDK mantêm suas próprias licenças: entre elas SDL, ImGui, fmt, spdlog, xxHash, toml++, SPIR-V e bibliotecas de processamento/compressão. FFmpeg e o descompressor LZX de libmspack incluem componentes LGPL. As cópias das licenças fornecidas junto ao pacote não devem ser removidas.

Este software usa código de [FFmpeg](https://ffmpeg.org), licenciado sob [LGPLv2.1 ou posterior](https://www.gnu.org/licenses/old-licenses/lgpl-2.1.html). Seu código-fonte correspondente está no arquivo `CosmicRecomp-test-0.2.1-sdk-corresponding-source.zip` anexado à mesma release. O SDK compila o subconjunto de áudio descrito em `thirdparty/CMakeLists.txt`, com as listas de codecs/parser/bitstream-filter em `thirdparty/ffmpeg-overlay`; não é uma distribuição do executável `ffmpeg`.

Os arquivos Microsoft Visual C++ Runtime, quando incluídos, mantêm os termos de redistribuição da Microsoft. O .NET Framework e componentes do Windows são requisitos do sistema e não são distribuídos como parte do launcher.

O arquivo de fontes inclui o SDK, dependências, diff atual e cabeçalhos atuais do consumidor. Este hotfix altera apenas o diagnóstico de falhas no runtime; essa alteração acompanha as fontes. O código atual do aplicativo/launcher também está na tag correspondente do repositório.

Para recompilar, use o pacote completo corrigido `CosmicRecomp-test-0.2.1-sdk-source-fix1.zip`, com seu SHA256SUMS separado. O pacote anterior omitia um header de shader próprio do projeto e mantinha um include privado. A correção inclui os mesmos bytes do shader e um auxiliar para ajustar os includes aos headers fornecidos. Os binários do jogo não mudaram. Consulte BUILD.md e SOURCE_CORRECTION.md dentro do arquivo; os arquivos já publicados são preservados para rastreabilidade.

Referência oficial: [Redistribute Visual C++ Files](https://learn.microsoft.com/cpp/windows/redistributing-visual-cpp-files). A licença BSD do projeto não se aplica aos DLLs da Microsoft. Não distribuímos o compilador, headers ou bibliotecas de desenvolvimento do Windows/MSVC.

As marcas e os recursos de Ben 10 pertencem a seus respectivos titulares. Nenhuma licença deste repositório concede direitos sobre esses recursos. Eles devem ser obtidos da cópia do próprio usuário.

O arquivo de fontes correspondentes do SDK, quando anexado à release, contém o SDK utilizado e seus submódulos, sem arquivos do jogo. Para modificar/recompilar o runtime, preserve os termos das dependências e consulte a documentação de build do SDK. Nenhuma restrição adicional é imposta à modificação das bibliotecas LGPL ou à engenharia reversa necessária para depurá-las.
