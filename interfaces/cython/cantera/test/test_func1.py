import unittest
import numpy as np

import cantera as ct
from . import utilities

class TestFunc1(utilities.CanteraTest):
    def test_function(self):
        f = ct.Func1(np.sin)
        self.assertNear(f(0), np.sin(0))
        self.assertNear(f(0.1), np.sin(0.1))
        self.assertNear(f(0.7), np.sin(0.7))

    def test_lambda(self):
        f = ct.Func1(lambda t: np.sin(t)*np.sqrt(t))
        for t in [0.1, 0.7, 4.5]:
            self.assertNear(f(t), np.sin(t)*np.sqrt(t))

    def test_callable(self):
        class Multiplier(object):
            def __init__(self, factor):
                self.factor = factor
            def __call__(self, t):
                return self.factor * t

        m = Multiplier(8.1)
        f = ct.Func1(m)
        for t in [0.1, 0.7, 4.5]:
            self.assertNear(f(t), 8.1*t)

    def test_constant(self):
        f = ct.Func1(5)
        for t in [0.1, 0.7, 4.5]:
            self.assertNear(f(t), 5)

    def test_failure(self):
        def fails(t):
            raise ValueError('bad')

        f = ct.Func1(fails)
        self.assertRaises(ValueError, f, 0.1)
