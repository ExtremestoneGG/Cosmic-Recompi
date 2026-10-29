[English](#english) | [Português (Brasil)](#portugues)

<a id="english"></a>
# Cosmic Recomp

Experimental Windows 10/11 x64 native port of **Ben 10 Ultimate Alien: Cosmic Destruction** (Xbox 360), using ReXGlue v0.10.0 and Direct3D 12.

**Alpha software. Camera/scenery flicker above 30 FPS is still present and is the next priority.**

**Bring your own supported copy.** No ISO, XEX, original assets, generated game source or personal saves are distributed.

[Download](https://github.com/ExtremestoneGG/Cosmic-Recompi/releases/tag/v0.4.0-alpha.4) · [User guide](USER_GUIDE.md) · [Build guide](BUILD.md) · [Contributor / coding-agent runbook](docs/CONTRIBUTOR_BUILD_RUNBOOK.md) · [Validation](VALIDATION.json) · [Sources](SOURCES.md)

Latest update: [animation and lever crash fixes](RELEASE_NOTES.md), including a manual phase-1 lever test. Rath upgrades with All Aliens disabled remain under investigation.

Latest update: **boss-transition stability, proportional Hero Time and accumulated native-quality optimizations**. [Details](RELEASE_NOTES.md).

## Alpha v0.4.0-alpha.4 — stability and performance

- One portable EXE: select your ISO, wait for acceptance, then **Prepare game**. Import uses a precompiled native runtime; it does not compile C++ on the player's PC.
- Separate launcher with **Play, Settings, Quit**. Play closes the frontend before gameplay. There is no automatic-start checkbox.
- Original in-game **Options → Video** menus: output resolution, 30/60/Unlimited frame limits, display mode and **Show FPS → On/Off**. Show FPS controls both the counter and frame-time graph. Launcher Settings edits the same values.
- Live output size in windowed mode and supported exclusive fullscreen modes; borderless uses the desktop's size. These are presentation settings, not a claim of increased internal rendering resolution. Native 1280×720 remains the minimum.
- Quit to launcher, early startup diagnostics, save-folder information, desktop shortcut creation, and user-supplied text-translation import.
- Repaired missing animation-stream resources for All Aliens. Eleven ordinary forms, including Rath, transformed in the recorded Paris scene. This is scene-level validation, not full campaign coverage.
- **Gameplay → Wipe Data** asks for confirmation, closes the game, archives this title's profile data, and returns to the launcher. Native video settings remain intact. Backups are in `game-data/save-backups`.

All Aliens starts **OFF**. Hold LT and tap RT to change pages; RT abilities are suppressed while LT is held. Normal Rath progression and the original cheat are preserved. All Aliens makes Rath available without changing the profile unlock. Four Arms is not included.

Hero Time starts **OFF**. Enable it under **Options → Gameplay → Extras**. Use a compatible normal alien, then **L3 / F** for its corresponding Ultimate. It lasts up to 60 seconds, followed by proportional recharge as Ben (time used / 3, maximum 20 seconds). See [Hero Time details](RELEASE_NOTES.md) for tested scope and known issues.

## Keyboard and controllers

Controllers are supported natively through the SDK SDL backend, including XInput-compatible controllers. Prompts retain Xbox button names. Contextual actions follow the original game.

| Keyboard | Xbox input / purpose |
| --- | --- |
| WASD | Left stick: movement |
| Arrows | Right stick: camera |
| Space or semicolon | A: confirm / jump |
| Backspace or apostrophe | B: back / original B action |
| L / P | X / Y |
| Q or I | LT: alien wheel |
| E or O | RT: abilities; next wheel page while LT held with All Aliens ON |
| 1 / 3 | LB / RB |
| F / K | Left / right stick click |
| Shift + arrows | D-pad |
| Enter or X | Start / pause |
| Tab or Z | Back / Select |

With All Aliens ON, hold Q and tap E to cycle pages. Mouse camera is opt-in and disabled by default. Mappings checked against the SDK keyboard driver and launcher configuration; not every contextual keyboard action has been playtested.

## Getting started

1. Run the installer and choose your installation folder on any writable drive. To update, select the existing CosmicRecomp folder.
2. In Settings, choose **Select ISO**, wait for validation, then **Prepare game**.
3. Click **Play**. The default is native 720p with a 30 FPS limit. Higher limits remain experimental.
4. Preserve `CosmicRecomp/game-data/user` when updating. Setup logs are in `CosmicRecomp/logs`; game/startup logs are in `CosmicRecomp/game-data/logs`.

Save/Continue was manually confirmed in an earlier test. No saves are supplied. Use a separate folder for a new test release and preserve your existing user folder before migration.

## Known limits

Stable 60 FPS and camera/scenery flicker above 30 FPS remain open. Full campaign compatibility, extended combat/abilities and the earlier first-boss death regression remain unverified. Returning from the main menu to Press Start with B is pending. Retained performance changes and the limits of the fixed-scene measurement are documented in [release notes](RELEASE_NOTES.md). Unsupported exclusive resolutions are rejected and logged.

ReXGlue supplies the kernel, GPU translation and audio. This project does not replace them with a new renderer or kernel. [SOURCES.md](SOURCES.md), [THIRD_PARTY.md](THIRD_PARTY.md), [PROVENANCE.json](PROVENANCE.json) and `LICENSES/` record sources, revisions, changes and licenses. Complete corresponding SDK sources accompany the release. A full clean-machine rebuild remains unverified.

---
<a id="portugues"></a>
# Português (Brasil)

Port nativo experimental para Windows 10/11 x64 de **Ben 10 Ultimate Alien: Cosmic Destruction**, baseado no ReXGlue v0.10.0 e Direct3D 12. **Você precisa fornecer sua própria cópia compatível.** Nenhum dado original, código gerado do jogo ou save pessoal é distribuído.

Esta versão corrige recursos de animação ausentes no All Aliens e acrescenta opções nos menus do próprio jogo: resolução de saída, 30/60/Unlimited, modo de tela e **Show FPS**, que mostra ou esconde juntos o contador e o gráfico de frametime. O launcher controla os mesmos valores. A resolução de saída muda sem reiniciar em janela e nos modos exclusivos compatíveis; borderless acompanha o desktop. Isso não significa aumento da resolução interna.

O All Aliens começa desligado. Segure LT e toque RT para trocar páginas; habilidades por RT ficam bloqueadas enquanto LT estiver segurado. As onze formas comuns, incluindo Rath, foram verificadas na cena de Paris. O progresso normal e o cheat do Rath são preservados. Quatro Braços não está incluído.

Wipe Data pede confirmação, fecha o jogo, guarda os dados deste título em `game-data/save-backups` e retorna ao launcher. As configurações nativas de vídeo são preservadas. Mantenha `game-data/user` ao atualizar. Logs ficam em `game-data/logs`.

A meta de 60 FPS estáveis e a tremedeira acima de 30 FPS continuam pendentes. A campanha completa e o crash antigo ao morrer no primeiro chefe ainda precisam de verificação. B para voltar ao Press Start também está pendente. As otimizações acumuladas e os limites da medição estão nas notas desta Alpha. Consulte as [notas](RELEASE_NOTES.md), [testes](TESTING.md) e [origens](SOURCES.md).

Hero Time agora inclui brilho verde e recarga proporcional ao uso, limitada a 20 segundos. A correção da transição dos Vreedle e as otimizações acumuladas estão nas [notas da Alpha 4](RELEASE_NOTES.md).

## Teclado e controle

Controles são aceitos nativamente via SDL, incluindo modelos compatíveis com XInput.
Os avisos do jogo continuam mostrando os botões do Xbox.

| Tecla | Ação / botão Xbox |
| --- | --- |
| WASD | Movimento |
| Setas | Câmera |
| Espaço ou ponto e vírgula | A: confirmar / pular |
| Backspace ou apóstrofo | B: voltar / ação original |
| L / P | X / Y |
| Q ou I | LT: seleção de aliens |
| E ou O | RT: habilidades / próxima página com LT segurado e All Aliens ligado |
| 1 / 3 | LB / RB |
| F / K | Clique do analógico esquerdo / direito |
| Shift + setas | Direcional |
| Enter ou X | Start / pausa |
| Tab ou Z | Back / Select |

Com All Aliens ligado, segure Q e toque E para trocar páginas. Mouse desativado
por padrão. Mapeamento conferido no código e configuração; nem toda ação
contextual foi testada em gameplay.
