"""Authored Hero Time rules. No original game code or assets.

IDs are verified against the local game's form table; this module never
changes a character or loads resources by itself.
"""
from dataclasses import dataclass

ULTIMATE_PAIRS = {1: 15, 2: 13, 3: 14, 4: 12, 5: 16}
BEN = 17
ACTIVE_SECONDS = 60.0
RECHARGE_SECONDS = 30.0


def eligible_target(current_form, *, enabled, resources_ready, busy):
    """Require an already active normal form, never Ben -> Ultimate."""
    if not enabled or not resources_ready or busy:
        return None
    return ULTIMATE_PAIRS.get(current_form)


@dataclass
class Cycle:
    """Reference lifecycle for testing the authored runtime implementation.

    Requests are commands for original game callbacks, not proof that those
    callbacks succeeded. Timers start only after observed completion.
    """
    phase: str = 'ready'
    target: int | None = None
    remaining: float = 0.0

    def request(self, current_form, *, enabled, resources_ready, busy):
        if self.phase != 'ready':
            return None
        target = eligible_target(current_form, enabled=enabled,
                                 resources_ready=resources_ready, busy=busy)
        if target is not None:
            self.target = target
            self.phase = 'activating'
        return target

    def update(self, current_form, dt, *, gameplay_running, busy=False):
        if dt < 0:
            raise ValueError('Negative game delta')
        if not gameplay_running:
            return None
        if self.phase == 'activating':
            if current_form == self.target and not busy:
                self.phase = 'active'
                self.remaining = ACTIVE_SECONDS
            return None
        if self.phase == 'returning':
            if current_form == BEN and not busy:
                self.phase = 'recharging'
                self.remaining = RECHARGE_SECONDS
            return None
        if self.phase not in ('active', 'recharging'):
            return None
        self.remaining = max(0.0, self.remaining - dt)
        if self.remaining:
            return None
        if self.phase == 'active':
            self.phase = 'returning'
            return BEN
        self.phase = 'ready'
        self.target = None
        return None

    @property
    def transformations_locked(self):
        return self.phase in ('returning', 'recharging')
