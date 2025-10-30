#include "cache_view/cache_view.h"
#include "gtest/gtest.h"

TEST(cache_view, basic) {
  widget_t* w = cache_view_create(NULL, 10, 20, 30, 40);

  ASSERT_EQ(widget_set_prop_int(w, CACHE_VIEW_PROP_UPDATE_INTERVAL, 123), RET_OK);
  ASSERT_EQ(widget_get_prop_int(w, CACHE_VIEW_PROP_UPDATE_INTERVAL, 0), 123);

  widget_destroy(w);
}
