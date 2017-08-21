import os
import shutil
import unittest

from libundo import PyUndoTree


def new_tree():
    if os.path.exists('test.libundo-session'):
        shutil.remove('test.libundo-session')
    return PyUndoTree('test.libundo-session', '')


class PyUndoTreeTestCase(unittest.TestCase):
    """Tests for navigation and serialization of PyUndoTree.
    """
    def test_navigate_linear(self):
        t = new_tree()
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

    def test_navigate_branch(self):
        t = new_tree()
        # Initial state -- one addition ('1'):
        #            1 (@)
        t.insert('My name is Joe.')
        self.assertEqual(t.buffer(), 'My name is Joe.')

        # Second state --  two more additions ('2' & '3'):
        #
        #            1
        #           / \
        #      (@) 3   2
        t.insert('My name is actually Bob.')
        self.assertEqual(t.buffer(), 'My name is actually Bob.')

        self.assertEqual(t.undo(), 'My name is Joe.')

        t.insert('My name is Bob.')
        self.assertEqual(t.buffer(), 'My name is Bob.')

        # Third state --  back to '2':
        #
        #             1
        #            / \
        #           3   2 (@)
        self.assertEqual(t.undo(), 'My name is Joe.')
        self.assertEqual(t.redo(), 'My name is actually Bob.')

        # Fourth state --  back to '3':
        #
        #            1
        #           / \
        #      (@) 3   2
        self.assertEqual(t.undo(), 'My name is Joe.')

        t.switch_branch()

        self.assertEqual(t.redo(), 'My name is Bob.')


if __name__ == '__main__':
    unittest.main()
