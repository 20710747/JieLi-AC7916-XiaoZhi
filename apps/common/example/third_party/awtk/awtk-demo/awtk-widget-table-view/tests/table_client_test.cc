#include "table_client/table_client.h"
#include "gtest/gtest.h"

TEST(table_client, basic) {
  widget_t* w = table_client_create(NULL, 10, 20, 30, 40);

  ASSERT_EQ(widget_set_prop_int(w, TABLE_CLIENT_PROP_ROW_HEIGHT, 100), RET_OK);
  ASSERT_EQ(widget_get_prop_int(w, TABLE_CLIENT_PROP_ROW_HEIGHT, 0), 100);
  ASSERT_EQ(TABLE_CLIENT(w)->row_height, 100);

  ASSERT_EQ(widget_set_prop_int(w, TABLE_CLIENT_PROP_ROWS, 200), RET_OK);
  ASSERT_EQ(widget_get_prop_int(w, TABLE_CLIENT_PROP_ROWS, 0), 200);
  ASSERT_EQ(TABLE_CLIENT(w)->rows, 200);
  ASSERT_EQ(table_client_get_virtual_h(w), 200 * 100);
  
  ASSERT_EQ(widget_set_prop_bool(w, TABLE_CLIENT_PROP_YSLIDABLE, TRUE), RET_OK);
  ASSERT_EQ(widget_get_prop_bool(w, TABLE_CLIENT_PROP_YSLIDABLE, FALSE), TRUE);
  ASSERT_EQ(TABLE_CLIENT(w)->yslidable, TRUE);
  
  ASSERT_EQ(widget_set_prop_int(w, TABLE_CLIENT_PROP_YOFFSET, 200), RET_OK);
  ASSERT_EQ(widget_get_prop_int(w, TABLE_CLIENT_PROP_YOFFSET, 0), 200);
  ASSERT_EQ(TABLE_CLIENT(w)->yoffset, 200);

  widget_destroy(w);
}
