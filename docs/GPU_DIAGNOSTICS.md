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

### Download and prepare

1. Under **Assets** on this release page, download
   **`CosmicRecomp-v0.4.0-alpha.4-gpu-test.4-win64.exe`**. You do not need the
   source ZIP to perform this test. Do not use GPU test 1 or 2 for this capture.
2. Close any running copy of the game. Run the downloaded EXE and choose a
   **new installation folder**, separate from your regular installation. Keep
   your existing installation and saves intact. Do not copy individual DLLs.
3. In the launcher, select your own supported Xbox 360 ISO and prepare the game.
   Wait for preparation to finish. Then click **Play** normally; there is no
   special keyboard shortcut or external profiler to start.
4. A fresh installation automatically selects the required `measure-phases`
   diagnostic profile. If you reused a folder with
   `game-data/graphics-diagnostic-profile.txt`, change that file's entire
   contents to `measure-phases` **before launching**. Otherwise a previous
   profile may disable the new CPU log.

### Reproduce the low FPS

5. Set **Resolution: 720p**, **Frame limit: Unlimited**, and **Display mode:
   Windowed** in Video settings. These match the previous submitted capture.
   Keep external FPS limiters disabled for this diagnostic. No driver changes
   or tool installation are needed. Known camera flicker above 30 FPS is not
   fixed by this test build.
6. Load the **same gameplay area where you previously saw 15–20 FPS**. A title
   screen, menu, loading screen or cutscene is not a useful substitute.
7. Leave the character and camera still in that slow area for **at least
   30 seconds**. The timing files update automatically, roughly every five
   seconds. Do not change resolution, FPS limit or display mode during capture.
   No recording, screenshot or manual capture button is required.
8. Close the game normally. **Do not launch it again before copying the logs.**

### Send the results

9. Open the installation folder you selected in step 2, then **`game-data/logs`**.
   Copy the following files into a ZIP:

   ```text
   launcher.log
   startup-output.log
   runtime.log
   gpu-timing-*.jsonl
   cpu-draw-phases-*.jsonl
   ```

   The asterisk means the automatically generated date/ID in the filename;
   include the actual matching files, not a file literally named with `*`.
10. Send that ZIP and a short note identifying the level/area you tested and
    the approximate FPS you saw. Do not include the ISO, extracted game files,
    saves, cache or the entire installation folder. You may redact personal
    paths while preserving timing data and error messages.

**If `cpu-draw-phases-*.jsonl` is missing:** send the three ordinary logs anyway
and tell us it was missing. Check that you used GPU test 4, stayed in actual
gameplay for 30 seconds, and did not retain an older profile file. Do not rebuild
the project, install profiling software or start replacing DLLs to work around
the missing file.

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
