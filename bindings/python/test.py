import os
import unittest

from libundo import PyUndoTree


def new_tree(name):
    if os.path.exists(name):
        os.remove(name)
    return PyUndoTree(name.encode(), ''.encode())


class PyUndoTreeTestCase(unittest.TestCase):
    """Tests for navigation and serialization of PyUndoTree.
    """
    def test_navigate_linear(self):
        t = new_tree('test.libundo-session')
        # Initial state -- one addition ('1'):
        #
        #             1 (@)
        t.insert('My name is Joe.'.encode(), 0)
        self.assertEqual(t.buffer().decode(), 'My name is Joe.')
        self.assertEqual(t.head().get('id'), 1)

        # Second state --  another addition ('2'):
        #
        #             1
        #              \
        #               2 (@)
        t.insert('My name is actually Bob.'.encode(), 0)
        self.assertEqual(t.buffer().decode(), 'My name is actually Bob.')
        self.assertEqual(t.head().get('id'), 2)

        # Third state -- back to 'A':
        #
        #             1 (@)
        #              \
        #               2
        self.assertEqual(t.undo()['buffer'].decode(), 'My name is Joe.')
        self.assertEqual(t.head().get('id'), 1)

        # Fourth state -- back to 'B':
        #
        #             1
        #              \
        #               2 (@)
        self.assertEqual(t.redo()['buffer'].decode(), 'My name is actually Bob.')
        self.assertEqual(t.head().get('id'), 2)

    def test_navigate_branch(self):
        t = new_tree('test.libundo-session')
        # Initial state -- one addition ('1'):
        #            1 (@)
        t.insert('My name is Joe.'.encode(), 0)
        self.assertEqual(t.buffer().decode(), 'My name is Joe.')
        self.assertEqual(t.head().get('id'), 1)

        # Second state --  two more additions ('2' & '3'):
        #
        #            1
        #           / \
        #      (@) 3   2
        t.insert('My name is actually Bob.'.encode(), 0)
        self.assertEqual(t.buffer().decode(), 'My name is actually Bob.')
        self.assertEqual(t.head().get('id'), 2)
        self.assertEqual(t.head().get('parent'), 1)

        self.assertEqual(t.undo()['buffer'].decode(), 'My name is Joe.')

        t.insert('My name is Bob.'.encode(), 0)
        self.assertEqual(t.buffer().decode(), 'My name is Bob.')
        self.assertEqual(t.head().get('id'), 3)
        self.assertEqual(t.head().get('parent'), 1)

        # Third state --  back to '2':
        #
        #             1
        #            / \
        #           3   2 (@)
        self.assertEqual(t.undo()['buffer'].decode(), 'My name is Joe.')
        self.assertEqual(t.head().get('id'), 1)

        self.assertEqual(t.redo()['buffer'].decode(), 'My name is actually Bob.')
        self.assertEqual(t.head().get('id'), 2)

        # Fourth state --  back to '3':
        #
        #            1
        #           / \
        #      (@) 3   2
        self.assertEqual(t.undo()['buffer'].decode(), 'My name is Joe.')

        t.switch_branch(1)

        self.assertEqual(t.redo()['buffer'].decode(), 'My name is Bob.')

    def test_serialize_valid(self):
        t = new_tree('persist.libundo-session')

        t.insert('Hello from libundo (C++)!'.encode(), 0)
        self.assertEqual(len(t), 1)
        t.save()

        t2 = PyUndoTree(
            'persist.libundo-session'.encode(),
            'Hello from libundo (C++)!'.encode())
        self.assertEqual(len(t2), 1)

    def test_serialize_invalid(self):
        t = new_tree('persist.libundo-session')

        t.insert('Hello from libundo (C++)!'.encode(), 0)
        self.assertEqual(len(t), 1)
        t.save()

        t2 = PyUndoTree(
            'persist.libundo-session'.encode(),
            'Hello from libundo!'.encode())
        self.assertEqual(len(t2), 0)


if __name__ == '__main__':
    unittest.main()
