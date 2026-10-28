"""Execute the actual authored target resolver in the existing VM model.

This does not claim that resource loading or real transformations are ready.
"""
from test_all_aliens_script import VM, scripts
from prepare_hero_time_candidate import authored, TARGET, READY, PHASE
from prepare_all_aliens_candidate import crc, decode
from hero_time_rules import ULTIMATE_PAIRS

vm = VM([1, 2, 3, 4])
for event in ('loaded', 'active', 'return', 'recharge', 'ready'):
    vm.globals[crc('overlay')][crc('cosmic_hero_' + event)] = lambda *_: None
vm.functions = {identity: (args, code) for identity, args, code in authored()}
vm.globals[crc('overlay')][crc('cosmic_hero_enabled')] = lambda *_: 1
vm.globals[crc(READY)] = True
vm.globals[crc(PHASE)] = 0
cases = 0
for form in range(32):
    vm.globals[crc('gCurrentCharacter')] = form
    # Stale target must be cleared on every rejected request.
    vm.globals[crc(TARGET)] = 123
    vm.run('CosmicHeroResolveTarget')
    assert vm.globals[crc(TARGET)] == ULTIMATE_PAIRS.get(form), form
    cases += 1
for flag, value in [(crc(READY), False), (crc(PHASE), 1),
                    (0x597a958a, True), (0xc16ef1c4, True)]:
    previous = vm.globals.get(flag)
    vm.globals[flag] = value
    vm.globals[crc('gCurrentCharacter')] = 3
    vm.run('CosmicHeroResolveTarget')
    assert vm.globals[crc(TARGET)] is None
    vm.globals[flag] = previous
    cases += 1
vm.globals[crc('overlay')][crc('cosmic_hero_enabled')] = lambda *_: 0
vm.run('CosmicHeroResolveTarget')
assert vm.globals[crc(TARGET)] is None
for _, _, code in authored():
    words, tokens = decode(code)
    boundaries = {at for at, _ in tokens} | {len(words)}
    for at, opcode in tokens:
        if opcode in (0x1d, 0x1f):
            assert words[at] >> (8 if opcode == 0x1d else 16) in boundaries
assert not vm.events, 'Resolver must not invoke transformations or resource operations'
print(f'Hero Time authored resolver: {cases + 1} cases passed; gameplay unverified.')

# Exercise the authored lifecycle itself, with original engine calls modeled
# as observations. No test directly assigns the result of a transformation.
from prepare_hero_time_candidate import REMAINING, COOLDOWN
for identity in vm.functions:
    vm.globals[identity] = identity
vm.globals[crc('overlay')][crc('cosmic_hero_enabled')] = lambda *_: 1
delta = [1.0]
vm.globals[crc('overlay')][crc('cosmic_hero_delta')] = lambda *_: delta[0]
vm.globals[crc('switch_to_ultimate')] = lambda: vm.events.append(('ultimate', vm.globals[0xf495406a]))
vm.globals[crc('switch_character_cb')] = 'original-ben-callback'
vm.globals[0xf02a2526] = lambda callback: vm.events.append(('ben', callback))
vm.globals[crc(PHASE)] = 0
vm.globals[crc('gCurrentCharacter')] = 3
vm.run('CosmicHeroRequest')
assert vm.events == [('ultimate', 3)]
assert vm.globals[crc(PHASE)] == 1
vm.run('CosmicHeroRequest')
assert vm.globals[crc(TARGET)] == 14, 'Repeated clicks must preserve pending target'
assert len(vm.events) == 1
vm.run('CosmicHeroAdvance')
assert vm.globals[crc(PHASE)] == 1, 'Request is not transformation completion'
vm.globals[crc('gCurrentCharacter')] = 14
vm.globals[0x597a958a] = True
vm.run('CosmicHeroAdvance')
assert vm.globals[crc(PHASE)] == 1
vm.globals[0x597a958a] = False
vm.run('CosmicHeroAdvance')
assert vm.globals[crc(PHASE)] == 2 and vm.globals[crc(REMAINING)] == 60
delta[0] = 0
vm.run('CosmicHeroAdvance')
assert vm.globals[crc(REMAINING)] == 60
delta[0] = 59
vm.run('CosmicHeroAdvance')
assert vm.globals[crc(REMAINING)] == 1
delta[0] = 1
vm.run('CosmicHeroAdvance')
assert vm.events[-1] == ('ben', 'original-ben-callback')
assert vm.globals[crc(PHASE)] == 3
vm.run('CosmicHeroAdvance')
assert len(vm.events) == 2 and vm.globals[crc(PHASE)] == 3
vm.globals[crc('gCurrentCharacter')] = 17
vm.run('CosmicHeroAdvance')
assert vm.globals[crc(PHASE)] == 4 and vm.globals[crc(REMAINING)] == 20
vm.run('CosmicHeroRequest')
assert len(vm.events) == 2
delta[0] = 20
vm.run('CosmicHeroAdvance')
assert vm.globals[crc(PHASE)] == 0 and vm.globals[crc(TARGET)] is None
print('Hero Time authored lifecycle: activation confirmation, 60/20 timing and single transition requests passed.')

from prepare_hero_time_candidate import LOAD_ACTIVE, LOAD_PENDING
loader = VM([1, 2, 3, 4])
for event in ('loaded', 'active', 'return', 'recharge', 'ready'):
    loader.globals[crc('overlay')][crc('cosmic_hero_' + event)] = lambda *_: None
loader.functions = {identity: (args, code) for identity, args, code in authored()}
for identity in loader.functions:
    loader.globals[identity] = identity
loader.globals[crc('overlay')][crc('cosmic_hero_enabled')] = lambda *_: 1
for form in ULTIMATE_PAIRS.values():
    loader.globals[crc('gFormData')][form] = {
        crc('characterFile'): f'ultimate_{form}', 0x7fb535f8: f'attack_{form}'}
loader.globals[0x1729d192] = 1
def original_completion(result):
    loader.globals[0x1729d192] -= 1
    loader.events.append(('original-completion', result))
loader.globals[0x27d93546] = original_completion
loader.run('CosmicHeroLoad')
assert loader.globals[0x1729d192] == 11
assert len(loader.pending) == 2, 'Only one data/descriptor pair in flight'
loader.run('CosmicHeroLoad')
assert len(loader.pending) == 2, 'No duplicate mission preload'
for pair in range(5):
    assert not loader.globals[crc(READY)]
    assert len(loader.pending) == 2
    loader.complete()
assert loader.globals[crc(READY)] is True
assert loader.globals[crc(LOAD_PENDING)] == 0
assert loader.globals[0x1729d192] == 1
assert len([event for event in loader.events if event[0] == 'load']) == 10
assert not loader.pending
before = list(loader.events)
loader.run('CosmicHeroLoaded', [True])
assert loader.events == before, 'Extra completion must not release the mission gate twice'
print('Hero Time authored preload: five sequential resource pairs and mission completion accounting passed.')
for form in ULTIMATE_PAIRS.values():
    loader.globals[crc('gFormData')][form][crc('character')] = f'character_{form}'
loader.run('CosmicHeroUnload')
loaded = sorted(event[1:] for event in loader.events if event[0] == 'load')
unloaded = sorted(event[1:] for event in loader.events if event[0] == 'unload')
assert loaded == unloaded and len(unloaded) == 10
assert len([event for event in loader.events if event[0] == 'remove']) == 5
assert loader.globals[crc(READY)] is None
assert loader.globals[crc(LOAD_ACTIVE)] is None
before = list(loader.events)
loader.run('CosmicHeroUnload')
loader.run('CosmicHeroLoaded', [True])
assert loader.events == before

from prepare_hero_time_candidate import transform_globals
from prepare_all_aliens_candidate import ROOT, records, transform_globals as aliens_transform
original = (scripts / 'globals.ds').read_bytes()
patched = transform_globals(original)
combined = aliens_transform(patched, preload_resources=True)
old = {identity: (args, code) for identity, args, code in records(original)}
new = {identity: (args, code) for identity, args, code in records(patched)}
assert {identity for identity in old if old[identity] != new[identity]} == {
    crc('switch_form_update'), crc('destroy_form_switching'), 0xaac6a726}
for _, _, code in records(combined):
    words, tokens = decode(code)
    boundaries = {at for at, _ in tokens} | {len(words)}
    for at, opcode in tokens:
        if opcode in (0x1d, 0x1e, 0x1f):
            assert words[at] >> (8 if opcode == 0x1d else 16) in boundaries
print('Hero Time teardown balance and composed All Aliens branch boundaries passed; cancellation remains unverified.')

meter_values = []
person = {crc('getmaxenergy'): lambda *_: 100.0,
          crc('setenergy'): lambda _, value: meter_values.append(value)}
vm.globals[crc('player')] = {crc('person'): lambda *_: person}
for phase, remaining, expected in [(2, 60, 100), (2, 30, 50), (2, 0, 0),
                                    (4, 20, 0), (4, 10, 50), (4, 0, 100)]:
    vm.globals[crc(PHASE)] = phase
    vm.globals[crc(REMAINING)] = remaining
    vm.run('CosmicHeroMeter')
    assert abs(meter_values[-1] - expected) < 0.00001
before = list(meter_values)
for phase in (None, 0, 1, 3):
    vm.globals[crc(PHASE)] = phase
    vm.run('CosmicHeroMeter')
assert meter_values == before, 'Do not overwrite original energy outside an owned timed phase'
print('Hero Time meter: original person API receives drain/refill values only in owned timed phases.')

for phase, form in [(2, 4), (4, 14)]:
    vm.globals[crc(PHASE)] = phase
    vm.globals[crc(TARGET)] = 14
    vm.globals[crc(REMAINING)] = 10
    vm.globals[crc('gCurrentCharacter')] = form
    before = list(vm.events)
    vm.run('CosmicHeroAdvance')
    assert vm.globals[crc(PHASE)] == 0 and vm.globals[crc(TARGET)] is None
    assert vm.events == before, 'External story form changes must not force Ben'
vm.globals[crc('overlay')][crc('cosmic_hero_button')] = lambda *_: 1
person[crc('isdead')] = lambda *_: True
vm.globals[crc(PHASE)] = 2
vm.globals[crc(TARGET)] = 14
before = list(vm.events)
vm.run('CosmicHeroUpdate')
assert vm.globals[crc(PHASE)] == 0 and vm.globals[crc(TARGET)] is None
assert vm.events == before, 'Original death/respawn remains game-owned'

loader.events = []
loader.globals[0x1729d192] = None
loader.run('CosmicHeroLoad')
for _ in range(5):
    loader.complete()
assert loader.globals[crc(READY)] is True
assert loader.globals[0x1729d192] is None
assert not any(event[0] == 'original-completion' for event in loader.events)
loader.run('CosmicHeroUnload')
assert sorted(event[1:] for event in loader.events if event[0] == 'load') == sorted(
    event[1:] for event in loader.events if event[0] == 'unload')
print('Hero Time death/story interruption and mid-mission resource accounting passed.')

for used in (0, 1, 10, 30, 45, 60):
    vm.globals[crc(PHASE)] = 2
    vm.globals[crc(TARGET)] = 14
    vm.globals[crc('gCurrentCharacter')] = 14
    vm.globals[crc(REMAINING)] = 60-used
    before = len(vm.events)
    vm.run('CosmicHeroReturn')
    assert abs(vm.globals[crc(COOLDOWN)] - used/3) < 0.000001
    assert 0 <= vm.globals[crc(COOLDOWN)] <= 20
    assert vm.globals[crc(PHASE)] == 3 and len(vm.events) == before+1
    vm.run('CosmicHeroReturn')
    assert len(vm.events) == before+1
    vm.globals[crc('gCurrentCharacter')] = 17
    vm.run('CosmicHeroAdvance')
    assert abs(vm.globals[crc(REMAINING)] - used/3) < 0.000001
    vm.run('CosmicHeroMeter')
    assert abs(meter_values[-1] - (100-used*100/60)) < 0.00001
person[crc('isdead')] = lambda *_: False
vm.globals[crc(LOAD_ACTIVE)] = True
vm.globals[crc(PHASE)] = 2
vm.globals[crc(TARGET)] = 14
vm.globals[crc('gCurrentCharacter')] = 14
vm.globals[crc(REMAINING)] = 30
delta[0] = 0
vm.run('CosmicHeroUpdate')
assert vm.globals[crc(PHASE)] == 3 and abs(vm.globals[crc(COOLDOWN)] - 10) < 0.000001
print('Hero Time proportional return: actual L3 route, one-third-used recharge, 20-second maximum and partial meter verified in VM model.')

flash_events = []
position = {crc('x'): 2, crc('y'): 3, crc('z'): 4}
vm.globals[crc('player')][crc('position')] = lambda *_: dict(position)
effect = {crc('release'): lambda *_: flash_events.append(('release',))}
def spawn(_, identity, location, flag):
    flash_events.append(('spawn', identity, dict(location), flag))
    return effect
vm.globals[0xaf1306de] = {crc('createfromrominstance'): spawn}
vm.run('CosmicHeroFlash')
assert flash_events == [('spawn', 0xb90ffdc1, {crc('x'): 2, crc('y'): 3.5, crc('z'): 4}, False), ('release',)]
assert position[crc('y')] == 3
flash_events.clear()
vm.globals[crc(PHASE)] = 0
vm.globals[crc(READY)] = True
vm.globals[crc('gCurrentCharacter')] = 17
vm.run('CosmicHeroRequest')
assert not flash_events, 'Rejected Ben request must not spawn the effect'
vm.globals[crc('gCurrentCharacter')] = 3
vm.run('CosmicHeroRequest')
vm.run('CosmicHeroRequest')
assert len(flash_events) == 2, 'One original flash and one handle release per accepted request'
print('Hero Time flash: original effect interface, offset, release and accepted-request-only behavior passed.')

import hashlib, json
from prepare_all_aliens_candidate import encode
proof = ROOT / 'build/diagnostics/hero-time/authored-program-proof.json'
proof.parent.mkdir(parents=True, exist_ok=True)
proof.write_text(json.dumps(dict(private=True, model_tests_passed=True,
    authored_sha256=hashlib.sha256(encode(authored())).hexdigest(),
    real_gameplay_validated=False,
    remaining=['Real transitions and attacks', 'Original HUD visibility',
               'Pause/loading guards in runtime', 'Pending callback cancellation',
               'In-game Extras navigation']), indent=2) + '\n')
