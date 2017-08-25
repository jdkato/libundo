#include <catch.hpp>
#include "libundo.h"

SCENARIO("Traversal", "[tree]") {
  GIVEN("An empty UndoTree") {
    std::unique_ptr<libundo::UndoTree> t =
        std::make_unique<libundo::UndoTree>();

    WHEN("I move up and down (linearly)") {
      /* Initial state -- one addition ('1'):
       *
       *             1 (@)
       */
      t->insert("My name is Joe.");
      REQUIRE(t->buffer() == "My name is Joe.");
      REQUIRE(t->current_node()->id == 1);

      /* Second state --  another addition ('2'):
       *
       *             1
       *              \
       *               2 (@)
       */
      t->insert("My name is actually Bob.");
      REQUIRE(t->buffer() == "My name is actually Bob.");
      REQUIRE(t->current_node()->id == 2);

      /* Third state -- back to 'A':
       *
       *             1 (@)
       *              \
       *               2
       */
      REQUIRE(t->undo() == "My name is Joe.");
      REQUIRE(t->current_node()->id == 1);

      /* Fourth state -- back to 'B':
       *
       *             1
       *              \
       *               2 (@)
       */
      REQUIRE(t->redo() == "My name is actually Bob.");
      REQUIRE(t->current_node()->id == 2);
    }

    WHEN("I move up and down (branching)") {
      /* Initial state -- one addition ('1'):
       *
       *             1 (@)
       */
      t->insert("My name is Joe.");
      REQUIRE(t->buffer() == "My name is Joe.");
      REQUIRE(t->current_node()->id == 1);

      /* Second state --  two more additions ('2' & '3'):
       *
       *             1
       *            / \
       *       (@) 3   2
       */
      t->insert("My name is actually Bob.");
      REQUIRE(t->buffer() == "My name is actually Bob.");
      REQUIRE(t->current_node()->id == 2);
      REQUIRE(t->current_node()->parent->id == 1);

      REQUIRE(t->undo() == "My name is Joe.");

      t->insert("My name is Bob.");
      REQUIRE(t->buffer() == "My name is Bob.");
      REQUIRE(t->current_node()->id == 3);
      REQUIRE(t->current_node()->parent->id == 1);

      /* Third state --  back to '2':
       *
       *             1
       *            / \
       *           3   2 (@)
       */
      REQUIRE(t->undo() == "My name is Joe.");
      REQUIRE(t->current_node()->id == 1);

      REQUIRE(t->redo() == "My name is actually Bob.");
      REQUIRE(t->current_node()->id == 2);

      /* Fourth state --  back to '3':
       *
       *             1
       *            / \
       *       (@) 3   2
       */
      REQUIRE(t->undo() == "My name is Joe.");
      REQUIRE(t->current_node()->id == 1);

      t->switch_branch(1);

      REQUIRE(t->redo() == "My name is Bob.");
      REQUIRE(t->current_node()->id == 3);
      REQUIRE(t->current_node()->parent->id == 1);
    }
  }
}
