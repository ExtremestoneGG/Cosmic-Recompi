[English](#english) | [Português (Brasil)](#portugues)

<a id="english"></a>
# v0.4.0-alpha.1 — settings and transformation fixes

**Alpha software.** This is an experimental build with limited scene-level validation. It is not a complete or fully verified port.

## Added and corrected

- Converted missing resident animation clips into the game's streamed clip format. This addresses the null-resource and invalid-dispatch failures reproduced while selecting extra All Aliens forms, including Rath, Armodrillo and Terraspin. All eleven ordinary forms transformed in the recorded Paris corridor. Original level clips remain unchanged in the merged banks.
- Kept normal Rath progression and the original cheat instead of forcibly unlocking him in the profile. All Aliens still exposes Rath without changing the saved unlock flag.
- Reserved RT for page changes while LT is held, preventing RT abilities from obscuring the form wheel. Releasing LT restores the normal input path.
- Added a single **Show FPS → On/Off** setting for the counter and frame-time graph, both in the game's Video menu and launcher settings.
- Added live presentation/output size and windowed, borderless and supported exclusive fullscreen choices through the SDK's SDL window. Borderless uses desktop dimensions. This is not internal supersampling or a new renderer.
- Verified the game-to-launcher process handoff. Existing checkpoint saves are preserved; quitting is not a promise to save an arbitrary point in a mission.
- Added **Wipe Data** with the game's confirmation dialog. After the process exits, this title's profile data is moved to a recoverable backup before returning to the launcher. Other titles, native video settings and original installations are preserved.
- Kept the separate Play/Settings/Quit frontend, ISO validation/preparation, startup logs and user-supplied translation-table import. No patch, background video or original game data is bundled.

## Next priority: flicker above 30 FPS

Camera/scenery movement can flicker or judder with the frame limit above 30 FPS, including Unlimited. **This Alpha does not fix that issue.** The next investigation runs with the frame limit disabled and targets update/presentation cadence. The default remains 30 FPS. Selecting 60 or Unlimited does not guarantee smooth motion or matching performance.

No new FPS improvement is claimed in this release. Full campaign coverage, extended combat/abilities, the earlier first-boss death failure, and B returning to Press Start remain pending. The Windows 10 compatibility of every GPU/driver combination is not established. Use the startup logs when reporting a launch failure.

The SDK remains pinned to ReXGlue v0.10.0. The runtime adds a narrow SDL presentation extension; the accepted GPU algorithms are unchanged. The single SDK diff, corresponding sources, revisions, tests and limits are documented in BUILD.md, SOURCES.md, PROVENANCE.json and VALIDATION.json. A complete clean-machine rebuild remains unverified.

---
<a id="portugues"></a>
# v0.4.0-alpha.1 — configurações e transformações

**Versão Alpha:** experimental, com validação em cenas específicas. Não é um port completo ou totalmente verificado.

Corrige recursos de animação ausentes nas transformações extras do All Aliens, incluindo Rath, Armodrillo e Terraspin. As onze formas comuns foram transformadas no corredor de Paris. O desbloqueio normal do Rath e seu cheat são preservados, e RT fica reservado à troca de páginas enquanto LT estiver segurado.

Inclui **Show FPS** para mostrar/esconder juntos contador e frametime, resolução de saída sem reiniciar, modos de janela/borderless/exclusivo compatíveis e retorno ao launcher. A resolução interna não é aumentada por essas opções de saída. Wipe Data usa confirmação do jogo e guarda backup antes de retornar ao launcher. Sair preserva os checkpoints existentes; não garante salvar um ponto arbitrário da missão.

**Próxima prioridade: o flicker/tremedeira acima de 30 FPS. Este Alpha ainda não o corrige.** A investigação seguinte usa FPS liberado. O padrão continua em 30 FPS; escolher 60 ou Unlimited não garante fluidez ou desempenho correspondente.

Não há ganho novo de FPS anunciado. Campanha completa, habilidades prolongadas, o crash antigo ao morrer no primeiro chefe e B para voltar ao Press Start continuam pendentes. Os logs iniciais ajudam a investigar falhas antes da abertura do jogo. Nenhuma ISO, dado original, fonte gerado do jogo, patch de tradução ou save pessoal é distribuído.
