#include "slidable_row/slidable_row.h"
#include "gtest/gtest.h"

TEST(slidable_row, basic) {
  value_t v;
  widget_t* w = slidable_row_create(NULL, 10, 20, 30, 40);
  
  widget_destroy(w);
}
