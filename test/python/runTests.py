"""
Unit tests for the Cantera Python module.

This script gathers all the tests defined in all of the test<foo>.py
files, runs them, and prints a report.
"""
import sys
import unittest

import Cantera

if __name__ == '__main__':
    print '\n* INFO: using Cantera module found at this location:'
    print '*     ', repr(Cantera.__file__), '\n'
    sys.stdout.flush()

    loader = unittest.TestLoader()
    runner = unittest.TextTestRunner(verbosity=2)
    suite = loader.loadTestsFromName('testSolution')
    runner.run(suite)
