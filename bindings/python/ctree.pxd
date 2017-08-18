cdef extern from "libundo.h":

    ctypedef struct UndoTree:
        pass

    UndoTree* new_tree()
    UndoTree* load_tree(const char* path, const char* buf)

    void save_tree(UndoTree* t, const char* path)
    void free_tree(UndoTree* t)
    void insert(UndoTree* t, const char* buf)
    void switch_branch(UndoTree* t)

    const char* undo(UndoTree* t)
    const char* redo(UndoTree* t)
    const char* buffer(UndoTree* t)

    int size(UndoTree* t)
    int branch(UndoTree* t)
