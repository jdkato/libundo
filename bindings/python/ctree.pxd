cdef extern from "libundo.h":

    ctypedef struct UndoTree:
        pass

    UndoTree* new_tree()
    UndoTree* load_tree(const char* path)
    void save_tree(UndoTree* t, const char* path)
    void insert(UndoTree* t, const char* buf)
