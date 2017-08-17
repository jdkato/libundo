#include <catch.hpp>
#include "libundo.h"

SCENARIO("Buffer contents can be serialized", "[file]") {
  GIVEN("A buffer with contents 'A'") {
    UndoTree* t = new_tree();
    t->insert("A");

    REQUIRE(t->size() == 1);
    REQUIRE(!t->current_node()->parent);

    WHEN("I save the UndoTree") {
      save_tree(t, "test.libundo-session");

      THEN("I should be able to load it back") {
        UndoTree* t = load_tree("test.libundo-session");
        REQUIRE(t->size() == 1);
      }
    }
  }
}
