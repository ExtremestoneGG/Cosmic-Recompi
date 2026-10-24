[English](#english) | [Português (Brasil)](#portugues)

<a id="english"></a>

# Architecture and validation

## From ISO to native execution

The C# launcher reads XDVDFS directly without mounting or modifying the ISO. It validates bounds, filenames, Windows device names, directory cycles, and the supported XEX hash before extraction. Assets are written to a new `import-<id>` directory, renamed to `game` only after extraction completes. Cancellation preserves an identifiable partial directory and never deletes existing saves. Partial imports are not resumed in this first implementation.

The package includes an already compiled Windows executable. Installation imports assets; static PowerPC-to-C++ generation happens during development with ReXGlue on a local game copy. Generated game source is not published.

## Frontend and game separation

The native frontend uses Dear ImGui, SDL3 and OpenGL. Its menu layout follows Full Emerald; only the reusable UI code and GIF loader are dependencies, not its game assets. An optional background GIF is decoded once and updated using one texture.

Play starts an independent C# bridge and destroys the frontend. The bridge waits for the frontend process to exit before starting the game. Therefore its window, graphics context and animation do not run during gameplay. The launcher always requires Play. Returning uses a per-launch named event; the bridge reopens the frontend.

The importer derives two menu files from the user's supported copy, keeping validated originals as backups. Original pause-menu QUIT is preserved; QUIT TO LAUNCHER has its own confirmation. VIDEO is added to original OPTIONS. SDK-dispatched callbacks open a native SDK-rendered video panel; a deferred guest callback restores the original Options menu afterward. No original bytecode is shipped.

Video preferences live in `user/pc-video.ini`. Resolution applies on next startup; the internal 30/60 Hz pacer updates live. Uncapped is disabled. A 60 Hz cap is not evidence of correct 60 FPS simulation or sufficient GPU performance.

The bridge supplies its owned startup settings explicitly on the command line. This avoids dependence on the SDK's narrow TOML filename handling when the installation path contains accented characters. Relative `logs/runtime.log` is resolved from the selected installation, not the binary directory. Original assets and saves continue to use the wide portable-root override.

## CPU, GPU, and system services

Recompiled PowerPC functions execute as native host CPU code. Xbox service calls and GPU commands still need equivalent implementations; the kernel, audio, controls, and Direct3D 12 renderer come from ReXGlue. This project does not implement another kernel or renderer. Native CPU code does not make Xbox GPU commands directly executable on every PC GPU.

## Portable data

`BEN10_PORTABLE_ROOT` selects the installation. `game` contains imported original assets; `user` contains saves/profile state; `cache` contains generated caches; `logs` contains local diagnostics; `metadata` is reserved for optional local metadata. None of these personal/original-data folders belongs in a release ZIP or Git. A launcher-selected destination remains separate from application binaries.

The launcher applies retained graphics settings and the selected 30/60 FPS cap, defaulting to 30. Runtime SHA-256 checks verify copies, not authenticity; they do not prohibit intentional library modification.

## Frame counting

The overlay counts game submissions through `VdSwap`, separately from repeated host-window presentations. External counters may report repeated presentations as additional FPS. The 30 Hz playback cadence improved camera behavior in manual testing. Correct simulation/animation at 60 Hz is not established merely by seeing a higher presentation counter, so uncapped mode remains disabled.

## Verified scope

- Supported ISO: 1,352 files and 5,435,433,670 extracted bytes.
- A full fresh import, with every path/size checked against the catalog and the XEX hash verified. A second full content-hash pass over all 5.4 GB was not performed.
- Portable native startup and XInput controller detection.
- In-game saving, process exit, restart, and Continue recovering the tested checkpoint, manually confirmed by the player.
- Nine synthetic importer tests: valid catalog, truncation, traversal, reserved device name, trailing dot, out-of-image file, a real cyclic directory-node reference, invalid closing signature, and unsupported XEX identity.

Not verified: the entire campaign, every save/checkpoint, all GPUs/drivers, a clean-machine full native build or stable 60 FPS. Original Options video integration has been exercised with automated controller replay; see TESTING.md for its exact scope.

---

<a id="portugues"></a>

# Como o port funciona

## Da ISO ao jogo

O launcher C# reconhece a partição XDVDFS e lê o catálogo da ISO sem montá-la nem alterá-la. Antes de extrair, compara o hash do `default.xex` com a edição para a qual o executável nativo foi gerado. Verifica limites de leitura, nomes de dispositivos Windows, caminhos inválidos e ciclos de diretórios.

A extração acontece numa pasta nova `installation/import-<id>`. Somente quando termina ela passa a ser `installation/game`. Cancelamentos deixam a pasta parcial identificada; saves existentes não são removidos. A primeira versão não retoma uma extração parcial: uma nova tentativa usa outra pasta. Pastas parciais podem ocupar espaço e devem ser tratadas pelo usuário quando nenhum processo as estiver utilizando.

O ZIP fornece um executável Windows já compilado. O importador não converte a ISO inteira em C++ durante a instalação. A geração estática das funções PowerPC acontece durante o desenvolvimento com `rexglue codegen`, sobre a cópia local do desenvolvedor. Essas fontes geradas não são publicadas no Git.

## Frontend separado

O frontend nativo usa Dear ImGui, SDL3 e OpenGL, seguindo o modelo visual do Full Emerald. Ele encerra antes de o jogo iniciar; uma ponte C# independente aguarda o encerramento e acompanha a execução. O início direto nem inicializa SDL. Um evento exclusivo por execução permite voltar ao launcher sem repetir o início direto.

O importador deriva os dois arquivos de menu da cópia do usuário e preserva os originais. QUIT TO LAUNCHER fica separado de QUIT; VIDEO fica em OPTIONS. O painel de vídeo usa o renderizador de interface do SDK e restaura o menu original ao fechar. Nenhum bytecode original é distribuído. As preferências ficam em `user/pc-video.ini`: resolução no próximo início e limite de 30/60 FPS, com 60 experimental e ilimitado desativado.

## CPU, GPU e serviços

As funções PowerPC recompiladas tornam-se código de CPU do Windows. Chamadas a serviços Xbox e comandos da GPU continuam precisando de implementações equivalentes: o kernel, áudio, controle e backend Direct3D 12 vêm do ReXGlue. O projeto não escreveu outro renderizador ou kernel.

Os shaders e recursos da GPU Xbox são tratados pelo backend do SDK. Código de CPU nativo não significa que comandos da GPU Xbox possam ser enviados diretamente a qualquer GPU de PC.

## Configuração portátil

`BEN10_PORTABLE_ROOT` identifica uma instalação independente da pasta de desenvolvimento. Ela contém:

| Pasta | Conteúdo | Distribuir? |
| --- | --- | --- |
| `game` | Recursos importados da ISO | Não |
| `user` | Saves, perfil e estado local | Não |
| `cache` | Shaders e caches gerados | Não |
| `logs` | Diagnósticos locais | Somente após revisão pelo usuário |
| `metadata` | Metadados opcionais locais | Não incluir recursos originais |

O launcher fornece os ajustes gráficos acumulados e o limite selecionado de 30/60 FPS, com padrão de 30. O manifesto SHA-256 detecta cópias incompletas/alteradas; não é assinatura digital nem proteção contra modificações deliberadas.

## FPS e cadência

O overlay conta submissões do jogo no caminho `VdSwap`, separadamente das reapresentações da janela. Contadores externos podem contar apresentações repetidas e exibir um número maior sem a mesma quantidade de novos quadros do jogo.

A build de testes usa a cadência interna de 30 Hz que melhorou o movimento da câmera no teste manual. Isso não prova que toda animação/simulação funcione corretamente a 60 Hz. Por esse motivo o modo ilimitado fica indisponível e as opções de 30/60 FPS dentro de Options serão uma atualização separada.

## O que foi confirmado

- Leitura da ISO suportada: 1.352 arquivos, 5.435.433.670 bytes de recursos.
- Extração completa numa instalação nova, com nomes/tamanhos conferidos contra o catálogo e hash do XEX validado. Não foi feita uma segunda leitura de hash de todos os 5,4 GB.
- Inicialização do executável portátil e reconhecimento do controle XInput.
- Save pelo menu do jogo, encerramento, nova abertura e carregamento por Continuar, confirmado pelo jogador no checkpoint testado.
- Nove testes sintéticos do parser: catálogo válido, imagem truncada, travessia de caminhos, nome reservado, ponto final, endereço fora da imagem, ciclo de árvore, assinatura final inválida e XEX de edição não suportada.

Não foram confirmados: campanha completa, todas as GPUs/drivers, todos os checkpoints, compilação em máquina limpa ou desempenho estável a 60 FPS. A integração de vídeo ao OPTIONS original passou pela reprodução automática descrita em TESTING.md.
