[English](#english) | [Português (Brasil)](#portugues)

<a id="english"></a>

# Cosmic Recomp

Experimental native Windows port of **Ben 10 Ultimate Alien: Cosmic Destruction**, based on the Xbox 360 edition and the ReXGlue SDK. Windows 10/11 x64, Direct3D 12.

**Bring your own game copy.** No ISO, XEX, original game assets, personal saves, or generated PowerPC recompilation sources are included in this repository or the test package.

[Download test releases](https://github.com/ExtremestoneGG/Cosmic-Recompi/releases) · [User guide](USER_GUIDE.md) · [Build guide](BUILD.md) · [Testing](TESTING.md) · [Credits](SOURCES.md)

## Desktop integration

- **Play / Settings / Quit**, with a floating dark-green menu following [Full Emerald](https://github.com/ExtremestoneGG/Full-Emerald).
- The same Dear ImGui / SDL family and the MIT-licensed ambient-animation loader from its recomp-ui fork. No Pokémon artwork, Mods page or Players page is included.
- **Start directly in game** skips the frontend. The frontend process exits before the game starts; no hidden launcher renderer remains running.
- **VIDEO** in the original **OPTIONS** menu, with resolution presets and 30/60 FPS caps. Uncapped remains disabled; resolution changes require a restart.
- A separate **QUIT TO LAUNCHER** action next to **QUIT** in the original pause menu.

The launcher uses OpenGL; the game continues to use the ReXGlue Direct3D 12 renderer. An optional background loop can be supplied later; no recording is bundled yet.

## Install and play

1. Download the Windows ZIP from [Releases](https://github.com/ExtremestoneGG/Cosmic-Recompi/releases).
2. Extract the **entire ZIP** to a writable folder on any drive. Do not run it inside the archive.
3. Open `CosmicRecomp.exe`. In **Settings**, choose an installation folder and select your own Xbox 360 ISO.
4. Wait for edition verification and asset import. Allow approximately 6 GB free beyond the ISO and caches.
5. Select **Play**. Xbox/XInput controllers are handled by the runtime; an in-game overlay shows game FPS and frame time. The default cap is 30 FPS.

The launcher imports your assets for the included, already compiled native executable. It **does not** run code generation or compile C++ on the player's computer.

Only the edition with this `default.xex` SHA-256 is currently supported:

```text
4e8c3b121081ce26fd3ec61f28e935951a755d7015ae42e23a2c7899b730dd89
```

Unsupported editions are rejected. Renaming an ISO does not change its compatibility.

## Saves and updates

Save using the **game's own save system**. Files live in **`<chosen installation folder>/user`**. Settings shows the full path; **Open saves** opens it.

Manual validation covered saving, exiting the game, restarting the same installation, and selecting Continue to recover the saved checkpoint. This confirms that tested checkpoint, not every campaign save.

To update, extract the new release into a separate program folder and select the existing installation in Settings. Preserve `user` and `game`. Never upload that installation: it contains original assets and personal data. First play may extend the verified local menu scripts, preserving original backups; it does not modify the XEX or your saves. With the game closed, rename `user` to `user-backup` to reset progress. Renaming only `user/pc-video.ini` resets PC video settings without removing game progress.

## Requirements and current limits

- Windows 10/11 x64, .NET Framework 4.5 or later, an AVX2 CPU, Direct3D 12 for the game and OpenGL 3.0 for the separate launcher.
- Default output: 1280×720 with an internal 30 FPS cap. Uncapped mode is disabled in this test distribution.
- Tested: supported ISO import, native startup, menus, audio, XInput controls, initial gameplay, and the manual save/Continue cycle.
- The full campaign, all checkpoints, and all GPUs are **not** validated. Performance depends on hardware; **stable 60 FPS has not been achieved/verified**.
- Video settings expose 30/60 FPS caps; choosing 60 does not guarantee 60 FPS performance or eliminate the remaining camera issues above 30 FPS. Translation coverage, validation and overall compatibility are separate measures.
- An extra achievements menu is deferred.
- The gameplay callback crash at `0x8210BB88` is corrected in v0.2.1-test.1. A separate crash reported when dying at the first boss (`0x00000000`) remains under investigation; failure-only register/stack diagnostics are included. See [hotfix details](HOTFIX.md).

## Reporting problems

Include release version, CPU, GPU, driver and reproduction steps. Local logs are at `<chosen installation folder>/logs/runtime.log`; review personal information before sharing them. Do not attach ISO, XEX, original resources or personal saves to issues.

This is an experimental, unsigned package. The launcher does not download the game.

## Developer documentation

- [Build guide](BUILD.md): source layout, toolchain, local generation, and reproducibility limits.
- [Component origins](SOURCES.md): direct dependencies, exact revisions, tool origins, and upstream lineage.
- [Architecture and validation](ARCHITECTURE.md): runtime design, import, saves, frame counting, and verified versus unverified behavior.
- [Third-party notices](THIRD_PARTY.md): licenses and corresponding SDK/dependency sources.
- [Machine-readable provenance](PROVENANCE.json): SDK/dependency revisions and runtime hashes.
- [User guide](USER_GUIDE.md), [testing](TESTING.md) and [troubleshooting](TROUBLESHOOTING.md).
- [Contributing](CONTRIBUTING.md), [roadmap](ROADMAP.md) and [security reports](SECURITY.md).

Project-owned code is BSD-3-Clause; third-party components retain their own terms.

---

<a id="portugues"></a>

# Cosmic Recomp

Recompilação nativa experimental de **Ben 10 Ultimate Alien: Cosmic Destruction**, edição Xbox 360, para Windows 10/11 x64. Baseada no SDK ReXGlue, com Direct3D 12, áudio e controles do SDK.

A versão v0.2.1-test.1 corrige o callback ausente `0x8210BB88`. O crash separado ao morrer no primeiro chefe (`0x00000000`) continua em investigação, com diagnóstico ampliado no log. Consulte [os detalhes](HOTFIX.md).

**Você precisa fornecer sua própria cópia do jogo.** Este repositório e os pacotes de distribuição não contêm ISO, XEX, recursos originais, saves pessoais ou fontes PowerPC recompiladas geradas.

## Instalar e jogar

1. Baixe o ZIP da versão de testes mais recente na página Releases.
2. Extraia **todo o ZIP** para uma pasta com permissão de escrita, em qualquer disco. Não execute de dentro do arquivo compactado.
3. Abra `CosmicRecomp.exe`. Em **Settings**, escolha a pasta de instalação e selecione sua ISO.
4. Aguarde a verificação da edição e a importação dos recursos. São necessários aproximadamente 6 GB livres, além do espaço da ISO e dos caches.
5. Clique em **Play**. O padrão é 30 FPS; FPS e frame time aparecem no jogo. **Start directly in game** permite pular o launcher nos próximos inícios.

O launcher segue o menu flutuante do Full Emerald e usa Dear ImGui/SDL, com um adaptador próprio. O processo da interface encerra antes de o jogo iniciar. Não há Mods nem Players. O jogo continua em Direct3D 12, com textos em inglês. **VIDEO** fica dentro de **OPTIONS**; **QUIT TO LAUNCHER** é separado de **QUIT** no menu de pausa.

O programa importa os recursos da ISO e os prepara para o executável nativo incluído. Ele **não** executa codegen nem compila C++ no computador do jogador.

A importação reconhece atualmente apenas o `default.xex` com SHA-256:

```text
4e8c3b121081ce26fd3ec61f28e935951a755d7015ae42e23a2c7899b730dd89
```

Outras edições são recusadas para evitar executar código incompatível. Renomear uma ISO não altera sua compatibilidade.

## Saves

Use o sistema de save do **próprio jogo**. Os arquivos ficam em **`<pasta de instalação escolhida>/user`**. Settings mostra o caminho completo e **Open saves** abre a pasta.

Foi testado manualmente: salvar no jogo, encerrar o processo, abrir novamente a mesma instalação e escolher **Continuar** recuperou o progresso/checkpoint. Isso valida o trecho testado; não equivale a verificar todos os saves da campanha.

Para atualizar, extraia o novo programa separadamente e escolha a instalação existente. Preserve `user` e `game`. Os scripts originais recebem backups antes das extensões do menu; o XEX e os saves não são modificados. Para resetar o progresso, feche o jogo e renomeie `user` para `user-backup`. Renomear apenas `user/pc-video.ini` reseta o vídeo do PC. Não publique sua instalação: ela contém recursos originais e dados pessoais.

## Requisitos e limites desta versão

- Windows 10/11 de 64 bits, .NET Framework 4.5 ou posterior e CPU com AVX2.
- GPU e driver compatíveis com Direct3D 12, além de OpenGL 3.0 para o launcher; resolução padrão de 1280×720.
- Limite padrão de 30 FPS. O modo ilimitado está desativado nesta distribuição.
- Menus, áudio, controle, gameplay inicial, importação da ISO e save/carregamento foram exercitados. A campanha completa não foi validada.
- Não há garantia de 30 FPS em qualquer hardware; **60 FPS estáveis ainda não foram atingidos/verificados**.
- VIDEO permite resolução e limite de 30/60 FPS. A resolução é aplicada no próximo início. Selecionar 60 não garante desempenho de 60 FPS nem corrige toda a instabilidade da câmera acima de 30.
- O menu extra de achievements ficou para uma etapa posterior.

## Problemas e relatórios

Inclua versão do pacote, CPU/GPU/driver, etapa do jogo e instruções para reproduzir. Logs locais ficam em `installation/logs/runtime.log`. Revise dados pessoais antes de compartilhar logs. Não envie ISO, XEX ou outros arquivos originais do jogo nas issues.

O pacote é experimental e não assinado. Nenhum download adicional de jogo é realizado pelo launcher.

## Desenvolvimento

Consulte [BUILD.md](BUILD.md) para a organização das fontes e a geração local do código a partir de sua própria cópia, [SOURCES.md](SOURCES.md) para a origem de cada componente, [ARCHITECTURE.md](ARCHITECTURE.md) para funcionamento e validação, e [THIRD_PARTY.md](THIRD_PARTY.md) para licenças. O código próprio do projeto usa BSD-3-Clause; componentes de terceiros mantêm suas licenças.
