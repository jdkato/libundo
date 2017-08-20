from libcpp.vector cimport vector
from libcpp.string cimport string

from ctree cimport (
    UndoTree,
    load,
    save
)


cdef class PyUndoTree:
    cdef UndoTree* _c_tree
    cdef string path

    def __cinit__(self, string path, string buf):
        self._c_tree = load(path, buf)
        if self._c_tree is NULL:
            raise MemoryError()
        self.path = path

    def __dealloc__(self):
        if self._c_tree is not NULL:
            del self._c_tree

    cpdef save(self):
        save(self._c_tree, self.path)

    cpdef undo(self):
        return self._c_tree.undo()

    cpdef redo(self):
        return self._c_tree.redo()

    cpdef size(self):
        return self._c_tree.size()

    cpdef branch(self):
        return self._c_tree.branch()

    cpdef insert(self, string buf):
        return self._c_tree.insert(buf)

    cpdef buffer(self):
        return self._c_tree.buffer()

    cpdef switch_branch(self):
        return self._c_tree.switch_branch()
