# Componentes de terceiros

O runtime é baseado no [ReXGlue SDK](https://github.com/rexglue/rexglue-sdk), commit `c94f5ebdcb3c9d1a460ca48e04f9758448f8d518`, com patches locais. O SDK mantém créditos ao projeto Xenia e usa BSD-3-Clause para seu código principal. O diff local acompanha o código deste projeto.

As dependências do SDK mantêm suas próprias licenças: entre elas SDL, ImGui, fmt, spdlog, xxHash, toml++, SPIR-V e bibliotecas de processamento/compressão. FFmpeg e o descompressor LZX de libmspack incluem componentes LGPL. As cópias das licenças fornecidas junto ao pacote não devem ser removidas.

Este software usa código de [FFmpeg](https://ffmpeg.org), licenciado sob [LGPLv2.1 ou posterior](https://www.gnu.org/licenses/old-licenses/lgpl-2.1.html). Seu código-fonte correspondente está no arquivo `CosmicRecompi-test-0.1-sdk-corresponding-source.zip` anexado à mesma release. O SDK compila o subconjunto de áudio descrito em `thirdparty/CMakeLists.txt`, com as listas de codecs/parser/bitstream-filter em `thirdparty/ffmpeg-overlay`; não é uma distribuição do executável `ffmpeg`.

Os arquivos Microsoft Visual C++ Runtime, quando incluídos, mantêm os termos de redistribuição da Microsoft. O .NET Framework e componentes do Windows são requisitos do sistema e não são distribuídos como parte do launcher.

Referência oficial: [Redistribute Visual C++ Files](https://learn.microsoft.com/cpp/windows/redistributing-visual-cpp-files). A licença BSD do projeto não se aplica aos DLLs da Microsoft. Não distribuímos o compilador, headers ou bibliotecas de desenvolvimento do Windows/MSVC.

As marcas e os recursos de Ben 10 pertencem a seus respectivos titulares. Nenhuma licença deste repositório concede direitos sobre esses recursos. Eles devem ser obtidos da cópia do próprio usuário.

O arquivo de fontes correspondentes do SDK, quando anexado à release, contém o SDK utilizado e seus submódulos, sem arquivos do jogo. Para modificar/recompilar o runtime, preserve os termos das dependências e consulte a documentação de build do SDK. Nenhuma restrição adicional é imposta à modificação das bibliotecas LGPL ou à engenharia reversa necessária para depurá-las.
