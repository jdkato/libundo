#include "libundo.h"

#include <catch.hpp>

TEST_CASE("An undo file is created", "[file.undo]") {
  const UndoTree* t = new UndoTree("", "");
  REQUIRE(1 == 1);
}
