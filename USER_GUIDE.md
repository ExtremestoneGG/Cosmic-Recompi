[English](#english) | [Português (Brasil)](#portugues)

<a id="english"></a>
# User guide

## Prepare your copy

Download the portable Windows EXE from Releases. Run it and choose an installation folder on any writable drive. Versioned host files go under app/ and shared game files, saves and logs under game-data/ inside the selected folder. Select the existing installation folder when updating. Avoid protected system folders. Do not mix runtime files from different releases.

Open Settings → Select ISO. Acceptance verifies the supported executable before extraction; it does not prove gameplay compatibility. Then choose Prepare game and wait for completion. The importer reads your ISO without changing it, stages extracted assets and activates only a completed import. It uses an included precompiled runtime, with no local C++ compiler required. Allow approximately 6 GB beyond the ISO and caches. A desktop shortcut is created after import.

Only the Xbox 360 executable revision identified by the importer's embedded SHA-256 check is supported. Other revisions require separate code generation and validation. The exact digest is in `launcher/InstalledGame.cs`; an unsupported ISO must not be forced through the check.

Click Play to start. The standalone OpenGL launcher exits; a small non-rendering bridge owns the installation lock and handles return requests. The native game uses the ReXGlue Direct3D 12 backend. Start directly in game has been removed.

## Video and input

Launcher Settings and the original game's Options → Video share `user/pc-video.ini`. Select 720p, 1080p, 1440p or 4K and 30, 60 or Unlimited FPS. The 30 FPS default uses internal pacing; 60/Unlimited are experimental and may expose cadence/judder problems. A cap is not a performance guarantee. Output resolution changes live in windowed and supported exclusive fullscreen modes. Borderless uses the desktop size; this is not increased internal rendering resolution. Show FPS switches the FPS counter and frame-time graph together. The game overlay counts guest frame submissions and shows frame times; it is not a guarantee that every submitted frame is a unique monitor image.

Gameplay → All Aliens is OFF by default. Hold LT and tap RT to cycle the original four forms, four additional forms and the remaining three. RT cannot activate the ability menu while LT is held. Releasing LT restores the original four slots. Rath uses normal progression/the original cheat, and is also available through All Aliens. His animation-resource crash was repaired in the tested scene. Four Arms is absent. All Aliens transformations/resource lifetime remain experimental. Turn it OFF to use the baseline roster path.

Quit to launcher requests exit through the bridge and reopens the frontend. It does not guarantee a new save: use the game's own save/checkpoint first. Save/Continue was manually verified in an earlier test, but full-campaign and every death/reload path are not validated.

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

## Saves, logs and updates

For the portable EXE, saves are under `CosmicRecomp/game-data/user`. For a manually staged ZIP, they are under `installation/user` beside the launcher unless an existing installation path was configured. Settings displays the actual save directory and offers Open saves. Preserve the whole user directory when updating; do not copy only one file from a multi-file save.

Close the game before making a backup. To reset, rename `user` to `user-backup` while it is closed; this preserves a reversible copy. Alternatively, Gameplay → Wipe Data asks for confirmation, closes the game and moves this title’s profiles to save-backups. Native video preferences remain intact. Keep the archive until you have verified your new profile.

Setup failures: `CosmicRecomp/logs/setup.log`. Game/startup failures: the installation's `logs/launcher.log`, `logs/startup-output.log`, `logs/runtime.log`. Early Windows/D3D failures may occur before runtime.log exists; include launcher.log and startup-output.log in a report. State what you selected, the last visible scene, whether All Aliens was ON, and your GPU/Windows version. Remove personal paths before posting logs publicly.

## Optional translation tables

After import, Settings → Add translation patch accepts a user-supplied `global/scripts/xbx_us.txt` table. The native menu labels are merged into the table; original backups are retained. This is translation-table support, not a universal binary/XEX patch engine. The separately distributed `X360-Ben-10-Ultimate-Alien-Cosmic-Destruction-USA-Traducao-PT-BR-V1.0-XEX-BRASIL` patch is the local compatibility reference; obtain it from its authors. No patch files are hosted here. Compatibility with every revision is not guaranteed.

---
<a id="portugues"></a>
# Guia em português

Execute o instalador e escolha a pasta de instalação em qualquer disco gravável. Para atualizar, escolha a pasta CosmicRecomp existente. Em Settings → Select ISO, selecione sua cópia compatível. Depois da aceitação, clique em Prepare game e aguarde o progresso. O programa importa os dados para um runtime já compilado; não compila C++ no PC do usuário nem altera a ISO. Após preparar, clique em Play. O launcher fecha e o jogo abre em processo separado. Um atalho é criado após a importação.

As configurações de vídeo ficam tanto no launcher quanto no menu original Options → Video. Há 720p/1080p/1440p/4K e 30/60/Unlimited FPS. O padrão é 30, com limite interno. 60/Unlimited são experimentais e podem apresentar problemas de cadência; a resolução de saída muda sem reiniciar em janela e nos modos exclusivos compatíveis. Borderless acompanha o desktop. Show FPS controla juntos contador e frametime. O contador mostra submissões de quadros do jogo, sem garantir imagens únicas no monitor.

Gameplay → All Aliens começa desligado. Segure LT e toque RT para alternar 4 + 4 + 3 formas. Enquanto LT estiver pressionado, RT não aciona as habilidades. Soltar LT restaura os quatro slots originais. Rath mantém o progresso/cheat normal e também fica disponível no All Aliens. O crash de recurso de animação foi corrigido na cena testada. Quatro Braços não está incluído. As transformações e o ciclo de recursos do All Aliens ainda são experimentais.

Controles são aceitos nativamente via SDL, incluindo XInput. Teclado: WASD move; setas = câmera; Espaço = A; Backspace = B; L/P = X/Y; Q/I = LT; E/O = RT; 1/3 = LB/RB; F/K = clique dos analógicos; Shift + setas = direcional; Enter/X = Start; Tab/Z = Back. Com All Aliens ligado, segure Q e toque E para trocar a página. Mouse não vem ativado. Mapeamento conferido no código/configuração; nem toda ação contextual foi testada.

No pacote portátil, preserve `CosmicRecomp/game-data/user` ao atualizar. Settings mostra a pasta real e permite abri-la. Feche o jogo antes de copiar os saves. Para resetar sem perder a cópia, renomeie a pasta user para user-backup. Quit to launcher não substitui o save/checkpoint do próprio jogo. Wipe Data pede confirmação e arquiva os perfis deste título em save-backups após o jogo fechar; as configurações nativas de vídeo são preservadas.

Logs da preparação ficam em `CosmicRecomp/logs`. Logs do jogo e da tentativa de iniciá-lo ficam em `CosmicRecomp/game-data/logs`. Envie launcher.log e startup-output.log mesmo quando runtime.log estiver vazio. Informe a ação, a fase, se All Aliens estava ligado e a versão do Windows/GPU. Remova dados pessoais antes de publicar os logs.

Settings → Add translation patch aceita a tabela `global/scripts/xbx_us.txt` fornecida pelo usuário. Não é um instalador universal de patches binários ou XEX. A tradução PT-BR XEX-BRASIL foi usada como referência local; obtenha o patch com seus autores. Nenhum arquivo da tradução é distribuído pelo projeto. Consulte as limitações em RELEASE_NOTES.md.
