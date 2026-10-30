# GPU test 5: investigate the apparent 15 FPS lock

This is a passive diagnostic, not a claimed fix or FPS optimization. Graphics,
guest refresh frequency, limiter algorithm and original game logic are unchanged.
It measures where time goes at the existing game-frame submission hook.

## Test on the affected RTX 4060 laptop

1. Install the complete GPU test 5 EXE into a separate folder. Import your own
   supported ISO. Preserve your normal installation and saves; do not mix DLLs.
2. Leave the diagnostic profile at its fresh-install default `measure-phases`.
   If you reused a data folder, put `measure-phases` in
   `game-data/graphics-diagnostic-profile.txt` before launching.
3. Disable external FPS caps for this test. Use 720p, Windowed, and frame limit 30.
4. Enter the affected gameplay area. Keep Ben and the camera still for 30 seconds.
5. In the game's Video menu choose Unlimited, return to the same view and leave
   it still for another 30 seconds. Known above-30-FPS flicker remains unresolved.
6. Close normally. Before another launch, ZIP only these files from game-data/logs:
   launcher.log, startup-output.log, runtime.log, gpu-timing-*.jsonl,
   cpu-draw-phases-*.jsonl, frame-cadence-*.jsonl, vblank-*.jsonl.
7. Include the area and FPS before/after the setting change. Do not send your ISO,
   extracted game files, saves or entire installation.

If you cannot reach gameplay, send the three ordinary logs. No source rebuild,
profiler installation or driver modification is required for this test.

## What the files distinguish

- `limiter_wall_ms`: time spent inside our existing limiter call.
- `registration_wall_ms`: native menu registration at the frame hook.
- `outside_hook_wall_ms`: everything between hook exits and following entries,
  including game work, graphics backpressure, guest synchronization and scheduling.
- `rate_frames_0_30_60`: actual selected limiter rate per observed frame. Mixed-rate
  windows must not be treated as a single 30/unlimited comparison.
- `vblank`: guest interrupt cadence, callback cost and worker sleep behavior.
- CPU phase / GPU timing: the existing graphics attribution.

The 60-Hz-multiple histogram is a correlation test, not proof of VSync causality.
These are wall-time scopes, not interchangeable thread CPU or GPU measurements.
Clock-window boundaries and instrumentation overhead limit precision. Compare
stable gameplay intervals only; menus, movies and loading are not FPS benchmarks.

Local muted startup verified the observer with both rate 30 and rate 0, normal
exit, and guest-vblank log generation. Local startup cannot validate the RTX
problem or prove a fix. The regular Alpha download is unchanged.
