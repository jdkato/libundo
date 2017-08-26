from libcpp.vector cimport vector
from libcpp.string cimport string
from libcpp.memory cimport shared_ptr
from libcpp.map cimport map

cdef extern from "libundo.h" namespace "libundo":

    cdef struct Delta:
        string buffer
        string patch
        int position

    cdef cppclass Node:
        int id
        shared_ptr[Node] parent
        vector[shared_ptr[Node]] children
        map[int, string] patches
        string timestamp

    cdef cppclass UndoTree:
        UndoTree() except +
        void insert(const string& buf, int pos)
        Delta undo()
        Delta redo()
        string buffer()
        shared_ptr[Node] current_node()
        vector[Node] nodes()
        int size()
        int branch()
        void switch_branch(int direction)

    cdef UndoTree* load(const string& path, const string& buf)
    cdef void save(UndoTree* t, const string& path)


