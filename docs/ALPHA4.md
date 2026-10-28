# Alpha 4 implementation notes

ReXGlue remains the kernel, audio and D3D12 provider. The baseline revision and
dependency origins are recorded in PROVENANCE.json and SOURCES.md. The single
SDK patch contains the retained changes; the complete source archive supplies
the matching patched SDK, authored shader source and dependency source trees.

`tools/apply_game_integration.py` runs after local codegen. Its companion recipes
expand both callback tables, initialize appended storage and update teardown,
add failure-only diagnostics, increase the file heap and insert bounded CPU
polling hints. They contain locations and authored changes, not original bodies.
The callback recipe's 12 modified bodies were compared with the compiled
candidate and a second application was checked to be a no-op.

The full callback object grows from 744 to 4620 bytes. Original scalar fields
stay in place. New record/queue regions are disjoint. Registration/removal tests
use guarded synthetic memory and reproduce the original overwrite; the manually
confirmed Vreedle test supplies separate encounter evidence. More RAM cannot
repair an overlapping fixed-size table by itself.

The GPU texture-copy optimization requires a matching supported host layout and
validity state and otherwise falls back. The CPU polling optimization retains
the original progress reads, timeout and loop checks, and inserts eight PAUSE
hints only after 64 unchanged observations. Progress or object changes reset it.
Neither change lowers image resolution or generates synthetic frames.

The private fixed-scene polling observation is reported in RELEASE_NOTES.md.
No performance measurement of the final installer is substituted for that
experiment. A release integration run checks normal-level loading and form
transitions; it cannot certify the entire game or resolve the above-30-FPS
camera issue. Full clean-machine source rebuild remains unverified.
