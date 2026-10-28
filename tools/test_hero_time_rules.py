"""Reference-rule checks only; these do not validate in-game transformations."""
import unittest
from hero_time_rules import Cycle, ULTIMATE_PAIRS, eligible_target


class Rules(unittest.TestCase):
    def test_proportional_early_return(self):
        for used in (0, 1, 10, 30, 45, 60):
            c = Cycle(phase='active', target=14, remaining=60-used)
            self.assertEqual(c.return_early(), 17)
            self.assertAlmostEqual(c.recharge_duration, used/3)
            self.assertIsNone(c.return_early())
            c.update(17, 1, gameplay_running=True)
            self.assertAlmostEqual(c.remaining, used/3)

    def test_only_corresponding_normal_forms(self):
        for form in range(0, 32):
            self.assertEqual(eligible_target(form, enabled=True,
                             resources_ready=True, busy=False), ULTIMATE_PAIRS.get(form))
        self.assertEqual(ULTIMATE_PAIRS[3], 14)  # Echo Echo -> Ultimate Echo Echo

    def test_guards(self):
        for enabled, ready, busy in [(False, True, False), (True, False, False),
                                     (True, True, True)]:
            self.assertIsNone(eligible_target(3, enabled=enabled,
                              resources_ready=ready, busy=busy))

    def test_confirmation_pause_expiry_and_recharge(self):
        c = Cycle()
        self.assertIsNone(c.request(17, enabled=True, resources_ready=True, busy=False))
        self.assertEqual(c.request(3, enabled=True, resources_ready=True, busy=False), 14)
        self.assertIsNone(c.request(3, enabled=True, resources_ready=True, busy=False))
        c.update(3, 100, gameplay_running=True)
        self.assertEqual(c.phase, 'activating')
        c.update(14, 1, gameplay_running=True, busy=True)
        self.assertEqual(c.phase, 'activating')
        c.update(14, 1, gameplay_running=True)
        self.assertEqual(c.remaining, 60)
        c.update(14, 120, gameplay_running=False)
        self.assertEqual(c.remaining, 60)
        self.assertIsNone(c.update(14, 59, gameplay_running=True))
        self.assertEqual(c.update(14, 1, gameplay_running=True), 17)
        self.assertTrue(c.transformations_locked)
        c.update(14, 100, gameplay_running=True)
        self.assertEqual(c.phase, 'returning')
        c.update(17, 100, gameplay_running=True)
        self.assertEqual(c.remaining, 20)
        c.update(17, 19, gameplay_running=True)
        self.assertTrue(c.transformations_locked)
        c.update(17, 1, gameplay_running=True)
        self.assertEqual(c.phase, 'ready')
        self.assertFalse(c.transformations_locked)


if __name__ == '__main__':
    unittest.main()
