#include "qr/qr.h"
#include "gtest/gtest.h"

TEST(qr, basic) {
  value_t v;
  widget_t* w = qr_create(NULL, 10, 20, 30, 40);

  widget_destroy(w);
}
