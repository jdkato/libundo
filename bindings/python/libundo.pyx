from libcpp.vector cimport vector
from libcpp.string cimport string

from ctree cimport UndoTree


cdef class PyUndoTree:
    cdef UndoTree* _c_tree  # a pointer to the C++ instance which we're wrapping
                          #
    def __cinit__(self):
        self._c_tree = new UndoTree()
        if self._c_tree is NULL:
            raise MemoryError()

    def __dealloc__(self):
        if self._c_tree is not NULL:
            del self._c_tree

    cpdef undo(self):
        self._c_tree.undo()

    cpdef redo(self):
        self._c_tree.redo()

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
