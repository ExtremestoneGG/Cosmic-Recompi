# GPU test 4 — detailed CPU graphics diagnosis

**Experimental diagnostic; no performance fix or FPS gain is claimed.**
Test 2 captured a slow/high-draw subset at about 15.48 FPS, with approximately
54 ms of graphics-worker CPU per frame and 14 ms of guest-command GPU time.
This points toward CPU-side graphics work; it does not identify an individual
function. The i7 HX / RTX 4060 Laptop is being selected correctly.

This package adds opt-in CPU wall-time totals for shader analysis, primitive
processing, render-target handling, pipeline selection, texture processing,
fixed state, constant/descriptor bindings, vertex residency, memexport and draw
tail. Original render commands, quality and game behavior are retained. The
CPU executable and runtime DLL are unchanged; the GPU DLL contains the scopes.
All three binaries are packaged together with a matching integrity manifest.
The rejected local vertex-batching experiment is **not** included.

## Test instructions

1. Install this complete package into a separate folder and import your ISO.
2. Set 720p and the same FPS/display settings used for test 2.
3. Stay in the slow gameplay scene for at least 30 seconds, then close normally.
4. Send `game-data/logs`: the three normal logs, `gpu-timing-*.jsonl`, and
   **`cpu-draw-phases-*.jsonl`**. Copy these before another launch.

The default profile is `measure-phases`. If you previously created
`game-data/graphics-diagnostic-profile.txt`, put `measure-phases` in that file
or use a fresh separate install. `standard` disables these collectors;
`measure-standard` collects only the old GPU/frame timing. Other test profiles
are retained but are not needed for this capture.

CPU phase rows are sequential wall-time totals per reporting window, including
driver calls and possible thread preemption. They are not per-phase thread CPU
or GPU measurements. Compare timestamps/windows; do not add them to the GPU
time or the overlapping coarse draw scope. Instrumentation has overhead, so
this is attribution, not a clean FPS comparison. Menu samples are not gameplay.

Local muted startup generated every expected phase and closed with exit code
zero. Profile-selector and installer checks are recorded in VALIDATION.
RTX 4060 phase results remain pending. Clean-machine full SDK rebuilding is
still unverified. Flicker above 30 FPS remains open. No frame generation,
resolution reduction, original game assets, saves or private test room ship.
The main Alpha remains the default download.

Corresponding sources include the exact effective SDK command processor,
the consumer phase-audit header, and one consolidated diff against the pinned
SDK. Run the archive include relocation helper; do not apply the patch twice.
Build the bridge with `--hero-time --gpu-diagnostics`. Normal builds omit the
diagnostic flag and do not enable the collectors.

## Português

Diagnóstico experimental, sem ganho confirmado. Os dados apontaram custo alto
na CPU gráfica; agora medimos cada etapa para localizar a causa. Instale
separadamente, teste 30 segundos na mesma cena lenta em 720p e envie a pasta
`game-data/logs`, incluindo `cpu-draw-phases-*.jsonl` e `gpu-timing-*.jsonl`.
Se houver arquivo de perfil antigo, use `measure-phases`. A candidata de
agrupamento de vértices foi rejeitada localmente e não faz parte deste pacote.
Não há redução de qualidade ou alteração dos comandos gráficos.
