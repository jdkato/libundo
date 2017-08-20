#include <catch.hpp>
#include "libundo.h"

SCENARIO("Buffer contents can be serialized", "[file]") {
  GIVEN("A buffer with contents 'A'") {
    std::unique_ptr<libundo::UndoTree> t =
        std::make_unique<libundo::UndoTree>();

    WHEN("I save the UndoTree") {
      t->insert("Hello from libundo (C++)!");
      REQUIRE(t->size() == 1);
      REQUIRE(!t->current_node()->parent);

      libundo::save(t.get(), "test.libundo-session");

      THEN("I should be able to load it back") {
        libundo::UndoTree* old =
            libundo::load("test.libundo-session", "Hello from libundo (C++)!");
        REQUIRE(old->size() == 1);
        REQUIRE(old->buffer() == "Hello from libundo (C++)!");
        delete old;
      }
    }

    WHEN("I corrupt the UndoTree") {
      t->insert("Hello from libundo (C++)!");
      REQUIRE(t->size() == 1);
      REQUIRE(!t->current_node()->parent);

      libundo::save(t.get(), "test.libundo-session");

      THEN("I should not be able to load it back") {
        libundo::UndoTree* old =
            libundo::load("test.libundo-session", "Hello from libundo!");
        REQUIRE(old->size() == 0);
        delete old;
      }
    }
  }
}
