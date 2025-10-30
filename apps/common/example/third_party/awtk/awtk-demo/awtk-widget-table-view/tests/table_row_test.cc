#include "table_row/table_row.h"
#include "gtest/gtest.h"

TEST(table_row, basic) {
  widget_t* w = table_row_create(NULL, 10, 20, 30, 40);
  
  ASSERT_EQ(widget_set_prop_int(w, TABLE_ROW_PROP_INDEX, 10), RET_OK);
  ASSERT_EQ(widget_get_prop_int(w, TABLE_ROW_PROP_INDEX, 0), 10);
  ASSERT_EQ(TABLE_ROW(w)->index, 10);

  widget_destroy(w);
}
