import os
import unittest

from libundo import PyUndoTree


def new_tree(name):
    if os.path.exists(name):
        os.remove(name)
    return PyUndoTree(name, '')


class PyUndoTreeTestCase(unittest.TestCase):
    """Tests for navigation and serialization of PyUndoTree.
    """
    def test_navigate_linear(self):
        t = new_tree('test.libundo-session')
        # Initial state -- one addition ('1'):
        #
        #             1 (@)
        t.insert('My name is Joe.')
        self.assertEqual(t.buffer(), 'My name is Joe.')
        self.assertEqual(t.head().get('id'), 1)

        # Second state --  another addition ('2'):
        #
        #             1
        #              \
        #               2 (@)
        t.insert('My name is actually Bob.')
        self.assertEqual(t.buffer(), 'My name is actually Bob.')
        self.assertEqual(t.head().get('id'), 2)

        # Third state -- back to 'A':
        #
        #             1 (@)
        #              \
        #               2
        self.assertEqual(t.undo(), 'My name is Joe.')
        self.assertEqual(t.head().get('id'), 1)

        # Fourth state -- back to 'B':
        #
        #             1
        #              \
        #               2 (@)
        self.assertEqual(t.redo(), 'My name is actually Bob.')
        self.assertEqual(t.head().get('id'), 2)

    def test_navigate_branch(self):
        t = new_tree('test.libundo-session')
        # Initial state -- one addition ('1'):
        #            1 (@)
        t.insert('My name is Joe.')
        self.assertEqual(t.buffer(), 'My name is Joe.')
        self.assertEqual(t.head().get('id'), 1)

        # Second state --  two more additions ('2' & '3'):
        #
        #            1
        #           / \
        #      (@) 3   2
        t.insert('My name is actually Bob.')
        self.assertEqual(t.buffer(), 'My name is actually Bob.')
        self.assertEqual(t.head().get('id'), 2)
        self.assertEqual(t.head().get('parent'), 1)

        self.assertEqual(t.undo(), 'My name is Joe.')

        t.insert('My name is Bob.')
        self.assertEqual(t.buffer(), 'My name is Bob.')
        self.assertEqual(t.head().get('id'), 3)
        self.assertEqual(t.head().get('parent'), 1)

        # Third state --  back to '2':
        #
        #             1
        #            / \
        #           3   2 (@)
        self.assertEqual(t.undo(), 'My name is Joe.')
        self.assertEqual(t.head().get('id'), 1)

        self.assertEqual(t.redo(), 'My name is actually Bob.')
        self.assertEqual(t.head().get('id'), 2)

        # Fourth state --  back to '3':
        #
        #            1
        #           / \
        #      (@) 3   2
        self.assertEqual(t.undo(), 'My name is Joe.')

        t.switch_branch()

        self.assertEqual(t.redo(), 'My name is Bob.')

    def test_serialize_valid(self):
        t = new_tree('persist.libundo-session')

        t.insert('Hello from libundo (C++)!')
        self.assertEqual(len(t), 1)
        t.save()

        t2 = PyUndoTree('persist.libundo-session', 'Hello from libundo (C++)!')
        self.assertEqual(len(t2), 1)

    def test_serialize_invalid(self):
        t = new_tree('persist.libundo-session')

        t.insert('Hello from libundo (C++)!')
        self.assertEqual(len(t), 1)
        t.save()

        t2 = PyUndoTree('persist.libundo-session', 'Hello from libundo!')
        self.assertEqual(len(t2), 0)


if __name__ == '__main__':
    unittest.main()
