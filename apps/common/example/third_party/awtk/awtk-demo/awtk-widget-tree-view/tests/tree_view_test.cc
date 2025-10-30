#include "tree_view/tree_view.h"
#include "gtest/gtest.h"

TEST(tree_view, basic) {
  value_t v;
  widget_t* w = tree_view_create(NULL, 10, 20, 30, 40);

  widget_destroy(w);
}
