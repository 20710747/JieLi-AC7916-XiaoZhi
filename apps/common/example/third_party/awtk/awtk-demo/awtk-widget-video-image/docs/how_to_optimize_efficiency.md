# 优化 video_image 控件效率

video_image 控件的缺省实现如下：

* 解析视频文件，计算每一帧的位图数据，并保存到 bitmap 对象中。
* 将 bitmap 对象中的图像数据拷贝到 lcd 显存中显示。

这存在一个问题：一般情况下 video_image 控件的分辨率都比较大，把 bitmap 对象中的数据拷贝到 lcd 显存上面显示的效率会比较低，特别是使用 video_image 控件播放开机动画时，效率问题尤为明显。

video_image 控件提供了一些可重载的函数接口，可以通过重载这些函数，将解析好的位图数据直接绘制到 lcd 显存上，减少把 bitmap 数据拷贝到 lcd 显存上的过程，从而提高效率。

> 由于 video_image 控件会解析出来的位图数据保存到 bitmap 上面，所以我们需要把 lcd 的 buffer 作为 bitmap 的 buffer ，然后把该 bitmap 给 video_image 控件使用。

支持该用法需设置的属性：

| 属性             | 作用                                   | 备注                                |
| ---------------- | -------------------------------------- | ----------------------------------- |
| draw_video_image | 是否自动绘制视频贴图数据（默认为TRUE） | 自定义绘制流程时需将该属性设为FALSE |

重载 video_image 控件相关接口时需调用的函数：

| 函数                                | 作用                                                                         |
| ----------------------------------- | ---------------------------------------------------------------------------- |
| video_image_set_init_bitmap_func    | 设置自定义初始化序列帧位图函数（需返回RET_OK，否则使用默认的方法创建位图。） |
| video_image_set_dispose_image_func  | 设置自定义释放序列帧位图函数                                                 |
| video_image_set_draw_image_func     | 设置自定义绘制序列帧位图函数                                                 |
| video_image_set_to_image_begin_func | 设置解码序列帧前回调函数                                                     |
| video_image_set_to_image_end_func   | 设置解码序列帧后回调函数                                                     |

使用该用法优化效率时，需要根据实际平台的 lcd 实现来编写代码，此处以 PC 上的 AGGE 渲染模式为例，示例代码如下：

```c
#include "lcd/lcd_mem.h"
#include "lcd/lcd_mem_special.h"

/* 销毁序列帧位图（需根据实际平台编写）*/
ret_t video_image_dispose_image(void* ctx, bitmap_t* bitmap) {
  bitmap->w = 0;
  bitmap->h = 0;
  bitmap->format = 0;
  bitmap->buffer = NULL;
  return RET_OK;
}

/* 初始化序列帧位图（需根据实际平台编写）*/
ret_t video_image_init_image(void* ctx, bitmap_t* bitmap, uint32_t w, uint32_t h, 
                             uint32_t channels, uint32_t line_length, bitmap_format_t format) {
  canvas_t* c = widget_get_canvas(window_manager());
  lcd_t* lcd = c->lcd;
  lcd_mem_special_t* special = (lcd_mem_special_t*)lcd;
  bitmap_format_t lcd_format = lcd_get_desired_bitmap_format(lcd);
  uint32_t lcd_channels = bitmap_get_bpp_of_format(lcd_format);

  if (lcd->w != w || lcd->h != h || lcd_channels != channels || lcd_format != format || 
      line_length != special->lcd_mem->line_length) {
    return RET_FAIL;
  }

  /* 把 offline buffer 设置到 bitmap 上面，让序列帧的位图数据保存到 offline buffer 上面。 */
  bitmap->w = lcd->w;
  bitmap->h = lcd->h;
  bitmap->format = special->lcd_mem->format;
  bitmap->buffer = special->lcd_mem->offline_gb;
  graphic_buffer_attach(special->lcd_mem->offline_gb, special->lcd_mem->offline_fb, w, h);
  bitmap_set_line_length(bitmap, tk_max(lcd_channels * w, special->lcd_mem->line_length));

  return RET_OK;
}

ret_t application_init(void) {  
  widget_t* win = window_open("main");
  widget_t* video_image = widget_lookup_by_type(win, WIDGET_TYPE_VIDEO_IMAGE, TRUE);
  
  /* 将 video_image 控件的 draw_video_image 属性设为 FALSE */
  video_image_set_draw_video_image(video_image, FALSE);

  /* 重载 init_bitmap 函数 */
  video_image_set_init_bitmap_func(video_image, video_image_init_image, NULL);

  /* 设置 video_name，指定播放的视频文件（必须在重载 init_bitmap 函数后设置） */
  video_image_set_video_name(video_image, "video_12");

  /* 重载 dispose_image 函数 */
  video_image_set_dispose_image_func(video_image, video_image_dispose_image, NULL);
  ...

  return RET_OK;
}
```

> 注意事项：
> 1. 一定要在设置 video_name 之前重载 init_bitmap 函数，这是由于设置 video_name 时就会调用 init_bitmap 创建位图对象。
> 2. 如果UI文件中有设置 video_name，就需要在 video_image_set_video_name 函数后才再重载 dispose_image 函数。
