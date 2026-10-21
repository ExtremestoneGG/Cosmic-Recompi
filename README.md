# Cosmic Recompi

Recompilação nativa experimental de **Ben 10 Ultimate Alien: Cosmic Destruction**, edição Xbox 360, para Windows 10/11 x64. Baseada no SDK ReXGlue, com Direct3D 12, áudio e controles do SDK.

**Você precisa fornecer sua própria cópia do jogo.** Este repositório e os pacotes de distribuição não contêm ISO, XEX, recursos originais, saves pessoais ou fontes PowerPC recompiladas geradas.

## Instalar e jogar

1. Baixe o ZIP da primeira versão de testes na página Releases.
2. Extraia **todo o ZIP** para uma pasta no disco D:. Não execute de dentro do arquivo compactado.
3. Abra `CosmicRecompi.exe` e selecione sua ISO no D:.
4. Aguarde a verificação da edição e a importação dos recursos. São necessários aproximadamente 6 GB livres, além do espaço da ISO e dos caches.
5. Clique em **Jogar — 30 FPS**. O controle Xbox/XInput é reconhecido pelo runtime. FPS e frame time aparecem no jogo.

O programa importa os recursos da ISO e os prepara para o executável nativo incluído. Ele **não** executa codegen nem compila C++ no computador do jogador.

A importação reconhece atualmente apenas o `default.xex` com SHA-256:

```text
4e8c3b121081ce26fd3ec61f28e935951a755d7015ae42e23a2c7899b730dd89
```

Outras edições são recusadas para evitar executar código incompatível. Renomear uma ISO não altera sua compatibilidade.

## Saves

Use o sistema de save do **próprio jogo**. Os arquivos ficam em `installation/user`, ao lado do launcher. O botão **Abrir pasta de saves** mostra essa pasta.

Foi testado manualmente: salvar no jogo, encerrar o processo, abrir novamente a mesma instalação e escolher **Continuar** recuperou o progresso/checkpoint. Isso valida o trecho testado; não equivale a verificar todos os saves da campanha.

Ao atualizar, preserve `installation/user` e os recursos em `installation/game`. Substitua apenas os arquivos do pacote. Não publique sua pasta `installation`: ela contém recursos originais e dados pessoais.

## Requisitos e limites desta versão

- Windows 10/11 de 64 bits, .NET Framework 4.5 ou posterior e CPU com AVX2.
- GPU e driver compatíveis com Direct3D 12; resolução padrão de 1280×720.
- Limite padrão de 30 FPS. O modo ilimitado está desativado nesta distribuição.
- Menus, áudio, controle, gameplay inicial, importação da ISO e save/carregamento foram exercitados. A campanha completa não foi validada.
- Não há garantia de 30 FPS em qualquer hardware; **60 FPS estáveis ainda não foram atingidos/verificados**.
- A próxima atualização planejada adicionará configurações de vídeo ao menu Options do jogo, com resolução e limites de 30/60 FPS. Essa integração ainda não faz parte desta versão.
- O menu extra de achievements ficou para uma etapa posterior.

## Problemas e relatórios

Inclua versão do pacote, CPU/GPU/driver, etapa do jogo e instruções para reproduzir. Logs locais ficam em `installation/logs/runtime.log`. Revise dados pessoais antes de compartilhar logs. Não envie ISO, XEX ou outros arquivos originais do jogo nas issues.

O pacote é experimental e não assinado. Nenhum download adicional de jogo é realizado pelo launcher.

## Desenvolvimento

Consulte [BUILD.md](BUILD.md) para a organização das fontes e a geração local do código a partir de sua própria cópia, [SOURCES.md](SOURCES.md) para a origem de cada componente, [ARCHITECTURE.md](ARCHITECTURE.md) para funcionamento e validação, e [THIRD_PARTY.md](THIRD_PARTY.md) para licenças. O código próprio do projeto usa BSD-3-Clause; componentes de terceiros mantêm suas licenças.
