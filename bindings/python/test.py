from libundo import PyUndoTree

t = PyUndoTree()

t.insert('My name is Joe.')
print("A: {0}".format(t.buffer()))

t.insert('My name is actually Bob.')
print("B: {0}".format(t.buffer()))

t.undo()
print("After undo: {0}".format(t.buffer()))

t.redo()
print("After redo: {0}".format(t.buffer()))
