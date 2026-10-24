[English](#english) | [Português (Brasil)](#portugues)

<a id="english"></a>
# Alpha validation scope

`VALIDATION.json` records the release gate. It separates unit/fixture evidence, real native-process checks and earlier user confirmation. It does not declare full game compatibility.

| Area | Evidence | Limit |
| --- | --- | --- |
| Animation conversion | Independent C# and Python output matched across eight level stream banks; original clips preserved, dictionary lookup and repeated installation checked | Format and installation checks do not establish every animation's gameplay |
| Transformations | Eleven ordinary forms visually appeared in the recorded Paris corridor; Rath rechecked with the new presentation runtime | Other levels, extended abilities and combat remain open |
| Video menus | Real original-menu callbacks applied Show FPS OFF/ON and window/display choices | Native GPU-only captures omit later host UI/font drawing |
| Live output size | One native PID changed client size 1280×720 → 1920×1080 → 1280×720 | Output/presentation size, not internal supersampling |
| Fullscreen modes | Visible startup samples showed exclusive 1280×720 and borderless desktop 1920×1080 without window captions; in-game mode callbacks accepted | One monitor/driver configuration; unsupported modes are rejected |
| Wipe Data | Opened confirmation, canceled, reopened and confirmed; game exited before archiving and launcher reopened | Disposable copy only; original user saves were never reset |
| Save archive | Six progress files preserved byte for byte, including additional container headers; native video preferences retained | Does not validate every campaign checkpoint |
| Save/Continue | Earlier user-confirmed save, shutdown, restart and Continue recovered the checkpoint | Separate evidence from the reset test |
| SDK runtime | Retained parent relink and recompiled original SDL unit were byte-identical before adding the presentation extension | Full clean-machine rebuild remains unverified |

The ISO parser, installation/save-preservation and menu transformation suites also pass. Branch targets, original Rath progression, the authored Gameplay input guard, idempotence and unknown-edit rejection are checked separately from gameplay.

No new FPS gain, stable 60 FPS, full campaign completion or fix for the earlier first-boss death crash is claimed. Flicker/judder above 30 FPS is the next priority, investigated with the frame limit disabled. Repeated host presents and an FPS average do not prove unique smooth game frames.

Local probes use recorded controller input and mute game audio. Their original assets, saves, raw logs and screenshots are not distributed. The public validation summary contains no personal paths or account data.

---
<a id="portugues"></a>
# Escopo dos testes Alpha

O arquivo VALIDATION.json separa testes de formato/instalação, execução real e confirmação anterior do usuário. Foram verificadas as onze formas na cena de Paris, troca de resolução de saída no mesmo processo, modos de tela no monitor de teste, Show FPS, cancelar/confirmar Wipe Data, backup após encerramento e retorno ao launcher.

Os seis arquivos de progresso copiados mantiveram seus hashes no backup; os saves originais ficaram intactos. O teste de Save/Continue confirmado pelo usuário é uma evidência anterior e separada. As verificações de SDK comparam o relink e a unidade SDL original antes da extensão.

Isso não comprova a campanha completa, todas as habilidades, cada GPU/driver ou o crash antigo no primeiro chefe. Não há ganho novo de FPS nem 60 FPS estáveis anunciados. A próxima prioridade é a tremedeira/flicker acima de 30 FPS, com limite desligado. Nenhum dado original, log pessoal ou save é publicado.
