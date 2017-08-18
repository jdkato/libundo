from libcpp.vector cimport vector
from libcpp.string cimport string

from ctree cimport UndoTree


cdef class PyUndoTree:
    cdef UndoTree* c_tree  # a pointer to the C++ instance which we're wrapping
                          #
    def __cinit__(self):
        self.c_tree = new UndoTree()

    def __dealloc__(self):
        del self.c_tree

    cpdef insert(self, string buf):
        return self.c_tree.insert(buf)

    cpdef get_buffer(self):
        return self.c_tree.buffer()
