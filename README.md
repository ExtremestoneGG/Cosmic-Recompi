[English](#english) | [Português (Brasil)](#portugues)

<a id="english"></a>
# Cosmic Recomp

Experimental Windows 10/11 x64 native port of **Ben 10 Ultimate Alien: Cosmic Destruction** (Xbox 360), built on ReXGlue v0.10.0 and Direct3D 12.

**Bring your own supported copy.** No ISO, XEX, game assets, generated game code or personal saves are distributed.

[Download](https://github.com/ExtremestoneGG/Cosmic-Recompi/releases/tag/v0.3.0-test.1) · [Releases](https://github.com/ExtremestoneGG/Cosmic-Recompi/releases) · [User guide](USER_GUIDE.md) · [Build guide](BUILD.md) · [Testing](TESTING.md) · [Sources and credits](SOURCES.md)

## Current test release: v0.3.0-test.1

- One portable executable: select your ISO, validate it, then **Prepare game** with progress reporting. Import uses a precompiled native runtime; it does not compile C++ on the player's PC.
- Independent Full Emerald-style launcher with **Play, Settings, Quit**, corrected spacing and a fading green selection highlight. Play closes the frontend renderer before gameplay. Automatic direct-start was removed.
- **Options → Video** uses the original game's menu system: resolution presets and 30/60/Unlimited frame limits. Launcher Settings controls the same saved values. Resolution changes require a restart; live resolution and additional fullscreen modes remain pending.
- **Quit to launcher**, early startup logs, save-folder information, translation-table import and desktop shortcut creation after import.
- Experimental **Gameplay → All Aliens**: hold LT and tap RT to change pages. RT is reserved for paging while LT is held. **Rath is disabled in this release**, including saved-slot selection; his transformation crash is not fixed. Four Arms is not included.

All Aliens starts OFF. Its resource loading and full roster remain experimental. Stable 60 FPS, the full campaign and every transformation have not been validated. The previous first-boss death crash remains unverified. See [release notes](RELEASE_NOTES.md) before testing.

## Getting started

1. Put the portable EXE in a writable folder on any drive and run it. It creates `CosmicRecomp` beside itself.
2. Open Settings, **Select ISO**, wait for acceptance, then **Prepare game**.
3. Click **Play**. Default is native 1280×720 with an internal 30 FPS limit. Higher limits do not guarantee matching performance or correct animation cadence.
4. Keep `CosmicRecomp/game-data/user` when updating. Setup logs are in `CosmicRecomp/logs`; game/startup logs are in `CosmicRecomp/game-data/logs`.

Save/Continue was manually confirmed in an earlier test. Save files are never supplied with the download. Supported executable identification, patches and recovery instructions are in the [user guide](USER_GUIDE.md).

## Technical basis

ReXGlue supplies the Xbox kernel services, GPU translation and audio; this project does not replace them with a new renderer/kernel. [SOURCES.md](SOURCES.md), [THIRD_PARTY.md](THIRD_PARTY.md), [PROVENANCE.json](PROVENANCE.json) and `LICENSES/` identify upstream sources, pinned revisions, local changes and licenses. Corresponding SDK sources accompany the release. CPU translation, function validation and whole-game compatibility are separate milestones.

---
<a id="portugues"></a>
# Português (Brasil)

Port nativo experimental para Windows 10/11 x64 de **Ben 10 Ultimate Alien: Cosmic Destruction**, baseado na edição Xbox 360, ReXGlue v0.10.0 e Direct3D 12.

**Você precisa fornecer sua própria cópia compatível.** O projeto não distribui ISO, XEX, dados do jogo, código gerado do jogo ou saves pessoais.

Baixe o executável portátil em [Releases](https://github.com/ExtremestoneGG/Cosmic-Recompi/releases), coloque em uma pasta gravável de qualquer disco e execute. Em Settings, selecione a ISO, aguarde a validação e clique em Prepare game. Depois clique em Play. A interface fecha antes de iniciar o jogo.

Esta versão inclui os ajustes do launcher, menu Video do próprio jogo, importação de tabela de tradução, logs iniciais, informações dos saves e All Aliens experimental. **Rath está desativado**, inclusive em slots de saves antigos; o crash dele continua em investigação. O Quatro Braços não está incluído. Segure LT e toque RT para trocar as páginas; enquanto LT estiver segurado, RT não aciona habilidades.

O padrão é 1280×720 a 30 FPS. 60 FPS e Unlimited são limites disponíveis, sem garantia de desempenho ou cadência correta. A resolução ainda exige reinício. All Aliens começa desligado; todas as transformações e a campanha inteira ainda precisam de validação. O crash anterior ao morrer no primeiro chefe também não foi confirmado como corrigido.

Preserve `CosmicRecomp/game-data/user` ao atualizar. Logs do jogo e de sua inicialização ficam em `CosmicRecomp/game-data/logs`; logs do pacote ficam em `CosmicRecomp/logs`. Consulte o [guia](USER_GUIDE.md), as [notas da versão](RELEASE_NOTES.md) e as [origens dos componentes](SOURCES.md).
