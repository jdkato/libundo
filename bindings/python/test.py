from libundo import PyUndoTree

t = PyUndoTree("foo.py", "")
t.insert("Hello from libundo (C++)!")
print(t.get_buffer())
