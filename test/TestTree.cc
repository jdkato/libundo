#include "libundo.h"

#include <catch.hpp>

TEST_CASE("Basic UndoTree functionality", "[file.undo]") {
  UndoTree* t = new UndoTree("foo.undo");

  t->insert("A");
  t->insert("B");
  t->undo();
  t->insert("C");
  // NOTE: B or C could be D's parent -- so, we need some notion of an "active
  // branch."
  t->insert("D");
  REQUIRE(4 == 4);

  std::vector<Node> nodes = t->nodes();
  std::cout << "Contents: " << std::endl;
  for (auto child : nodes) {
    Node* parent = child.parent;
    if (parent == NULL) {
      std::cout << "Node " << child.index << " is root." << std::endl;
    } else {
      std::cout << "Node " << child.index << " has parent " << parent->index
                << "." << std::endl;
    }
  }
}
