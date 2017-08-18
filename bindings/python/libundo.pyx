cimport ctree

cdef class UndoTree:
    """A queue class for C integer values.

    >>> buf = self.view.substr(sublime.Region(0, view.size()))
    >>> tree = UndoTree(self.view.file_name(), buf)
    >>> q.append(5)
    >>> q.peek()
    5
    >>> q.pop()
    5
    """
    cpdef ctree.UndoTree* _c_tree

    def __cinit__(self, const char* path, const char* buf):
        # Start a new tree if
        #
        # 1. If `path` exists, but `buf` != `ctree.buffer()`; or
        #
        # 2. if `path` doesn't exist.
        #
        # Otherwise, load the tree from disk.
        self._c_tree = ctree.load_tree(path, buf)
        if self._c_tree is NULL:
            raise MemoryError()

    def __dealloc__(self):
        if self._c_tree is not NULL:
            ctree.free_tree(self._c_tree)

    cpdef save(self, const char* path):
        ctree.save_tree(self._c_tree, path)

    cpdef insert(self, const char* buf):
        """
        { item_description }
        """
        ctree.insert(self._c_tree, buf)

    cpdef undo(self):
        """
        """
        return ctree.undo(self._c_tree)

    cpdef redo(self):
        """
        """
        return ctree.redo(self._c_tree)

    cpdef switch_branch(self):
        """
        """
        return ctree.switch_branch(self._c_tree)

    cpdef size(self):
        return ctree.size(self._c_tree)

    cpdef branch(self):
        return ctree.branch(self._c_tree)

    cpdef buf(self):
        return ctree.buffer(self._c_tree)

    def __bool__(self):
        return self.size() > 0
