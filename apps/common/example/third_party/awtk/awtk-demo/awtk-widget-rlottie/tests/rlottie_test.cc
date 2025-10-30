#include "rlottie/rlottie.h"
#include "gtest/gtest.h"

TEST(rlottie, basic) {
  value_t v;
  widget_t* w = rlottie_create(NULL, 10, 20, 30, 40);

  widget_destroy(w);
}
