#include "usb_camera/usb_camera.h"
#include "gtest/gtest.h"

TEST(usb_camera, basic) {
  value_t v;
  widget_t* w = usb_camera_create(NULL, 10, 20, 30, 40);

  widget_destroy(w);
}
