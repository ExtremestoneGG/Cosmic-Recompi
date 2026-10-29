# Contributor and coding-agent build runbook

This guide is for changing the native project, not importing an ISO as a player.
The installer extracts locally owned assets and starts a bundled runtime; it
does **not** compile generated C++ on the player's machine.

**Validation status:** the published Alpha 4 runtime works in the development
environment. The complete workflow below has not been independently reproduced
on a clean machine. Do not call it a verified clean-machine build or promise
100% success. Report the first failing command instead of inventing fixes.

## 1. Choose one source identity

Use either the complete corresponding-source archive from the release you are
rebuilding, or a Git checkout with the pinned SDK and consolidated patch.
Do not combine the runtime from one release with a renderer from another.

- SDK base: `c94f5ebdcb3c9d1a460ca48e04f9758448f8d518`.
- Project patch: `sdk-patches/rex-sdk-local.diff`.
- Frontend dependency: `ExtremestoneGG/recomp-ui`, commit
  `047bc82d8679e96d1dec19ce2cd8ab677003465f`.
- Alpha 4 bridge: `--hero-time`, native menu API 7.
- GPU test bridge only: add `--gpu-diagnostics` if intentionally reproducing
  the experimental release. This is not a general performance fix.

Record `git rev-parse HEAD`, `git status --short`, compiler version and CMake
version in your private build log. Keep original game data and generated C++
out of commits, issue attachments and source archives.

## 2. Prepare the terminal

Open an x64 Visual Studio Developer PowerShell with the Microsoft C++ libraries
and Windows SDK available. LLVM/Clang, CMake, Ninja, Git and Python must already
be installed. Do not let an agent install or update tools without authorization.
Tool sources are listed in `SOURCES.md` and `THIRD_PARTY.md`.

Choose any writable checkout directory. No particular drive letter is required.
From the consumer repository root:

```powershell
$ErrorActionPreference = 'Stop'
$root = (Get-Location).Path
$env:TEMP = Join-Path $root 'data/temp'
$env:TMP = $env:TEMP
New-Item -ItemType Directory -Force $env:TEMP | Out-Null
git --version
cmake --version
ninja --version
clang-cl --version
clang++ --version
python --version
```

If a command is missing, fix the tool environment before configuring. A Windows
Store Python alias is not a Python interpreter. Use your installed interpreter's
full path if necessary, consistently throughout the build.

PowerShell's `$ErrorActionPreference` does not reliably turn native nonzero exit
codes into exceptions. After **every** native command below inspect
`$LASTEXITCODE`; stop immediately if it is nonzero. Save complete stdout/stderr.
Do not continue to staging after a failed build.

## 3A. SDK from a pristine Git checkout

Do this only if `third_party/rexglue-sdk` does not already exist:

```powershell
git clone https://github.com/rexglue/rexglue-sdk.git third_party/rexglue-sdk
git -C third_party/rexglue-sdk checkout c94f5ebdcb3c9d1a460ca48e04f9758448f8d518
git -C third_party/rexglue-sdk submodule update --init --recursive
./tools/apply_sdk_patch.ps1
```

The helper checks applicability and relocates consumer includes. If the patch
check fails, inspect the SDK revision and diff. It may already be applied.
Never force it, silently discard changes, or repeatedly apply it.

```powershell
Push-Location third_party/rexglue-sdk
cmake --preset win-amd64 "-DCMAKE_INSTALL_PREFIX=$root/toolchain/rexglue-sdk"
cmake --build --preset win-amd64-release --parallel 2
cmake --install out/build/win-amd64 --config Release
Pop-Location
```

`win-amd64` is the configure preset; `win-amd64-release` is the build preset.
Explicit Release matters because the multi-configuration default may be Debug.
Use a new build directory when changing compiler or ABI. Do not erase someone
else's build directory to resolve a configuration mismatch.

## 3B. SDK from a complete corresponding-source archive

The archive contains sibling `rexglue-sdk` and `consumer` trees. The patch is
**already applied**. At the archive root run:

```powershell
python prepare_sdk_source.py --sdk rexglue-sdk --consumer consumer
```

Then configure/build/install the SDK from its own directory using the same
Release preset sequence. Set the install prefix to the consumer's
`toolchain/rexglue-sdk` directory. Return to `consumer` for the remaining steps.
Do not clone another SDK inside this archive or apply the patch again. Include
relocation and source completeness checks are not proof of a full compilation.

## 4. Prepare your own supported game files

The manifest expects the supported executable at `originals/default.xex`, with
the extracted game filesystem in `originals`. See `native-rex/ben10_manifest.toml`
and the edition checks in the importer. The ISO itself is not a codegen input.

Use the normal importer to extract your own supported ISO into a disposable
installation, or the documented `InspectIso.exe --extract <new-folder>` flow.
Do not extract into a save directory or overwrite an existing install. Match the
manifest's filesystem layout; do not point codegen at patched menu scripts or a
different-region XEX. Keep the original ISO intact.

## 5. Generate and integrate the title

From the consumer root:

```powershell
& "$root/toolchain/rexglue-sdk/bin/rexglue.exe" codegen native-rex/ben10_manifest.toml
python tools/apply_game_integration.py --generated native-rex/generated/default
```

Stop on either failure. The second command is required: it applies the authored
Alpha 4 callback/storage, 96 MiB heap, input and polling integration to locally
generated code. Review its output before compiling. Never replace a missing
integration recipe with guessed edits to arbitrary generated functions.
Generated game sources remain ignored and local.

## 6. Compile the consumer against that installed SDK

```powershell
cmake -S native-rex -B build/native -G Ninja -DCMAKE_BUILD_TYPE=Release `
  -DCMAKE_CXX_COMPILER=clang-cl "-DCMAKE_PREFIX_PATH=$root/toolchain/rexglue-sdk"
cmake --build build/native --parallel 2
```

If configure resolves another SDK installation, stop and correct the prefix or
use a fresh build directory. Inspect `build/native/CMakeCache.txt` for compiler,
configuration and package paths. A successful link alone does not prove runtime
ABI compatibility. The SDL presentation extension and overlay virtual interface
must come from the same SDK headers and binaries.

## 7. Build the separate launcher and matching bridge

If the UI dependency is absent:

```powershell
git clone https://github.com/ExtremestoneGG/recomp-ui.git third_party/full-emerald-ui
git -C third_party/full-emerald-ui checkout 047bc82d8679e96d1dec19ce2cd8ab677003465f
```

Then run:

```powershell
./tools/build_launcher.ps1
```

The helper builds the frontend into `build/launcher`, invokes
`python tools/build_release_bridge.py --hero-time`, and copies the matching
bridge from `build/hero-time-release`. Do not manually compile only four C# files:
the current bridge also needs All Aliens, save-reset sources, defines and authored
embedded programs. The UI's OpenGL-enabled SDL is separate from the game's SDL;
do not replace the runtime SDL configuration to fix a launcher issue.

## 8. Stage a coherent development app

Create a new disposable app directory. Copy the frontend and its bridge from
`build/launcher` into the app root. Put the **consumer build's** `ben10.exe` and
the matching SDK `rexruntime.dll`, `rexgpu-xenos.dll` and required runtime DLLs
in its `runtime` subdirectory. The SDK target helper stages runtime dependencies
beside the consumer output; verify those paths and timestamps rather than
searching the disk for similarly named DLLs.

The required native menu marker is `runtime/pc-menu-api.txt` containing `7`.
Create it only for the reviewed API 7 build; changing a marker cannot add an
export or fix an ABI mismatch. The launcher also requires `runtime/SHA256SUMS.txt`:
each runtime file uses lowercase SHA-256, two spaces, then its basename. Cover
the marker and dependencies; omit the manifest itself. Hash only your coherent
staged build, not downloaded DLLs used to make startup errors disappear.

Use the importer for a new disposable installation. For advanced local testing,
`installation-path.txt` in the app root selects an existing disposable data root;
it must have the import metadata and `game/default.xex`, not just loose assets.
Back up saves before using an older or modified build. Never publish that file
with a personal path, game assets, saves or caches.

## 9. Validate in order

1. Check stage hashes and required DLLs before launch.
2. Reach the title screen, close normally, inspect all three logs.
3. Test Continue and save persistence using a disposable copy.
4. Test the gameplay feature actually modified. A menu-only startup does not
   validate transformations, boss transitions, fullscreen, or rendering quality.
5. For performance, compare the same scene and camera with the same resolution,
   cap and display mode. Record game-frame FPS and frame times, not merely host
   presentation rate. Keep unsupported claims out of release notes.

Logs are in the selected data root's `logs` directory, normally
`game-data/logs`; older installs may use `installation/logs`. Preserve
`launcher.log`, `startup-output.log`, and `runtime.log` before launching again.
If the bridge fails first, the runtime log may be stale. Read timestamps.

## 10. Agent stop rules: prevent the rebuild loop

- Stop at the first failed stage. Record the exact command, first error, exit
  code, SDK revision and consumer commit. Diagnose that failure before retrying.
- Do not regenerate all game sources to fix a renderer-only compilation error.
- Do not update the pinned SDK, remove integration hooks, fake exports, change
  the API marker, or copy release DLLs into a new build to conceal a failure.
- If startup dies before the first draw, verify loader dependencies, exports,
  architecture and header/binary identity before debugging a draw-path change.
- Rebuild all consumers of a changed class layout or virtual interface. A
  renderer-only relink is insufficient for an ABI change.
- Distinguish player installation from contributor compilation. They are
  different workflows with different tools and outputs.
- Never describe a build as fixed until the original failure is retested. If
  another machine is required, label the artifact experimental and state exactly
  which local checks passed and which remote checks remain.

For a useful failure report, send the first failing command and its text output,
not an AI summary saying it is rebuilding. See `GPU_DIAGNOSTICS.md` for the
separate RTX 4060 investigation. No original game files should be attached.
