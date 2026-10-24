[English](#english) | [Português (Brasil)](#portugues)

<a id="english"></a>
# Alpha 0.4.0-alpha.2 — animation and lever crash fixes

**Experimental Alpha. Bring your own supported game copy.**

- Include existing level-resident animation clips when preparing stream banks.
  A missing Paris bird idle animation reproduced the null-resource crash path.
- Reconstruct converted clip timing with the original game initializer's
  algorithm, addressing incorrect Rath and other converted animations.
- Migrate exact previous converter outputs, preserve original backups and reject
  unknown bank edits. Original streamed clips retain priority and their bytes.
- Record native/guest fault details in normal runs without suppressing faults.
- Preserve the reviewed heap and input patches in consumer rebuilds.

The user tested the first lever in phase 1 and reported no crash. Eleven forms
were visually reviewed; Rath movement/jump/attack input and corridor replays
exited normally. 2,476 clip timing cases match the original generated initializer.
These are bounded checks, not full-campaign certification.

**Known issues:** flicker/judder above 30 FPS, stable 60 FPS, extended combat and
the older first-boss death report remain unverified. Rath upgrade availability
with All Aliens disabled is a separate reported issue, pending investigation.
The default remains 30 FPS. No new performance gain is claimed.

Keep your `CosmicRecomp/game-data/user` folder when updating. No ISO, XEX,
original assets, generated game sources, translation patch or saves are bundled.
See docs/ANIMATION_HOTFIX.md for the cause, original routines and test limits.

---
<a id="portugues"></a>
# Alpha 0.4.0-alpha.2 — animações e crash da alavanca

Inclui animações residentes da fase que faltavam nos bancos convertidos e
reconstrói seus tempos usando o algoritmo original do jogo, corrigindo a causa
identificada das animações incorretas do Rath. Preserva bancos originais,
checkpoints e os patches anteriores de heap e controles.

O usuário testou a primeira alavanca da fase 1 sem crash. Onze formas foram
conferidas visualmente e 2.476 casos de tempo coincidiram com o inicializador
original. Isso não comprova a campanha completa nem todas as habilidades.

**Ainda é Alpha.** Tremedeira acima de 30 FPS, 60 FPS estáveis e o relato antigo
de crash ao morrer no primeiro chefe continuam pendentes. O bloqueio de upgrades
do Rath com All Aliens desligado será investigado depois desta publicação.
Não há ganho novo de FPS anunciado. Preserve `CosmicRecomp/game-data/user`.
É necessário fornecer sua própria cópia; nenhum dado original ou save é incluído.
