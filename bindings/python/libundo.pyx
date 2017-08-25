from libcpp.vector cimport vector
from libcpp.string cimport string
from libcpp.memory cimport shared_ptr
from cython.operator cimport dereference

from ctree cimport (
    UndoTree,
    load,
    save,
    Node
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

    def __len__(self):
        return self._c_tree.size()

    cpdef save(self):
        save(self._c_tree, self.path)

    cpdef undo(self):
        return self._c_tree.undo()

    cpdef redo(self):
        return self._c_tree.redo()

    cpdef branch(self):
        return self._c_tree.branch()

    cpdef insert(self, string buf):
        return self._c_tree.insert(buf)

    cpdef buffer(self):
        return self._c_tree.buffer()

    cpdef switch_branch(self, int direction):
        return self._c_tree.switch_branch(direction)

    cpdef head(self):
        cdef shared_ptr[Node] ptr = self._c_tree.current_node()
        return self.__make_node(dereference(ptr))

    cpdef nodes(self):
        cdef vector[Node] nodes = self._c_tree.nodes()
        return [self.__make_node(n) for n in nodes]

    cdef __make_node(self, Node n):
        return {
            'id': n.id,
            'timestamp': n.timestamp,
            'parent': dereference(n.parent).id if n.parent.get() else None,
            'children': [dereference(c).id for c in n.children]
        }
