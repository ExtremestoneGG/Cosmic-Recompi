[English](#english) | [Português (Brasil)](#portugues)

<a id="english"></a>
# v0.3.0-test.1 — launcher, original game menus and Rath exclusion

This is an experimental public test requested with Rath disabled. It does not contain a fix for his transformation resource crash. Rath is removed from the All Aliens roster and guarded in all four original transformation input paths, including profiles that already selected him. His menu unlock is kept OFF. Other ordinary forms remain experimental, with pages of 4 + 4 + 2 for a normal four-form preset.

The standalone launcher uses its own OpenGL-enabled SDL build, exits before gameplay, always requires Play and has corrected menu spacing/highlights. ISO acceptance and preparation are separate steps. The portable EXE extracts only host binaries and notices beside itself; user assets and saves use the shared game-data directory. A desktop shortcut is created after import. Startup failures are logged before the game runs.

Video settings use the original Ben 10 menus, with resolution presets and 30/60/Unlimited limits; the launcher edits the same settings. Native 720p remains the minimum. Resolution changes still require restart. Experimental live-resolution/fullscreen work is not included. Quit to launcher and user-supplied translation tables are included; no translation or background video is bundled.

All Aliens reserves RT for paging while LT is held, restores the original preset when LT is released, and uses the original HUD and selection-sound call. Audible sound, all transformations, mission transitions, death/reload and full-campaign compatibility are not established by the policy/unit tests. An automated extra-form selection also reproduced a missing animation resource associated with Armodrillo; the user reported other forms working. This remains an open issue, not a blanket validation of the roster. Keep All Aliens OFF for the baseline path. No new FPS gain or stable 60 FPS claim is made. The earlier first-boss death failure remains unverified.

Verification records distinguish automated exclusion/installer checks, limited native-game probes and earlier user-confirmed Save/Continue. SDK/GPU binaries retain the v0.2.1 accepted hashes. The consumer adds the native input filter and a 64 MiB DB File Heap budget; corresponding authored integration and SDK sources are supplied. No generated game source, game assets or personal information is shipped.

---
<a id="portugues"></a>
# v0.3.0-test.1 — launcher, menus do jogo e Rath desativado

Versão pública experimental solicitada com o Rath desativado. O crash da transformação dele não está corrigido: ele foi retirado do All Aliens e bloqueado nos quatro caminhos de seleção, inclusive para saves antigos. As páginas ficam em 4 + 4 + 2 para o grupo inicial normal.

Inclui ajustes do launcher, Play obrigatório, seleção/validação da ISO separada da preparação, pacote em um executável, atalho após importação, logs iniciais, menus Video do próprio Ben 10, Quit to launcher e importação de tabela de tradução fornecida pelo usuário. O launcher fecha antes do jogo. Nenhum patch ou vídeo de fundo está incluído.

LT + RT troca páginas sem acionar as habilidades por RT enquanto LT estiver segurado. Um teste automático também reproduziu recurso ausente ao selecionar Armodrillo; o usuário relatou os outros funcionando. Essa divergência continua em investigação. All Aliens começa desligado e continua experimental; todas as transformações, mortes e mudanças de fase ainda precisam de testes. Resolução ainda exige reinício; os limites 60/Unlimited não comprovam 60 FPS estáveis. O crash anterior ao morrer no primeiro chefe permanece sem confirmação de correção. Save/Continue já foi confirmado manualmente em teste anterior.

Os binários do SDK/GPU mantêm os hashes aceitos da versão anterior. O executável do jogo recebe o filtro de entrada e orçamento de 64 MiB para DB File Heap. As origens, alterações próprias, testes e limitações estão documentados. Não há dados do jogo, código gerado do jogo nem saves pessoais no download.
