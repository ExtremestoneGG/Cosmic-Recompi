# Como o port funciona

## Da ISO ao jogo

O launcher C# reconhece a partição XDVDFS e lê o catálogo da ISO sem montá-la nem alterá-la. Antes de extrair, compara o hash do `default.xex` com a edição para a qual o executável nativo foi gerado. Verifica limites de leitura, nomes de dispositivos Windows, caminhos inválidos e ciclos de diretórios.

A extração acontece numa pasta nova `installation/import-<id>`. Somente quando termina ela passa a ser `installation/game`. Cancelamentos deixam a pasta parcial identificada; saves existentes não são removidos. A primeira versão não retoma uma extração parcial: uma nova tentativa usa outra pasta. Pastas parciais podem ocupar espaço e devem ser tratadas pelo usuário quando nenhum processo as estiver utilizando.

O ZIP fornece um executável Windows já compilado. O importador não converte a ISO inteira em C++ durante a instalação. A geração estática das funções PowerPC acontece durante o desenvolvimento com `rexglue codegen`, sobre a cópia local do desenvolvedor. Essas fontes geradas não são publicadas no Git.

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

O launcher fornece os ajustes gráficos acumulados e o limite de 30 FPS ao iniciar o runtime. O manifesto SHA-256 do runtime detecta cópias incompletas/alteradas; não é assinatura digital nem proteção contra modificações deliberadas.

## FPS e cadência

O overlay conta submissões do jogo no caminho `VdSwap`, separadamente das reapresentações da janela. Contadores externos podem contar apresentações repetidas e exibir um número maior sem a mesma quantidade de novos quadros do jogo.

A build de testes usa a cadência interna de 30 Hz que melhorou o movimento da câmera no teste manual. Isso não prova que toda animação/simulação funcione corretamente a 60 Hz. Por esse motivo o modo ilimitado fica indisponível e as opções de 30/60 FPS dentro de Options serão uma atualização separada.

## O que foi confirmado

- Leitura da ISO suportada: 1.352 arquivos, 5.435.433.670 bytes de recursos.
- Extração completa numa instalação nova, com nomes/tamanhos conferidos contra o catálogo e hash do XEX validado. Não foi feita uma segunda leitura de hash de todos os 5,4 GB.
- Inicialização do executável portátil e reconhecimento do controle XInput.
- Save pelo menu do jogo, encerramento, nova abertura e carregamento por Continuar, confirmado pelo jogador no checkpoint testado.
- Nove testes sintéticos do parser: catálogo válido, imagem truncada, travessia de caminhos, nome reservado, ponto final, endereço fora da imagem, ciclo de árvore, assinatura final inválida e XEX de edição não suportada.

Não foram confirmados: campanha completa, todas as GPUs/drivers, todos os checkpoints, compilação em máquina limpa, desempenho estável a 60 FPS ou a futura integração de vídeo ao menu original. Estes itens não devem aparecer como concluídos em anúncios da versão.
