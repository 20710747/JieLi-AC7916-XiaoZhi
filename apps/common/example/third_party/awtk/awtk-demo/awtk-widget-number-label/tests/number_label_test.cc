#include "number_label/number_label.h"
#include "gtest/gtest.h"

TEST(NumberLabel, props) {
  value_t v;
  widget_t* w = number_label_create(NULL, 10, 20, 30, 40);
  number_label_t* number_label = NUMBER_LABEL(w);

  widget_set_prop_str(w, WIDGET_PROP_FORMAT, "%lf");
  ASSERT_STREQ(number_label->format, "%lf");
  ASSERT_STREQ(widget_get_prop_str(w, WIDGET_PROP_FORMAT, NULL), "%lf");

  value_set_double(&v, 1.1);
  widget_set_prop(w, WIDGET_PROP_VALUE, &v);
  ASSERT_EQ(number_label->value, 1.1);
  ASSERT_EQ(widget_get_prop(w, WIDGET_PROP_VALUE, &v), RET_OK);
  
  value_set_double(&v, 0.6);
  widget_set_prop(w, NUMBER_LABEL_PROP_DECIMAL_FONT_SIZE_SCALE, &v);
  ASSERT_EQ(number_label->decimal_font_size_scale, 0.6);
  ASSERT_EQ(widget_get_prop(w, NUMBER_LABEL_PROP_DECIMAL_FONT_SIZE_SCALE, &v), RET_OK);
  
  value_set_double(&v, 1.1);
  widget_set_prop(w, WIDGET_PROP_MIN, &v);
  ASSERT_EQ(number_label->min, 1.1);
  ASSERT_EQ(widget_get_prop(w, WIDGET_PROP_MIN, &v), RET_OK);
  
  value_set_double(&v, 10.1);
  widget_set_prop(w, WIDGET_PROP_MAX, &v);
  ASSERT_EQ(number_label->max, 10.1);
  ASSERT_EQ(widget_get_prop(w, WIDGET_PROP_MAX, &v), RET_OK);
  
  value_set_double(&v, 0.1);
  widget_set_prop(w, WIDGET_PROP_STEP, &v);
  ASSERT_EQ(number_label->step, 0.1);
  ASSERT_EQ(widget_get_prop(w, WIDGET_PROP_STEP, &v), RET_OK);
  
  value_set_bool(&v, TRUE);
  widget_set_prop(w, WIDGET_PROP_READONLY, &v);
  ASSERT_EQ(number_label->readonly, TRUE);
  ASSERT_EQ(widget_get_prop(w, WIDGET_PROP_READONLY, &v), RET_OK);
  
  widget_destroy(w);
}
