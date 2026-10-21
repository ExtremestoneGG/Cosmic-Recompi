[English](#english) | [Português (Brasil)](#portugues)

<a id="english"></a>

# User guide

## First run

Extract the complete release ZIP. Start `CosmicRecomp.exe`; do not start the runtime executable manually. Settings contains **Installation folder**, **Select ISO** and **Open saves**. Choose a writable destination before importing if you do not want the default `installation` directory beside the program.

Select your own Xbox 360 ISO. The importer reads it without modifying it, checks the supported executable revision and extracts resources into a new staging directory. Only a completed import becomes the active game directory. An existing import is preserved rather than overwritten. A failed import can leave a partial staging directory for inspection; it is not considered playable.

The package includes the compiled native program for one recognized revision. Import does not download a game or install a compiler. Other revisions need their own analysis, code generation and validation.

## Launcher controls

Use mouse, keyboard navigation or a supported gamepad for Play, Settings and Quit. File and folder selection use Windows dialogs. **Start directly in game** remembers your preference beside the launcher. When enabled, opening the program starts the recognized installation directly, without creating the frontend graphics context.

The frontend exits completely before the game starts. A small bridge waits without rendering to own the installation lock and handle return requests. The game runs in its own process using Direct3D 12.

## Video settings

Open **Start → Options → Video**. Resolution presets are 1280×720, 1920×1080, 2560×1440 and 3840×2160. These are the SDK's guest-video-mode/startup-window presets; do not assume they independently implement a new internal supersampling renderer. No preset below the game's native 1280×720 is offered.

Choose 30 or 60 FPS and select **Apply**. The cap takes effect through the internal frame pacer. Resolution is saved for the next start. **Uncapped** is visible but unavailable. Use Back or the controller's cancel button to return to the game's Options menu.

30 FPS is the default. The 60 FPS setting is experimental: it does not guarantee sufficient rendering performance or fix all camera instability above 30 FPS. The FPS overlay measures guest game submissions, not repeated desktop/window presents. It is normal for the game counter to stop while the native video dialog deliberately pauses guest menu execution.

## Leaving the game

The original **Quit** action keeps its original meaning. **Quit to Launcher** is a separate nearby action; confirm the game's warning to return to the frontend. Save/checkpoint first if you want to preserve recent progress. Return forces the frontend to appear even if direct-start is enabled.

## Storage

| Location | Purpose |
|---|---|
| `<installation>/game` | Imported original resources; local menu extensions and their original backups |
| `<installation>/user` | Original game saves and PC preferences |
| `<installation>/user/pc-video.ini` | PC resolution and cap only |
| `<installation>/cache` | Regenerable runtime/shader cache |
| `<installation>/logs/runtime.log` | Local diagnostic log |
| `<program>/installation-path.txt` | Chosen installation path |
| `<program>/launcher-preferences.ini` | Direct-start preference |

Settings displays the actual save path. Preserve `user` when updating or moving resources. Changing the selected folder does not automatically move or delete the old installation.

To reset game progress, close the game and rename `user` to `user-backup`. Keep the backup. To reset only PC video settings, rename `pc-video.ini` instead. Never upload your installation or saves with a bug report.

## Updating

Extract a new release into a separate program directory, open Settings and select the existing installation. Keep the old program until the new one works. Do not mix runtime DLLs or executables from different releases. Runtime checksums detect incomplete or mixed packages; they are not DRM and may be regenerated when intentionally rebuilding dependencies.

---

<a id="portugues"></a>

# Guia de uso

Extraia todo o ZIP e abra `CosmicRecomp.exe`. Em Settings, escolha uma pasta gravável e selecione sua própria ISO. A edição é verificada antes da importação. Uma instalação existente é preservada; arquivos parciais de uma falha não viram uma instalação pronta.

Play inicia o jogo e encerra o processo gráfico do launcher. Uma ponte pequena, sem renderização, aguarda o encerramento e trata o retorno. **Start directly in game** permite pular a interface nos próximos inícios. As escolhas de arquivos e pastas usam os diálogos do Windows.

No jogo, use **Start → Options → Video**. Apply salva resolução e limite de 30/60 FPS. A resolução é aplicada no próximo início; o limite usa o temporizador interno. Ilimitado está desativado. Os presets configuram o modo de vídeo/janela do SDK; não representam a criação de um renderer próprio de supersampling. A menor opção é 1280×720.

30 FPS é o padrão. 60 continua experimental e não garante desempenho ou estabilidade completa da câmera. O contador considera submissões do jogo, sem contar repetições da janela.

**Quit to Launcher** fica separado de **Quit**. Confirme o aviso do jogo após salvar seu progresso. O launcher reaparece mesmo com início direto marcado.

Saves ficam em `<instalação>/user`; Settings mostra o caminho completo. `pc-video.ini` contém apenas as preferências de vídeo do PC. Para resetar, feche o jogo e renomeie a pasta/arquivo que quiser resetar, guardando a cópia. Atualize extraindo o programa separadamente e selecionando a instalação existente. Não misture DLLs de versões diferentes nem envie recursos ou saves nas issues.
