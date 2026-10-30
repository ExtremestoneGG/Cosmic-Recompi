# Continue this project instead of starting over

This document is the handoff for a contributor's coding agent. The authoritative
public baseline is Alpha 4, with All Aliens and Hero Time. GPU test 4 is a
diagnostic derivative; it does not establish a performance fix.

## What already exists

| Feature | Authored source / integration |
| --- | --- |
| Separate launcher, import and startup logging | `launcher/native`, `launcher/NativeBridge.cs`, `launcher/InstalledGame.cs`, `launcher/IsoImage.cs` |
| Original in-game Video/Extras menus | `launcher/GameMenuPatch.cs`, `native-rex/src/pc_menu_bridge.h` |
| All Aliens and safe resource loading | `launcher/AllAliensPatch.cs`, `tools/prepare_all_aliens_candidate.py`, `tools/merge_alien_streams.py` |
| Hero Time | `tools/prepare_hero_time_candidate.py`, `tools/hero_time_rules.py`, bridge compile defines/resources |
| Heap and boss callback fixes | `tools/apply_game_integration.py`, `tools/apply_callback_capacity.py`, `tools/apply_resource_diagnostics.py` |
| Native GPU/runtime changes | `sdk-patches/rex-sdk-local.diff` against the pinned SDK |
| Saves, reset and translation import | `launcher/SaveReset.cs`, bridge/menu sources |

All Aliens and Hero Time default to **OFF**. Look under **Options → Gameplay →
Extras** to enable them. Hold LT and tap RT for alien pages; Hero Time uses a
compatible normal alien followed by L3 / F. Ultimate duration is up to 60 seconds,
with proportional recharge as Ben (used time / 3, maximum 20 seconds). Four Arms
is absent. Above-30-FPS flicker, some graphics issues and Rath upgrades remain
open. Do not advertise the campaign as fully validated.

Game scripts are patched locally from the owner's ISO. Their original bytes and
generated C++ are intentionally absent from Git. Their absence is not permission
to replace existing features with fresh SDK boilerplate.

## Obtain and build the current source

1. Record your current changes. Fetch upstream and inspect the branch/revision;
   do not reset or overwrite a contributor's work. A fork must explicitly fetch
   `https://github.com/ExtremestoneGG/Cosmic-Recompi.git`; downloading a release
   installer does not update a fork's sources. Read `AGENTS.md`.
2. Follow `CONTRIBUTOR_BUILD_RUNBOOK.md` sections 2–4: x64 developer terminal,
   tools, SDK commit `c94f5ebdcb3c9d1a460ca48e04f9758448f8d518`, consolidated patch,
   Release SDK installation and your own supported ISO extracted to `originals`.
   Pin `ExtremestoneGG/recomp-ui` to
   `047bc82d8679e96d1dec19ce2cd8ab677003465f` as documented.
3. From the consumer root, with your actual installed Python interpreter:

   ```powershell
   ./tools/build_development.ps1 -Python python -Regenerate
   ```

   Use `-SdkPrefix <installed-sdk-directory>` if different from
   `toolchain/rexglue-sdk`. This helper does not clone/install tools or build the
   SDK. It generates title C++, applies the authored integration, builds Release,
   builds the **Hero Time** bridge and stages matching local output.
4. For subsequent source edits omit `-Regenerate` unless codegen input changed.
   The generated integration is idempotent. Never run `rexglue init` here.
5. Launch **`build/development-app/CosmicRecomp.exe`** and import your ISO into a
   disposable installation. Do not launch an older desktop shortcut or installer.
   Inspect `DEVELOPMENT_BUILD.json` for commit, dirty status, profile and runtime
   hashes. It describes staged bytes, not proof that a feature passed gameplay.

## Why an old or broken build can appear

- A stale fork/branch does not contain upstream changes.
- Older revisions of `build_release_bridge.py` **without `--hero-time`** chose a
  legacy settings profile. This revision fixes the default to Hero Time; explicit
  `--hero-time` remains accepted. Do not use the legacy `--rath-disabled` profile.
- Rebuilding only the game EXE leaves an older bridge, which owns the authored
  script resources/menu patching. Rebuilding only the frontend also is insufficient.
- Copying release DLLs into a newly built SDK/consumer may cause ABI failures.
- Running an old shortcut can start another installation, even after a good build.
- Missing generated files are expected before codegen; skipping the integration
  recipe loses heap/input/callback corrections. Stop if the recipe rejects input.
- Both optional gameplay features start OFF; this alone is not an old build.

These are failure modes, not a confirmed diagnosis of your machine's rebuild loop.
Preserve the first compiler error and actual commands rather than an AI summary.

## Stop the rebuild loop

Work in stages and stop at the first nonzero native exit code. Record stdout,
stderr, exact command, consumer commit, SDK commit/patch hash and compiler version.
Fix that first error before retrying. Do not repeatedly delete output, regenerate
everything, upgrade dependencies, disable features, fake exports or mix DLLs.
Do not claim a successful build if a later step launched an older executable.
No ISO, original assets, generated title code, personal paths or saves may be pushed.

Validate title/startup first, save/Continue second, then All Aliens and Hero Time
in gameplay. Only afterwards change performance code, one hypothesis at a time.
Logs normally live under the selected installation's `game-data/logs` (older
installs: `installation/logs`). Preserve all three startup/runtime logs before
another launch. For the RTX investigation follow `GPU_DIAGNOSTICS.md`.

The submitted RTX 4060 timing points to costly graphics-worker CPU work (~54 ms
per frame in the slow subset), with guest GPU commands around 14 ms. The GPU is
selected correctly. This does not identify the expensive function; GPU test 4's
CPU phase capture is pending. Do not assume unused hardware or a proven fix.

## Validation limits

The bridge can be rebuilt from public sources. A complete fresh-machine SDK,
codegen, consumer and gameplay reproduction has **not** yet been verified.
Local private experiments and the debug room are deliberately not public baseline
features. Do not try to reconstruct them from screenshots or invent missing
history. Report verified results, hypotheses and untested behavior separately.
