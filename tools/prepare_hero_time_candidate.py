"""Author private Hero Time VM functions, without copying game instructions.

Private transactions call original transitions only after a resource-ready
gate. They are not installed until ownership and gameplay guards are wired.
"""
from prepare_all_aliens_candidate import Code, crc, encode, records, relocate, hook
from hero_time_rules import ULTIMATE_PAIRS, ACTIVE_SECONDS, RECHARGE_SECONDS

TARGET = 'cosmic_hero_target'
READY = 'cosmic_hero_resources_ready'
PHASE = 'cosmic_hero_phase'
REMAINING = 'cosmic_hero_remaining'
LOAD_ACTIVE = 'cosmic_hero_load_active'
LOAD_NEXT = 'cosmic_hero_load_next'
LOAD_PENDING = 'cosmic_hero_load_pending'
PAIR_PENDING = 'cosmic_hero_pair_pending'
FORMS = 'cosmic_hero_forms'
OWNS_GATE = 'cosmic_hero_owns_mission_gate'
COOLDOWN = 'cosmic_hero_recharge_duration'


def authored():
    c = Code()
    c.nil(0); c.put(0, TARGET)
    c.native('cosmic_hero_enabled', 0)
    c.num(1, 1); c.binary(0x10, 2, 0, 1); c.false(2, 'end')
    c.get(0, READY); c.false(0, 'end')
    c.get(0, PHASE); c.num(1, 0)
    c.binary(0x10, 2, 0, 1); c.false(2, 'end')
    # Preserve both original transformation and loading guards.
    for flag in (0x597a958a, 0xc16ef1c4):
        c.get(0, flag); c.false(0, 'clear_' + str(flag)); c.jump('end')
        c.label('clear_' + str(flag))
    c.get(0, 'gCurrentCharacter')
    for normal, ultimate in ULTIMATE_PAIRS.items():
        c.num(1, normal); c.binary(0x10, 2, 0, 1)
        c.false(2, 'next_' + str(normal))
        c.num(3, ultimate); c.put(3, TARGET); c.jump('end')
        c.label('next_' + str(normal))
    c.label('end'); c.done()
    result = [(crc('CosmicHeroResolveTarget'), 0, c.finish())]

    # Called only from gameplay after original pause/loading/cutscene guards.
    # Native delta is guest simulation time, never host wall-clock time.
    c = Code()
    c.get(0, PHASE); c.num(1, 1); c.binary(0x10, 2, 0, 1)
    c.false(2, 'returning')
    c.get(3, 'gCurrentCharacter'); c.get(4, TARGET)
    c.binary(0x10, 2, 3, 4); c.false(2, 'end')
    for flag in (0x597a958a, 0xc16ef1c4):
        c.get(3, flag); c.false(3, 'activation_clear_' + str(flag)); c.jump('end')
        c.label('activation_clear_' + str(flag))
    c.num(0, 2); c.put(0, PHASE)
    c.num(0, ACTIVE_SECONDS); c.put(0, REMAINING)
    c.native('cosmic_hero_active'); c.jump('end')

    c.label('returning'); c.num(1, 3); c.binary(0x10, 2, 0, 1)
    c.false(2, 'timed')
    c.get(3, 'gCurrentCharacter'); c.num(4, 17)
    c.binary(0x10, 2, 3, 4); c.false(2, 'end')
    for flag in (0x597a958a, 0xc16ef1c4):
        c.get(3, flag); c.false(3, 'return_clear_' + str(flag)); c.jump('end')
        c.label('return_clear_' + str(flag))
    c.num(0, 4); c.put(0, PHASE)
    c.get(0, COOLDOWN); c.put(0, REMAINING)
    c.native('cosmic_hero_recharge'); c.jump('end')

    c.label('timed'); c.num(1, 2); c.binary(0x10, 2, 0, 1)
    c.false(2, 'check_cooldown')
    c.get(3, 'gCurrentCharacter'); c.get(4, TARGET)
    c.binary(0x10, 2, 3, 4); c.false(2, 'cancel'); c.jump('advance')
    c.label('check_cooldown'); c.num(1, 4); c.binary(0x10, 2, 0, 1)
    c.false(2, 'end')
    c.get(3, 'gCurrentCharacter'); c.num(4, 17)
    c.binary(0x10, 2, 3, 4); c.false(2, 'cancel')
    c.label('advance'); c.native('cosmic_hero_delta', 5)
    c.num(6, 0); c.binary(0x13, 2, 5, 6); c.false(2, 'end')
    c.get(3, REMAINING); c.binary(0x0b, 3, 3, 5)
    c.binary(0x13, 2, 3, 6); c.false(2, 'expired')
    c.put(3, REMAINING); c.jump('end')
    c.label('expired'); c.put(6, REMAINING)
    c.num(1, 2); c.binary(0x10, 2, 0, 1); c.false(2, 'ready')
    c.call('CosmicHeroReturn'); c.jump('end')
    c.label('ready'); c.num(1, 0); c.put(1, PHASE)
    c.nil(1); c.put(1, TARGET)
    c.native('cosmic_hero_ready')
    c.jump('end')
    c.label('cancel'); c.num(1, 0); c.put(1, PHASE)
    c.put(1, REMAINING); c.nil(1); c.put(1, TARGET)
    c.label('end'); c.done()
    result.append((crc('CosmicHeroAdvance'), 0, c.finish()))

    c = Code()
    c.get(0, PHASE); c.num(1, 0); c.binary(0x10, 2, 0, 1)
    c.false(2, 'end')
    c.call('CosmicHeroResolveTarget')
    c.get(0, TARGET); c.false(0, 'end')
    c.get(1, 'gCurrentCharacter'); c.put(1, 0xf495406a)
    c.num(1, 1); c.put(1, PHASE)
    c.call('switch_to_ultimate')
    c.call('CosmicHeroFlash')
    c.label('end'); c.done()
    result.append((crc('CosmicHeroRequest'), 0, c.finish()))

    # Same original ROM effect used by both ordinary transformation directions
    # (577E126C and F02A2526). Only the authored request invokes it here.
    c = Code()
    c.get(0, 'player'); c.false(0, 'end')
    c.method(0, 'position', [0], 1)
    c.field(2, 1, 0xfbdb2615); c.num(3, 0.5)
    c.binary(0x0a, 2, 2, 3); c.fieldput(1, 0xfbdb2615, 2)
    c.get(0, 0xaf1306de); c.false(0, 'end')
    c.emit(2 << 8 | 5, 0xb90ffdc1); c.boolean(3, False)
    c.method(0, 'createfromrominstance', [0, 2, 1, 3], 4)
    c.false(4, 'end'); c.method(4, 'release', [4])
    c.label('end'); c.done()
    result.append((crc('CosmicHeroFlash'), 0, c.finish()))

    c = Code()
    c.get(0, PHASE); c.num(1, 2); c.binary(0x10, 2, 0, 1)
    c.false(2, 'end')
    c.get(0, 'gCurrentCharacter'); c.get(1, TARGET)
    c.binary(0x10, 2, 0, 1); c.false(2, 'end')
    c.num(0, ACTIVE_SECONDS); c.get(1, REMAINING)
    c.binary(0x0b, 3, 0, 1); c.num(4, 0)
    c.binary(0x13, 2, 3, 4); c.false(2, 'zero'); c.jump('upper')
    c.label('zero'); c.num(3, 0)
    c.label('upper'); c.binary(0x13, 2, 3, 0); c.false(2, 'scale')
    c.num(3, ACTIVE_SECONDS)
    c.label('scale'); c.num(4, RECHARGE_SECONDS / ACTIVE_SECONDS)
    c.binary(0x0c, 3, 3, 4)
    c.num(5, RECHARGE_SECONDS); c.binary(0x13, 2, 3, 5)
    c.false(2, 'duration'); c.num(3, RECHARGE_SECONDS)
    c.label('duration'); c.put(3, COOLDOWN)
    # Own the pending transition before invoking the original asynchronous path.
    c.num(1, 3); c.put(1, PHASE)
    c.native('cosmic_hero_return')
    c.get(7, 'switch_character_cb'); c.call(0xf02a2526, [7])
    c.label('end'); c.done()
    result.append((crc('CosmicHeroReturn'), 0, c.finish()))

    # Mission-preload transaction follows the original loader's completion
    # accounting, with a single character/descriptor pair in flight.
    c = Code()
    c.native('cosmic_hero_enabled', 0); c.num(1, 1)
    c.binary(0x10, 2, 0, 1); c.false(2, 'end')
    c.get(0, LOAD_ACTIVE); c.false(0, 'start'); c.jump('end')
    c.label('start'); c.get(0, 0x1729d192); c.false(0, 'no_gate')
    c.num(1, 0); c.binary(0x13, 2, 0, 1); c.false(2, 'no_gate')
    c.num(1, 10); c.binary(0x0a, 0, 0, 1); c.put(0, 0x1729d192)
    c.boolean(1, True); c.put(1, OWNS_GATE); c.jump('setup')
    c.label('no_gate'); c.boolean(1, False); c.put(1, OWNS_GATE)
    c.label('setup'); c.num(1, 10); c.put(1, LOAD_PENDING)
    c.boolean(1, True); c.put(1, LOAD_ACTIVE)
    c.boolean(1, False); c.put(1, READY)
    c.num(1, 0); c.put(1, PHASE)
    c.table(0); c.put(0, FORMS)
    for index, ultimate in enumerate(ULTIMATE_PAIRS.values(), 1):
        c.num(1, index); c.num(2, ultimate); c.store(0, 1, 2)
    c.num(1, 1); c.put(1, LOAD_NEXT); c.call('CosmicHeroLoadNext')
    c.label('end'); c.done()
    result.append((crc('CosmicHeroLoad'), 0, c.finish()))

    c = Code()
    c.get(0, LOAD_NEXT); c.num(1, 6)
    c.binary(0x12, 2, 0, 1); c.false(2, 'end')
    c.num(1, 1); c.binary(0x0a, 1, 0, 1); c.put(1, LOAD_NEXT)
    c.num(1, 2); c.put(1, PAIR_PENDING)
    c.get(3, FORMS); c.index(4, 3, 0)
    c.get(5, 'gFormData'); c.index(6, 5, 4)
    c.get(7, 0xf3d407b1); c.method(7, 'getcurrentmission', [7], 8)
    c.get(9, 'CosmicHeroLoaded')
    c.get(10, 0x0bfbfd2d); c.field(11, 6, 'characterFile')
    c.method(8, 0xbc35779c, [8, 10, 11, 9])
    c.get(10, 0x79c61e79); c.field(11, 6, 0x7fb535f8)
    c.method(8, 0x1a7328ab, [8, 10, 11, 9])
    c.label('end'); c.done()
    result.append((crc('CosmicHeroLoadNext'), 0, c.finish()))

    c = Code()
    c.get(1, LOAD_ACTIVE); c.false(1, 'end')
    c.get(1, LOAD_PENDING); c.num(2, 0)
    c.binary(0x13, 3, 1, 2); c.false(3, 'end')
    c.num(2, 1); c.binary(0x0b, 1, 1, 2); c.put(1, LOAD_PENDING)
    c.get(1, OWNS_GATE); c.false(1, 'pair')
    c.call(0x27d93546, [0])
    c.label('pair')
    c.get(1, PAIR_PENDING); c.num(2, 1)
    c.binary(0x0b, 1, 1, 2); c.put(1, PAIR_PENDING)
    c.num(2, 0); c.binary(0x10, 3, 1, 2); c.false(3, 'end')
    c.get(1, LOAD_PENDING); c.binary(0x10, 3, 1, 2)
    c.false(3, 'next'); c.boolean(1, True); c.put(1, READY)
    c.native('cosmic_hero_loaded'); c.jump('end')
    c.label('next'); c.call('CosmicHeroLoadNext')
    c.label('end'); c.done()
    result.append((crc('CosmicHeroLoaded'), 1, c.finish()))

    c = Code()
    # Consume the edge even while dead; a held click must not activate after
    # respawn. Keep the original death/respawn transition entirely game-owned.
    c.native('cosmic_hero_button', 0)
    c.get(1, 'player'); c.false(1, 'end')
    c.method(1, 'person', [1], 2); c.false(2, 'end')
    c.method(2, 'isdead', [2], 3); c.false(3, 'alive')
    c.num(1, 0); c.put(1, PHASE); c.put(1, REMAINING)
    c.nil(1); c.put(1, TARGET); c.jump('end')
    c.label('alive')
    c.call('CosmicHeroLoad')
    c.call('CosmicHeroAdvance')
    c.call('CosmicHeroMeter')
    c.num(1, 1)
    c.binary(0x10, 2, 0, 1); c.false(2, 'end')
    c.get(0, PHASE); c.num(1, 2); c.binary(0x10, 2, 0, 1)
    c.false(2, 'activate'); c.call('CosmicHeroReturn'); c.jump('end')
    c.label('activate'); c.call('CosmicHeroRequest')
    c.label('end'); c.done()
    result.append((crc('CosmicHeroUpdate'), 0, c.finish()))

    c = Code()
    c.get(0, PHASE); c.num(1, 2); c.binary(0x10, 2, 0, 1)
    c.false(2, 'cooldown'); c.num(3, 1.0 / ACTIVE_SECONDS); c.jump('meter')
    c.label('cooldown'); c.num(1, 4); c.binary(0x10, 2, 0, 1)
    c.false(2, 'end'); c.num(3, -1.0 / RECHARGE_SECONDS)
    c.label('meter'); c.get(4, REMAINING); c.binary(0x0c, 4, 4, 3)
    c.num(1, 4); c.binary(0x10, 2, 0, 1); c.false(2, 'person')
    c.num(1, 1); c.binary(0x0a, 4, 4, 1)
    c.label('person'); c.get(5, 'player'); c.false(5, 'end')
    c.method(5, 'person', [5], 6); c.false(6, 'end')
    c.method(6, 'getmaxenergy', [6], 7)
    c.binary(0x0c, 7, 7, 4); c.method(6, 'setenergy', [6, 7])
    c.label('end'); c.done()
    result.append((crc('CosmicHeroMeter'), 0, c.finish()))

    # Only called during original form-system teardown. Keep owned resources
    # resident when the option is merely toggled off during a mission.
    c = Code()
    c.get(0, LOAD_ACTIVE); c.false(0, 'clear')
    c.get(1, 0xf3d407b1); c.method(1, 'getcurrentmission', [1], 2)
    c.get(3, FORMS); c.get(4, 'gFormData'); c.get(5, LOAD_NEXT)
    for index in range(1, 6):
        c.num(6, index); c.binary(0x12, 7, 6, 5)
        c.false(7, 'clear')
        c.index(7, 3, 6); c.index(8, 4, 7)
        c.get(9, 0x93aca275); c.field(10, 8, 'character')
        c.method(9, 'removecharacter', [9, 10])
        c.get(9, 0x0bfbfd2d); c.field(10, 8, 'characterFile'); c.boolean(11, True)
        c.method(2, 'unloaddata', [2, 9, 10, 11])
        c.get(9, 0x79c61e79); c.field(10, 8, 0x7fb535f8)
        c.method(2, 'unloaddesc', [2, 9, 10])
    c.label('clear')
    for name in (LOAD_ACTIVE, LOAD_NEXT, LOAD_PENDING, PAIR_PENDING, FORMS, OWNS_GATE,
                 READY, TARGET, REMAINING, COOLDOWN):
        c.nil(0); c.put(0, name)
    c.num(0, 0); c.put(0, PHASE)
    c.done()
    result.append((crc('CosmicHeroUnload'), 0, c.finish()))
    return result


def transform_globals(blob):
    """Private composition before All Aliens; not an installer migration."""
    rows = []
    found = set()
    for identity, args, code in records(blob):
        # Apply to original boundaries before the All Aliens relocation. The
        # caller must compose this before transform_globals from that module.
        if identity == crc('switch_form_update'):
            code = relocate(code, {34: hook('CosmicHeroUpdate')}); found.add('update')
        if identity == crc('destroy_form_switching'):
            code = relocate(code, {0: hook('CosmicHeroUnload')}); found.add('unload')
        if identity == 0xaac6a726:
            code = relocate(code, {87: hook('CosmicHeroLoad')}); found.add('load')
        rows.append((identity, args, code))
    if found != {'update', 'unload', 'load'}:
        raise ValueError('Original Hero Time lifecycle missing')
    existing = {identity for identity, _, _ in rows}
    if existing.intersection(identity for identity, _, _ in authored()):
        raise ValueError('Hero Time already installed')
    return encode(rows + authored())


if __name__ == '__main__':
    from pathlib import Path
    destination = Path(__file__).resolve().parents[1] / 'build/diagnostics/hero-time'
    destination.mkdir(parents=True, exist_ok=True)
    (destination / 'HeroTimeProgram.bin').write_bytes(encode(authored()))
    print('Private authored target resolver generated; not installed or gameplay validated.')
