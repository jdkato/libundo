import os
import shutil
import unittest

from libundo import PyUndoTree


if os.path.exists('test.libundo-session'):
    shutil.remove('test.libundo-session')


class PyUndoTreeTestCase(unittest.TestCase):
    """Tests for navigation and serialization of PyUndoTree.
    """
    def test_navigate_linear(self):
        t = PyUndoTree('test.libundo-session', '')
        # Initial state -- one addition ('1'):
        #
        #             1 (@)
        t.insert('My name is Joe.')
        self.assertEqual(t.buffer(), 'My name is Joe.')

        # Second state --  another addition ('2'):
        #
        #             1
        #              \
        #               2 (@)
        t.insert('My name is actually Bob.')
        self.assertEqual(t.buffer(), 'My name is actually Bob.')

        # Third state -- back to 'A':
        #
        #             1 (@)
        #              \
        #               2
        self.assertEqual(t.undo(), 'My name is Joe.')

        # Fourth state -- back to 'B':
        #
        #             1
        #              \
        #               2 (@)
        self.assertEqual(t.redo(), 'My name is actually Bob.')


if __name__ == '__main__':
    unittest.main()
