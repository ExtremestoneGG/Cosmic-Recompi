# Alpha 4 GPU test 2 — automatic CPU/GPU timing

**Diagnostic build, not a confirmed performance fix.** GPU test 1 did not
resolve the collaborator's reported performance problem. Its logs confirmed
the RTX 4060 Laptop, CPU bounds disabled, 1080p and unlimited FPS. No actual
frame timings were available. This build collects them automatically.

Install separately and provide your own supported ISO. Play in the slow scene
for at least 30 seconds, then close normally. Send the contents of
`game-data/logs`: the three normal logs plus the new `gpu-timing-*.jsonl` file.
Copy them before the next run. Do not attach your ISO or save data.

The default is `measure-standard`: official Alpha 4 graphics policy with GPU
timestamp and graphics-worker timing collection. The native EXE, GPU and runtime
are unchanged. Only the process bridge differs. Queries can add overhead; this
is bottleneck attribution, not a clean benchmark or an FPS gain claim.

The JSONL records game-frame intervals, FPS, p50/p95/p99, graphics-worker CPU,
draw/submit/fence wall times, and guest-command GPU time. Wall scopes overlap:
do not sum them. GPU time excludes presentation/idle gaps and is not GPU usage
percentage. Graphics-worker CPU is not the whole game's CPU time. Preserve raw
files and identify the gameplay portion; menu/startup samples are not comparable.

For a comparison with the native-720p development target, choose 720p in Video.
This does not establish that 1080p is the cause. Keep the same scene, camera,
display mode and FPS setting; report them. The tested hardware is an i7 14th Gen
HX, RTX 4060 Laptop and 32 GB DDR5. Remote timing results remain pending.

Optional file `game-data/graphics-diagnostic-profile.txt` accepts:
- `measure-standard`: default, official graphics policy plus timing.
- `measure-gpu-bounds`: only CPU draw-extent estimation off, plus timing.
- `standard`: official policy without this timing collector.
- `gpu-bounds`: CPU draw-extent estimation off without timing.
- `viewport` or `stencil`: independent workaround tests, no timing collector.

Close/reopen between profiles. Test 1's previous `gpu-bounds` profile file, if
present, overrides the new default: remove that file or write `measure-standard`.
Only this diagnostic package reads it. No save data is modified by the selector.
Normal gameplay still saves through the original game. Unknown profile names
are rejected instead of silently selecting a different test.

The bridge tests cover isolation, timing-path selection and rejected inputs.
Local startup, timing-file generation and installer checks are recorded in the
separate VALIDATION asset. No RTX 4060 performance gain or complete clean-machine
SDK rebuild is claimed. Known above-30-FPS camera flicker remains unresolved.
The main Alpha remains the default download. No assets or private test room ship.

Source: use the matching corresponding-source archive and the documented SDK
revision/patch. Build the bridge with `--hero-time --gpu-diagnostics`.
Omit `--gpu-diagnostics` for the ordinary uninstrumented launcher.

## Português

Versão de diagnóstico, sem ganho confirmado. A primeira candidata não resolveu.
Instale separadamente, rode a cena lenta por pelo menos 30 segundos e envie os
três logs mais `gpu-timing-*.jsonl` em `game-data/logs`. O padrão usa a política
gráfica da Alpha 4 e registra tempos de CPU/GPU; consultas podem ter custo.
Se mantiver um arquivo de perfil antigo, escreva `measure-standard` nele.
Compare em 720p com cena/câmera/configurações iguais. Flicker acima de 30 FPS
continua pendente. A Alpha principal não foi substituída.
