# Experimental RTX 4060 investigation — Alpha 4 GPU test 1

**Possible fix / diagnostic test, not a confirmed performance improvement.**
This package targets a report of 15–20 FPS on an Intel Core i7 14th Gen HX,
NVIDIA RTX 4060 Laptop GPU and 32 GB DDR5. We do not have that hardware locally.
The supplied logs confirmed use of the RTX 4060, 1080p output and a 30 FPS limit.
They contain no per-frame timings and show normal exits, not the separate source
build failure. No percentage improvement is claimed.

## What changes

The default experimental profile disables the SDK's CPU draw-extent estimator,
using its existing fallback. This tests whether CPU-side vertex evaluation limits
performance on this system. It may increase GPU transfer work or cause a visual
regression; neither benefit nor visual equivalence is assumed. No resolution,
texture quality, game speed, frame generation or gameplay logic is changed.
The complete EXE/runtime/GPU set is identical to Alpha 4. Only the C# process
bridge changes. Do not replace individual native DLLs.

Install into a **separate folder** and import your own supported ISO. Choose
720p for comparison with the development target; keep the same scene, camera,
display mode, FPS limit and power conditions between profiles. First test at
30 FPS; an optional uncapped comparison may show the known above-30 camera
flicker. That flicker is not fixed here. Preserve saves before testing.

## Select one profile

Close the game. In the selected installation's `game-data` folder, create the
plain text file `graphics-diagnostic-profile.txt` with exactly one of these:

| Text | Change relative to official Alpha 4 |
| --- | --- |
| `gpu-bounds` | CPU draw-extent estimator off; experimental default if file is absent |
| `standard` | Official Alpha 4 graphics policy; comparison baseline |
| `viewport` | Only the viewport workaround off; CPU estimator remains on |
| `stencil` | Only the stencil workaround off; CPU estimator remains on |

These are independent tests, not cumulative optimizations. Start with
`standard`, then `gpu-bounds`. Try the other two only if needed. Return to
`standard` if geometry, shadows, transparency or lighting changes. Reopen the
game after changing profiles. The standard public build ignores this file.

Send FPS observations from the same scene for each profile, any visual defects,
and `game-data/logs/launcher.log`, `startup-output.log`, `runtime.log` copied
before the next launch. The launcher records the selected profile; the native
runtime records the resulting setting. Do not publish personal paths in issues.

## Validation boundaries

The bridge compiles and automated tests check independent profile selection,
the experimental default, and rejection of unknown profiles. Native runtime
hashes are checked against Alpha 4. Local startup results are recorded separately.
There is no RTX 4060 benchmark yet, no proven FPS gain and no clean-machine full
SDK rebuild claim. A separate host-timer test showed one 38.716 ms interval
(33.436 ms mean); its strict jitter gate failed. This package changes no timer
code and does not claim that issue is resolved.

Source build: use the pinned SDK and complete corresponding sources from
Alpha 4, with this release's consumer source snapshot. Build the bridge with
`python tools/build_release_bridge.py --hero-time --gpu-diagnostics`.
The regular command without `--gpu-diagnostics` preserves normal behavior.

## Português

Versão experimental para investigar 15–20 FPS na RTX 4060 Laptop. Não há ganho
confirmado. Instale separadamente, forneça sua ISO e compare a mesma cena em
720p. O perfil padrão experimental desliga a estimativa de limites na CPU;
o caminho original do SDK continua disponível. Pode haver regressões visuais.
Crie `game-data/graphics-diagnostic-profile.txt`: `standard` reproduz a política
da Alpha 4; `gpu-bounds` testa a candidata. `viewport` e `stencil` são testes
independentes. Feche e reabra o jogo entre perfis. Envie os três logs e suas
observações. Flicker acima de 30 FPS e validação na RTX 4060 seguem pendentes.
