# Alpha animation hotfix investigation

This work addresses the October 24 report of a crash near the first lever and
incorrect Rath animation. No FPS improvement or full-campaign fix is claimed.

## Reproduced missing animation

The user's public Alpha executable matches the retained release executable
(`ae3fb4c34462c87712061d4dcfaf4542c4d44fe411f1ba8cf636c136f508101e`).
The original report ends in a guest read at `0x00000006`, without the requesting
animation key. A private, muted replay of Big Chill moving in the Paris corridor
reproduced this failure through the same null animation-resource path and logged
key `E8E2CE3D`, resource type 7. That replay alone did not prove the exact lever interaction was reached or
that every lever failure has the same cause; the manual check below is separate.

That key is `idle_01` in the level's existing `g_boid_m.xbx` archive: a bird/NPC
animation, rather than a missing Big Chill animation. The previous local archive
conversion covered the eleven ordinary player forms but omitted level residents.
The candidate also includes the current level's `g_*.xbx` animation records,
keeping original streamed records and their payloads authoritative. It uses only
the user's own local files. No game data is distributed.

The same movement/input replay completed after this change without the previous
null-resource failure. This is a bounded corridor regression check, not an
end-to-end lever or campaign test.

## Incorrect converted animation timing

Resident clips have zero placeholders at bytes 64 and 68. The game's original
resident initializer (`820E0E90`) resolves their channel indices and calls
`8219ED50` to calculate start/end times. The streamed initializer (`82231288`)
only resolves inline offsets and assumes timing is already present. Copying the
resident header without that calculation therefore produces incorrect timing.

The converter now follows `8219ED50` for the three channels of each animated
bone. Ordinary channel timestamps are big-endian floats; compressed type 16
timestamps are bytes multiplied by the game's constant 160. Bounds, indices,
strides and finite timestamps are checked. Existing streamed clips are untouched.

An isolated native test executes the unchanged generated `8219ED50` body on
2,476 local clip fixtures and compares both timing fields bit for bit. All cases
passed. This is equivalence with the generated original initializer, not an
independent Xbox hardware trace. C# converter output also matches the independent
Python archive implementation for all eight level banks.

## Updates and diagnostics

An installation can migrate only an original bank or the exact output of the
previous player-only converter. Its original backup remains unchanged. Unknown
third-party bank edits are rejected. The retained Alpha release assets remain
immutable while the candidate is being verified.

The normal Rath progression and original `primus` cheat are preserved. The cheat
sets the unlock flag; it does not replace resource loading or initialize a
converted clip's timing. This fix does not force that flag in users' saves.

The menu-only local build helper also lost retained heap/input objects when
starting from an older response file. This was reproduced separately and the
helper now preserves both objects. Binary inspection confirms the public Alpha
already contained those adjustments; that local regression must not be used as
the explanation for the public lever report.

Production fault observation now records the native location, guest registers
and stack for invalid guest pointers. It continues normal exception dispatch,
does not suppress crashes and excludes protected physical-page notifications.

## Validation and remaining scope

The user manually tested the first lever in phase 1 with this candidate and
reported no crash. Eleven forms were visually reviewed in a private corridor
test. Rath movement/jump/attack inputs and the corridor replay exited normally.
The 2,476 timing cases match the original generated initializer bit for bit.
This does not establish full campaign or extended combat coverage.

Above-30-FPS flicker and stable 60 FPS remain open. No new FPS gain is claimed.
Rath upgrades while All Aliens is disabled are a separately reported issue and
are not fixed by this release.
