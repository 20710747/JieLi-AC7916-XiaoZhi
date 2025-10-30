#include "video_image/video_image.h"
#include "gtest/gtest.h"
#include "tkc/utils.h"

TEST(video_image, basic) {
  value_t v;
  widget_t* w = video_image_create(NULL, 10, 20, 30, 40);

  ASSERT_EQ(video_image_set_video_name(w, "test"), RET_NOT_FOUND);
  ASSERT_EQ(tk_str_cmp(VIDEO_IMAGE(w)->video_name, "test"), 0);

  ASSERT_EQ(video_image_set_delay_play(w, 1000), RET_OK);
  ASSERT_EQ(VIDEO_IMAGE(w)->delay_play, 1000);

  widget_destroy(w);
}
