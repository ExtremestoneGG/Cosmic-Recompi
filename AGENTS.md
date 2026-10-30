# Instructions for coding agents and contributors

This is an existing native PC port, not a greenfield recompilation. Read
`docs/AI_HANDOFF.md`, `docs/CONTRIBUTOR_BUILD_RUNBOOK.md`, and `SOURCES.md` first.

- Preserve All Aliens, Hero Time, original in-game Video/Extras menus, saves,
  launcher and crash fixes. These are authored source features already present.
- Use the pinned SDK and `sdk-patches/rex-sdk-local.diff`. Never run `rexglue init`
  over this checkout or replace it with newly generated boilerplate.
- Codegen is only for locally owned game input; follow it with
  `tools/apply_game_integration.py`. Do not commit generated game C++ or assets.
- Build the bridge with `--hero-time` for explicit provenance. The unqualified
  command also selects that current profile; `--rath-disabled` is legacy only.
- Prefer `tools/build_development.ps1` after prerequisites and patched SDK install.
  Run `build/development-app/CosmicRecomp.exe`, not an older downloaded installer.
- Stop at the first failing command; save its exact output. Do not repeatedly
  regenerate, change SDK revisions, mix downloaded DLLs or fake API markers.
- A changed SDK class layout requires every dependent translation unit rebuilt.
  Use a new build directory for incompatible compiler/SDK configurations.
- No automatic installs, destructive cleanup, publication or uploads without
  the repository owner's authorization. Never commit personal paths or saves.
- Measure matched gameplay before optimization claims. Host Present counts are
  not game FPS. Keep graphics intact; no frame generation. Above-30-FPS flicker
  and the remote RTX performance problem remain unresolved.
- Report verified results separately from assumptions and pending remote tests.
  A successful compile does not prove gameplay or a clean-machine rebuild.
