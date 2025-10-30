#include "title_value/title_value.h"
#include "gtest/gtest.h"

TEST(TitleValue, grab_focus) {
  widget_t* w = title_value_create(NULL, 0, 0, 400, 300);
  title_value_t* title_value = TITLE_VALUE(w);

  ASSERT_EQ(title_value->grab_focus, FALSE);
  ASSERT_EQ(widget_get_prop_bool(w, TITLE_VALUE_PROP_GRAB_FOCUS, TRUE), FALSE);
  ASSERT_EQ(widget_set_prop_str(w, TITLE_VALUE_PROP_GRAB_FOCUS, "true"), RET_OK);
  ASSERT_EQ(widget_get_prop_bool(w, TITLE_VALUE_PROP_GRAB_FOCUS, FALSE), TRUE);
  ASSERT_EQ(title_value->grab_focus, TRUE);

  widget_destroy(w);
}


TEST(TitleValue, return_key_to_grab_focus) {
  widget_t* w = title_value_create(NULL, 0, 0, 400, 300);
  title_value_t* title_value = TITLE_VALUE(w);

  ASSERT_EQ(title_value->return_key_to_grab_focus, FALSE);
  ASSERT_EQ(widget_get_prop_bool(w, TITLE_VALUE_PROP_RETURN_KEY_TO_GRAB_FOCUS, TRUE), FALSE);
  ASSERT_EQ(widget_set_prop_str(w, TITLE_VALUE_PROP_RETURN_KEY_TO_GRAB_FOCUS, "true"), RET_OK);
  ASSERT_EQ(widget_get_prop_bool(w, TITLE_VALUE_PROP_RETURN_KEY_TO_GRAB_FOCUS, FALSE), TRUE);
  ASSERT_EQ(title_value->return_key_to_grab_focus, TRUE);

  widget_destroy(w);
}

