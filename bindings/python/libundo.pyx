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
    cdef ctree.UndoTree* _c_tree

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
        self.path = path

    def __dealloc__(self):
        if self._c_tree is not NULL:
            ctree.free_tree(self._c_tree)

    cpdef save(self):
        ctree.save_tree(self._c_tree, self.path)

    cpdef insert(self, const char* buf):
        """
        { item_description }
        """
        ctree.insert(self._c_tree, buf)

    '''
    cpdef int pop(self) except? -1:
        if cqueue.queue_is_empty(self._c_queue):
            raise IndexError("Queue is empty")
        return <int>cqueue.queue_pop_head(self._c_queue)
    '''

    '''
    def __bool__(self):
        return not cqueue.queue_is_empty(self._c_queue)
    '''
