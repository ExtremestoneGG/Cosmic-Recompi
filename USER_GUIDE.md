[English](#english) | [Português (Brasil)](#portugues)

<a id="english"></a>
# User guide

## Prepare your copy

Download the portable Windows EXE from Releases. Place it in a writable folder on any drive; it expands its versioned host files under `CosmicRecomp/app` and uses `CosmicRecomp/game-data` for the shared installation. Avoid protected system folders. Do not mix runtime files from different releases.

Open Settings → Select ISO. Acceptance verifies the supported executable before extraction; it does not prove gameplay compatibility. Then choose Prepare game and wait for completion. The importer reads your ISO without changing it, stages extracted assets and activates only a completed import. It uses an included precompiled runtime, with no local C++ compiler required. Allow approximately 6 GB beyond the ISO and caches. A desktop shortcut is created after import.

Only the Xbox 360 executable revision identified by the importer's embedded SHA-256 check is supported. Other revisions require separate code generation and validation. The exact digest is in `launcher/InstalledGame.cs`; an unsupported ISO must not be forced through the check.

Click Play to start. The standalone OpenGL launcher exits; a small non-rendering bridge owns the installation lock and handles return requests. The native game uses the ReXGlue Direct3D 12 backend. Start directly in game has been removed.

## Video and input

Launcher Settings and the original game's Options → Video share `user/pc-video.ini`. Select 720p, 1080p, 1440p or 4K and 30, 60 or Unlimited FPS. The 30 FPS default uses internal pacing; 60/Unlimited are experimental and may expose cadence/judder problems. A cap is not a performance guarantee. Resolution applies at the next game start; live resolution and extra fullscreen modes are not available in this release. The game overlay counts guest frame submissions and shows frame times; it is not a guarantee that every submitted frame is a unique monitor image.

Gameplay → All Aliens is OFF by default. Hold LT and tap RT to cycle the original four forms, four additional forms and the remaining two. RT cannot activate the ability menu while LT is held. Releasing LT restores the original four slots. Rath is explicitly disabled in this public test, including old saved slots; Four Arms is absent. All Aliens transformations/resource lifetime remain experimental. Turn it OFF to use the baseline roster path.

Quit to launcher requests exit through the bridge and reopens the frontend. It does not guarantee a new save: use the game's own save/checkpoint first. Save/Continue was manually verified in an earlier test, but full-campaign and every death/reload path are not validated.

## Saves, logs and updates

For the portable EXE, saves are under `CosmicRecomp/game-data/user`. For a manually staged ZIP, they are under `installation/user` beside the launcher unless an existing installation path was configured. Settings displays the actual save directory and offers Open saves. Preserve the whole user directory when updating; do not copy only one file from a multi-file save.

Close the game before making a backup. To reset, rename `user` to `user-backup` while it is closed; this preserves a reversible copy. No in-game Wipe data option is promised by this release.

Setup failures: `CosmicRecomp/logs/setup.log`. Game/startup failures: the installation's `logs/launcher.log`, `logs/startup-output.log`, `logs/runtime.log`. Early Windows/D3D failures may occur before runtime.log exists; include launcher.log and startup-output.log in a report. State what you selected, the last visible scene, whether All Aliens was ON, and your GPU/Windows version. Remove personal paths before posting logs publicly.

## Optional translation tables

After import, Settings → Add translation patch accepts a user-supplied `global/scripts/xbx_us.txt` table. The native menu labels are merged into the table; original backups are retained. This is translation-table support, not a universal binary/XEX patch engine. The separately distributed `X360-Ben-10-Ultimate-Alien-Cosmic-Destruction-USA-Traducao-PT-BR-V1.0-XEX-BRASIL` patch is the local compatibility reference; obtain it from its authors. No patch files are hosted here. Compatibility with every revision is not guaranteed.

---
<a id="portugues"></a>
# Guia em português

Baixe o EXE portátil, coloque em uma pasta gravável de qualquer disco e execute. Em Settings → Select ISO, selecione sua cópia compatível. Depois da aceitação, clique em Prepare game e aguarde o progresso. O programa importa os dados para um runtime já compilado; não compila C++ no PC do usuário nem altera a ISO. Após preparar, clique em Play. O launcher fecha e o jogo abre em processo separado. Um atalho é criado após a importação.

As configurações de vídeo ficam tanto no launcher quanto no menu original Options → Video. Há 720p/1080p/1440p/4K e 30/60/Unlimited FPS. O padrão é 30, com limite interno. 60/Unlimited são experimentais e podem apresentar problemas de cadência; a resolução exige reinício. O contador mostra submissões de quadros do jogo, sem garantir imagens únicas no monitor.

Gameplay → All Aliens começa desligado. Segure LT e toque RT para alternar 4 + 4 + 2 formas. Enquanto LT estiver pressionado, RT não aciona as habilidades. Soltar LT restaura os quatro slots originais. Rath está desativado nesta versão, mesmo para saves que o selecionaram. Quatro Braços não está incluído. As transformações e o ciclo de recursos do All Aliens ainda são experimentais.

No pacote portátil, preserve `CosmicRecomp/game-data/user` ao atualizar. Settings mostra a pasta real e permite abri-la. Feche o jogo antes de copiar os saves. Para resetar sem perder a cópia, renomeie a pasta user para user-backup. Quit to launcher não substitui o save/checkpoint do próprio jogo.

Logs da preparação ficam em `CosmicRecomp/logs`. Logs do jogo e da tentativa de iniciá-lo ficam em `CosmicRecomp/game-data/logs`. Envie launcher.log e startup-output.log mesmo quando runtime.log estiver vazio. Informe a ação, a fase, se All Aliens estava ligado e a versão do Windows/GPU. Remova dados pessoais antes de publicar os logs.

Settings → Add translation patch aceita a tabela `global/scripts/xbx_us.txt` fornecida pelo usuário. Não é um instalador universal de patches binários ou XEX. A tradução PT-BR XEX-BRASIL foi usada como referência local; obtenha o patch com seus autores. Nenhum arquivo da tradução é distribuído pelo projeto. Consulte as limitações em RELEASE_NOTES.md.
