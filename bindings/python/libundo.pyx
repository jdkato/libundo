cimport ctree

cdef class UndoTree:
    """A queue class for C integer values.

    >>> q = Queue()
    >>> q.append(5)
    >>> q.peek()
    5
    >>> q.pop()
    5
    """
    cdef ctree.UndoTree* _c_tree
    def __cinit__(self):
        self._c_tree = ctree.new_tree()
        if self._c_tree is NULL:
            raise MemoryError()

    '''TODO: is there anything we need to dealloc?
    def __dealloc__(self):
        if self._c_queue is not NULL:
            cqueue.queue_free(self._c_queue)
    '''

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
