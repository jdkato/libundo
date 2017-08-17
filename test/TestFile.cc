#include <catch.hpp>
#include "libundo.h"

SCENARIO("Buffer contents can be serialized", "[file]") {
  GIVEN("A buffer with contents 'A'") {
    UndoTree* t = newUndoTree();
    t->insert("A");

    REQUIRE(t->size() == 1);
    REQUIRE(!t->current_node()->parent);

    WHEN("I save the UndoTree") {
      saveUndoTree(t, "test.libundo-session");

      THEN("I should be able to load it back") {
        UndoTree* t = loadUndoTree("test.libundo-session");
        REQUIRE(t->size() == 1);
      }
    }
  }
}
