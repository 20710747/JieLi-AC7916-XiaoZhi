#include "app_config.h"
#include "system/includes.h"
#include "os/os_api.h"
#include "gx_api.h"

#define DUAL_THREAD_ACCELERATION  //利用双核双线程加速,目前分割工作量实测可加速%, 但是也会占据另外一个核的%性能
#ifdef DUAL_THREAD_ACCELERATION
static OS_SEM mcu_lcd_physics_flush_sem, mcu_lcd_physics_flush_end_sem;
#endif

void  tx_kernel_enter(void)
{
    tx_application_define(NULL);
}

static unsigned short FrameBufer[LCD_W * LCD_H];
static void _mcu_lcd_physics_flush(GX_CANVAS *canvas)
{
    static unsigned short fb2[ LCD_W * LCD_H ];//由于脏矩形，下面转换会改到干净矩形的区域,所以需要多申请一块
    unsigned char *data = (unsigned char *)fb2;
    for (int i = 0; i < LCD_W * LCD_H; i++) {
        unsigned short color = FrameBufer[i] ;
        data[2 * i] = (color >> 8) & 0xff;
        data[2 * i + 1] = color & 0xff;
    }
    lcd_lvgl_full(0, LCD_W - 1, 0, LCD_H - 1, data);

#if 1
    static int fps_time_hdl;
    static unsigned int fps;
    ++fps;
    if (time_lapse(&fps_time_hdl, 1000)) {
        printf("GUIX FPS=%d\r\n", fps);
        fps = 0;
    }
#endif
}

static void mcu_lcd_physics_flush(void)
{
#ifdef DUAL_THREAD_ACCELERATION
    os_sem_pend(&mcu_lcd_physics_flush_end_sem, 0);
    os_sem_post(&mcu_lcd_physics_flush_sem);
#else
    _mcu_lcd_physics_flush();
#endif
}

#ifdef DUAL_THREAD_ACCELERATION
static void guix_mcu_lcd_flush_task(GX_CANVAS *canvas)
{
    while (1) {
        os_sem_pend(&mcu_lcd_physics_flush_sem, 0);
        _mcu_lcd_physics_flush(canvas);
        os_sem_post(&mcu_lcd_physics_flush_end_sem);
    }
}
#endif

VOID gx_jl_display_buffer_toggle(GX_CANVAS *canvas, GX_RECTANGLE *dirty)
{
    GX_RECTANGLE                  Limit;
    GX_RECTANGLE                  Copy;


    /* Driver just becomes ready. So we need to refresh the whole display. */
    static char frist = 1;
    if (frist) {
        frist = 0;

#ifdef DUAL_THREAD_ACCELERATION
        lcd_interface_set_non_block(0);// mcu lcd硬件接口要求阻塞释放cpu,否则提前释放信号有概率花屏
        os_sem_create(&mcu_lcd_physics_flush_sem, 0);
        os_sem_create(&mcu_lcd_physics_flush_end_sem, 1);
        thread_fork("guix_mcu_lcd_flush_task", 26, 200, 0, NULL, guix_mcu_lcd_flush_task, canvas);
#endif


        gx_utility_rectangle_define(&canvas -> gx_canvas_dirty_area, 0, 0,
                                    canvas -> gx_canvas_x_resolution - 1,
                                    canvas -> gx_canvas_y_resolution - 1);
    }

    gx_utility_rectangle_define(&Limit, 0, 0,
                                canvas -> gx_canvas_x_resolution - 1,
                                canvas -> gx_canvas_y_resolution - 1);

    if (gx_utility_rectangle_overlap_detect(&Limit, &canvas -> gx_canvas_dirty_area, &Copy)) {

        /* static unsigned long gx_canvas_memory2[LCD_W * LCD_H];// TODO:双BUF swap */
        /**/
        if (canvas->gx_canvas_display -> gx_display_color_format == GX_COLOR_FORMAT_565RGB) {
            memcpy(FrameBufer, canvas -> gx_canvas_memory, LCD_W * LCD_H * 2);
        } else if (canvas->gx_canvas_display -> gx_display_color_format == GX_COLOR_FORMAT_24XRGB) {

#if 0 //测试用， 显示屏不够大，剪裁中间一块出来显示
            static unsigned short crop_rgba_buf[LCD_W * LCD_H * 4];
            crop_rgba_frame(canvas -> gx_canvas_memory, canvas -> gx_canvas_x_resolution, canvas -> gx_canvas_y_resolution,
                            crop_rgba_buf, 180, 80, LCD_W, LCD_H);
            rgba_buf_to_bgr565_buf(crop_rgba_buf, FrameBufer, LCD_W, LCD_H);
#else
            rgba_buf_to_bgr565_buf(canvas -> gx_canvas_memory, FrameBufer, LCD_W, LCD_H);
#endif
        }

        /*put_buf(canvas -> gx_canvas_memory,canvas -> gx_canvas_memory_size);*/
        mcu_lcd_physics_flush();
    }
}

UINT jl_graphics_driver_setup_565rgb(void *display)
{
    _gx_display_driver_565rgb_setup(display, NULL, gx_jl_display_buffer_toggle);

    return (GX_SUCCESS);
}

UINT jl_graphics_driver_setup_24xrgb(void *display)
{
    _gx_display_driver_24xrgb_setup(display, NULL, gx_jl_display_buffer_toggle);

    return (GX_SUCCESS);
}


#include "gx_system.h"
void gx_jl_touch_event_process(u16 x, u16 y, u8 event_press)
{
    x = _gx_system_display_created_list->gx_display_width - x;
    if (x > _gx_system_display_created_list->gx_display_width) {
        x = _gx_system_display_created_list->gx_display_width;
    }

    y = _gx_system_display_created_list->gx_display_height - y;
    if (y > _gx_system_display_created_list->gx_display_height) {
        y = _gx_system_display_created_list->gx_display_height;
    }

    /*printf("gx_touch x:%d, y:%d \r\n",x,y);*/

    GX_EVENT NewEvent; //TODO: FIX_ME 重入风险?
    memset(&NewEvent, 0, sizeof(GX_EVENT));
    /*NewEvent.gx_event_sender = 0;*/
    /*NewEvent.gx_event_target = GX_NULL;*/

    NewEvent.gx_event_payload.gx_event_pointdata.gx_point_x = x;
    NewEvent.gx_event_payload.gx_event_pointdata.gx_point_y = y;
    if (event_press == 0) {
        NewEvent.gx_event_type = GX_EVENT_PEN_UP;
    } else if (event_press == 1) {
        NewEvent.gx_event_type = GX_EVENT_PEN_DOWN;
    } else if (event_press == 2) {
        NewEvent.gx_event_type = GX_EVENT_PEN_DRAG;
    }
    /*NewEvent.gx_event_display_handle = (ULONG)driver_data;*/
    gx_system_event_send(&NewEvent);
}
