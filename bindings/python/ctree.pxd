from libcpp.vector cimport vector
from libcpp.string cimport string

cdef extern from "libundo.h" namespace "libundo":

    cdef cppclass UndoTree:
        UndoTree() except +
        void insert(const string& buf)
        string undo()
        string redo()
        string buffer()
        int size()
        int branch()
        void switch_branch()


