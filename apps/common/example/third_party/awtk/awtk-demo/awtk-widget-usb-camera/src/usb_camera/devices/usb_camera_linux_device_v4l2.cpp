/**
 * File:   usb_camera_linux_device_v4l.cpp
 * Author: AWTK Develop Team
 * Brief:  Linux 的 v4l2 协议的 USB 摄像头驱动代码
 *
 * Copyright (c) 2020 - 2020 Guangzhou ZHIYUAN Electronics Co.,Ltd.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * License file for more details.
 *
 */

/**
 * History:
 * ================================================================
 * 2020-06-17 Luo ZhiMing <luozhiming@zlg.cn> created
 *
 */

#include "usb_camera_devices.h"
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include "color_format_conversion.inc"

#include <asm/types.h>
#include <linux/videodev2.h>

#ifndef V4L2_PIX_FMT_SBGGR8
#define V4L2_PIX_FMT_SBGGR8                                                    \
  v4l2_fourcc('B', 'A', '8', '1') /* 8 BGBG.. GRGR.. */
#endif

#ifndef V4L2_PIX_FMT_SN9C10X
#define V4L2_PIX_FMT_SN9C10X                                                   \
  v4l2_fourcc('S', '9', '1', '0') /* SN9C10x cmpr.  \                          \
                                     */
#endif

#ifndef V4L2_PIX_FMT_SGBRG
#define V4L2_PIX_FMT_SGBRG                                                     \
  v4l2_fourcc('G', 'B', 'R', 'G') /* bayer GBRG   GBGB.. RGRG.. */
#endif

#define MAX_V4L_BUFFERS 10
#define DEFAULT_V4L_BUFFERS 4

typedef struct _usb_camera_v4l2_buffer_t {
    void *start;
    uint32_t length;
} usb_camera_v4l2_buffer_t;

#define DEFAULT_WIDTH 640
#define DEFAULT_HEIGHT 480

#define CHANNEL_NUMBER 1
#define MAX_CAMERAS 8

#define MAX_DEVICE_DRIVER_NAME 80

typedef struct _usb_camera_device_t {
    usb_camera_device_base_t base;

    int device_handle;

    unsigned long palette;

    v4l2_input inp;
    v4l2_format form;
    timeval timestamp;
    v4l2_buf_type type;
    v4l2_capability cap;
    v4l2_requestbuffers req;
    usb_camera_v4l2_buffer_t image_data_buffers;
    usb_camera_v4l2_buffer_t buffers[MAX_V4L_BUFFERS];

} usb_camera_device_t;

static ret_t usb_camera_try_init_v4l2(usb_camera_device_t *usb_camera_device);
static ret_t
usb_camera_auto_setup_mode_v4l2(usb_camera_device_t *usb_camera_device);
static usb_camera_device_t *usb_camera_device_init(uint32_t device_id,
        uint32_t width,
        uint32_t height,
        bool_t is_mirror);

ret_t usb_camera_get_all_devices_info(slist_t *devices_list)
{
    int device_handle = 0;
    int camera_number = 0;
    char device_name[MAX_DEVICE_DRIVER_NAME] = {0};

    while (camera_number < MAX_CAMERAS) {
        sprintf(device_name, "/dev/video%d", camera_number);
        device_handle = open(device_name, O_RDONLY);
        if (device_handle != -1) {
            usb_camera_device_info_t *device_info =
                TKMEM_ZALLOC(usb_camera_device_info_t);
            device_info->camera_id = camera_number;
            memcpy(device_info->camera_name, device_name,
                   MAX_DEVICE_DRIVER_NAME < USB_CAMERA_NAME_MAX_LENGTH
                   ? MAX_DEVICE_DRIVER_NAME
                   : USB_CAMERA_NAME_MAX_LENGTH);
            slist_append(devices_list, device_info);
            close(device_handle);
        }
        camera_number++;
    }
    return RET_OK;
}

ret_t usb_camera_get_devices_ratio_list_for_device_id(uint32_t device_id,
        slist_t *ratio_list)
{
    int i = 0;
    ret_t ret = RET_OK;
    v4l2_frmsizeenum frmsize;
    usb_camera_device_t *usb_camera_device =
        usb_camera_device_init(device_id, 320, 240, FALSE);
    return_value_if_fail(usb_camera_device != NULL, RET_BAD_PARAMS);

    ret = usb_camera_try_init_v4l2(usb_camera_device);

    if (ret == RET_OK) {
        ret = usb_camera_auto_setup_mode_v4l2(usb_camera_device);
        if (ret == RET_OK) {
            frmsize.pixel_format = usb_camera_device->palette;
            for (i = 0;; i++) {
                frmsize.index = i;
                if (ioctl(usb_camera_device->device_handle, VIDIOC_ENUM_FRAMESIZES,
                          &frmsize) == -1) {
                    break;
                }
                usb_camera_ratio_t *usb_camera_ratio = TKMEM_ZALLOC(usb_camera_ratio_t);
                usb_camera_ratio->id = i;
                usb_camera_ratio->width = frmsize.discrete.width;
                usb_camera_ratio->height = frmsize.discrete.height;
                usb_camera_ratio->bpp = usb_camera_device->base.device_ratio.bpp;
                slist_append(ratio_list, usb_camera_ratio);
            }
            usb_camera_close_device(usb_camera_device);
            return RET_OK;
        }
    }
    usb_camera_close_device(usb_camera_device);
    return RET_FAIL;
}

ret_t usb_camera_get_devices_ratio_list(void *p_usb_camera_device,
                                        slist_t *ratio_list)
{
    int i = 0;
    v4l2_frmsizeenum frmsize;
    usb_camera_device_t *usb_camera_device = NULL;
    return_value_if_fail(p_usb_camera_device != NULL, RET_BAD_PARAMS);

    usb_camera_device = (usb_camera_device_t *)p_usb_camera_device;

    frmsize.pixel_format = usb_camera_device->palette;

    for (i = 0;; i++) {
        frmsize.index = i;
        if (ioctl(usb_camera_device->device_handle, VIDIOC_ENUM_FRAMESIZES,
                  &frmsize) == -1) {
            break;
        }
        usb_camera_ratio_t *usb_camera_ratio = TKMEM_ZALLOC(usb_camera_ratio_t);
        usb_camera_ratio->id = i;
        usb_camera_ratio->width = frmsize.discrete.width;
        usb_camera_ratio->height = frmsize.discrete.height;
        usb_camera_ratio->bpp = usb_camera_device->base.device_ratio.bpp;
        slist_append(ratio_list, usb_camera_ratio);
    }
    return RET_OK;
}

static ret_t usb_camera_try_init_v4l2(usb_camera_device_t *usb_camera_device)
{
    int device_index = 0;
    usb_camera_device_info_t *d_info = NULL;
    char device_name[MAX_DEVICE_DRIVER_NAME] = {0};
    slist_t *devices_list = slist_create(usb_camera_devices_info_list_destroy,
                                         usb_camera_devices_info_list_compare);

    usb_camera_get_all_devices_info(devices_list);

    if (slist_size(devices_list) == 0) {
        slist_destroy(devices_list);
        return RET_FAIL;
    }

    d_info = (usb_camera_device_info_t *)slist_find(
                 devices_list, &(usb_camera_device->base.device_info));

    if (d_info == NULL) {
        d_info = (usb_camera_device_info_t *)devices_list->first;
    }

    memcpy(usb_camera_device->base.device_info.camera_name, d_info->camera_name,
           USB_CAMERA_NAME_MAX_LENGTH);
    usb_camera_device->base.device_info.camera_id = d_info->camera_id;

    sprintf(device_name, "/dev/video%d",
            usb_camera_device->base.device_info.camera_id);
    usb_camera_device->device_handle = open(device_name, O_RDWR | O_NONBLOCK, 0);
    if (-1 == usb_camera_device->device_handle) {
        slist_destroy(devices_list);
        printf("try_init_v4l2 open \"%s\": %s\n", device_name, strerror(errno));
        return RET_FAIL;
    }
    slist_destroy(devices_list);

    memset(&usb_camera_device->cap, 0, sizeof(v4l2_capability));
    if (-1 == ioctl(usb_camera_device->device_handle, VIDIOC_QUERYCAP,
                    &usb_camera_device->cap)) {
        printf("(DEBUG) try_init_v4l2 VIDIOC_QUERYCAP \"%s\": %s\n", device_name,
               strerror(errno));
        return RET_FAIL;
    }

    if (-1 ==
        ioctl(usb_camera_device->device_handle, VIDIOC_G_INPUT, &device_index)) {
        printf("(DEBUG) try_init_v4l2 VIDIOC_G_INPUT \"%s\": %s\n", device_name,
               strerror(errno));
        return RET_FAIL;
    }

    memset(&usb_camera_device->inp, 0, sizeof(v4l2_input));
    usb_camera_device->inp.index = device_index;
    if (-1 == ioctl(usb_camera_device->device_handle, VIDIOC_ENUMINPUT,
                    &usb_camera_device->inp)) {
        printf("(DEBUG) try_init_v4l2 VIDIOC_ENUMINPUT \"%s\": %s\n", device_name,
               strerror(errno));
        return RET_FAIL;
    }
    return RET_OK;
}

static ret_t usb_camera_try_palette_v4l2(usb_camera_device_t *usb_camera_device,
        unsigned long pixel_format)
{
    return_value_if_fail(usb_camera_device != NULL, RET_BAD_PARAMS);

    memset(&usb_camera_device->form, 0, sizeof(v4l2_format));

    usb_camera_device->form.fmt.pix.field = V4L2_FIELD_NONE;
    usb_camera_device->form.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    usb_camera_device->form.fmt.pix.pixelformat = pixel_format;
    usb_camera_device->form.fmt.pix.width =
        usb_camera_device->base.device_ratio.width;
    usb_camera_device->form.fmt.pix.height =
        usb_camera_device->base.device_ratio.height;

    return_value_if_fail(ioctl(usb_camera_device->device_handle, VIDIOC_S_FMT,
                               &usb_camera_device->form) != -1,
                         RET_FAIL);

    if (pixel_format != usb_camera_device->form.fmt.pix.pixelformat) {
        return RET_FAIL;
    } else {
        if (usb_camera_device->form.fmt.pix.width !=
            usb_camera_device->base.device_ratio.width ||
            usb_camera_device->form.fmt.pix.height !=
            usb_camera_device->base.device_ratio.height) {
            usb_camera_device->base.device_ratio.width = USB_CAMERA_DEFAULT_WIDTH;
            usb_camera_device->base.device_ratio.height = USB_CAMERA_DEFAULT_HEIGHT;
            usb_camera_try_palette_v4l2(usb_camera_device, pixel_format);
        }
        usb_camera_device->base.device_ratio.bpp =
            usb_camera_device->form.fmt.pix.bytesperline /
            usb_camera_device->base.device_ratio.width;
        return RET_OK;
    }
}

static ret_t
usb_camera_auto_setup_mode_v4l2(usb_camera_device_t *usb_camera_device)
{
    return_value_if_fail(usb_camera_device != NULL, RET_BAD_PARAMS);

    if (usb_camera_try_palette_v4l2(usb_camera_device, V4L2_PIX_FMT_BGR24) ==
        RET_OK) {
        usb_camera_device->palette = V4L2_PIX_FMT_BGR24;
    } else if (usb_camera_try_palette_v4l2(usb_camera_device,
                                           V4L2_PIX_FMT_RGB565) == RET_OK) {
        usb_camera_device->palette = V4L2_PIX_FMT_RGB565;
    } else if (usb_camera_try_palette_v4l2(usb_camera_device,
                                           V4L2_PIX_FMT_YVU420) == RET_OK) {
        usb_camera_device->palette = V4L2_PIX_FMT_YVU420;
    } else if (usb_camera_try_palette_v4l2(usb_camera_device,
                                           V4L2_PIX_FMT_YUV411P) == RET_OK) {
        usb_camera_device->palette = V4L2_PIX_FMT_YUV411P;
    } else
#if 0
        if (usb_camera_try_palette_v4l2(usb_camera_device, V4L2_PIX_FMT_MJPEG) == RET_OK ||
            usb_camera_try_palette_v4l2(usb_camera_device, V4L2_PIX_FMT_JPEG) == RET_OK) {
            usb_camera_device->palette = V4L2_PIX_FMT_JPEG;
        } else
#endif
            if (usb_camera_try_palette_v4l2(usb_camera_device, V4L2_PIX_FMT_YUYV) ==
                RET_OK) {
                usb_camera_device->palette = V4L2_PIX_FMT_YUYV;
            } else if (usb_camera_try_palette_v4l2(usb_camera_device,
                                                   V4L2_PIX_FMT_UYVY) == RET_OK) {
                usb_camera_device->palette = V4L2_PIX_FMT_UYVY;
            }
#if 0
            else  if (usb_camera_try_palette_v4l2(usb_camera_device, V4L2_PIX_FMT_SN9C10X) == RET_OK) {
                usb_camera_device->palette = V4L2_PIX_FMT_SN9C10X;
            } else if (usb_camera_try_palette_v4l2(usb_camera_device, V4L2_PIX_FMT_SBGGR8) == RET_OK) {
                usb_camera_device->palette = V4L2_PIX_FMT_SBGGR8;
            } else if (usb_camera_try_palette_v4l2(usb_camera_device, V4L2_PIX_FMT_SGBRG) == RET_OK) {
                usb_camera_device->palette = V4L2_PIX_FMT_SGBRG;
            }
#endif
            else {
                return RET_FAIL;
            }
    return RET_OK;
}

static ret_t
usb_camera_get_buffer_number(usb_camera_device_t *usb_camera_device)
{
    uint32_t buffer_number = DEFAULT_V4L_BUFFERS;
    do {
        usb_camera_device->req.count = buffer_number;
        usb_camera_device->req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        usb_camera_device->req.memory = V4L2_MEMORY_MMAP;

        if (-1 == ioctl(usb_camera_device->device_handle, VIDIOC_REQBUFS,
                        &usb_camera_device->req)) {
            if (EINVAL == errno) {
                printf("/dev/video%d does not support memory mapping\n",
                       usb_camera_device->base.device_info.camera_id);
            } else {
                printf("VIDIOC_REQBUFS \n");
            }
            return RET_FAIL;
        }

        if (usb_camera_device->req.count < buffer_number) {
            if (buffer_number == 1) {
                printf("Insufficient buffer memory on /dev/video%d\n",
                       usb_camera_device->base.device_info.camera_id);
                return RET_FAIL;
            } else {
                buffer_number--;
                printf("Insufficient buffer memory on /dev/video%d -- decreaseing "
                       "buffers\n",
                       usb_camera_device->base.device_info.camera_id);
            }
        } else {
            break;
        }
    } while (true);

    return RET_OK;
}

static ret_t usb_camera_buffer_init(usb_camera_device_t *usb_camera_device)
{
    for (uint32_t n_buffers = 0;
         n_buffers < (uint32_t)usb_camera_device->req.count; ++n_buffers) {
        v4l2_buffer buf;

        memset(&buf, 0, sizeof(v4l2_buffer));

        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = n_buffers;

        if (-1 == ioctl(usb_camera_device->device_handle, VIDIOC_QUERYBUF, &buf)) {
            printf("VIDIOC_QUERYBUF \n");
            return RET_FAIL;
        }

        usb_camera_device->buffers[n_buffers].length = buf.length;
        usb_camera_device->buffers[n_buffers].start =
            mmap(NULL, buf.length, PROT_READ | PROT_WRITE, MAP_SHARED,
                 usb_camera_device->device_handle, buf.m.offset);

        if (MAP_FAILED == usb_camera_device->buffers[n_buffers].start) {
            printf("mmap \n");
            return RET_FAIL;
        }

        if (n_buffers == 0) {
            usb_camera_device->image_data_buffers.start = malloc(buf.length);
            usb_camera_device->image_data_buffers.length = buf.length;
        }
    }
    return RET_OK;
}

static ret_t usb_camera_start(usb_camera_device_t *usb_camera_device)
{
    ret_t ret = RET_OK;
    for (uint32_t i = 0; i < ((uint32_t)usb_camera_device->req.count); i++) {
        struct v4l2_buffer buf;
        memset(&buf, 0, sizeof buf);
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = i;
        if (ioctl(usb_camera_device->device_handle, VIDIOC_QBUF, &buf) == -1) {
            printf("VIDIOC_QBUF \n");
            ret = RET_FAIL;
        }
    }

    v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (ioctl(usb_camera_device->device_handle, VIDIOC_STREAMON, &type) == -1) {
        printf("VIDIOC_STREAMON \n");
        ret = RET_FAIL;
    }

    return ret;
}

static ret_t
usb_camera_read_frame_v4l2(usb_camera_device_t *usb_camera_device)
{
    v4l2_buffer buf;
    memset(&buf, 0, sizeof(buf));
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    if (ioctl(usb_camera_device->device_handle, VIDIOC_DQBUF, &buf) == -1) {
        printf("ioctl(usb_camera_device->device_handle, VIDIOC_DQBUF, &buf), "
               "device_handle:%d \r\n",
               usb_camera_device->device_handle);
        return RET_FAIL;
    }

    if (buf.index < usb_camera_device->req.count) {
        memcpy(usb_camera_device->image_data_buffers.start,
               usb_camera_device->buffers[buf.index].start, buf.bytesused);

        usb_camera_device->image_data_buffers.length = buf.bytesused;

        if (ioctl(usb_camera_device->device_handle, VIDIOC_QBUF, &buf) == -1) {
            printf("ioctl(usb_camera_device->device_handle, VIDIOC_QBUF, &buf), "
                   "device_handle:%d \r\n",
                   usb_camera_device->device_handle);
            return RET_FAIL;
        }

        usb_camera_device->timestamp = buf.timestamp;
        return RET_OK;
    }
    return RET_OK;
}

static ret_t
usb_camera_read_image_data_v4l2(usb_camera_device_t *usb_camera_device)
{
    int r = 0;
    fd_set fds;
    timeval tv;

    FD_ZERO(&fds);
    FD_SET(usb_camera_device->device_handle, &fds);

    tv.tv_sec = 10;
    tv.tv_usec = 0;

    r = select(usb_camera_device->device_handle + 1, &fds, NULL, NULL, &tv);

    if (-1 == r) {
        printf("select \r\n");
        return RET_FAIL;
    }

    if (0 == r) {
        printf("select timeout \r\n");
        return RET_FAIL;
    }

    return usb_camera_read_frame_v4l2(usb_camera_device);
}

static usb_camera_device_t *usb_camera_device_init(uint32_t device_id,
        uint32_t width,
        uint32_t height,
        bool_t is_mirror)
{
    usb_camera_device_t *usb_camera_device = TKMEM_ZALLOC(usb_camera_device_t);
    if (usb_camera_device != NULL) {
        usb_camera_device->base.device_info.camera_id = device_id;
        usb_camera_device->base.is_mirror = is_mirror;

        usb_camera_device->base.device_ratio.width = width;   // DEFAULT_WIDTH;
        usb_camera_device->base.device_ratio.height = height; // DEFAULT_HEIGHT;
    }
    return usb_camera_device;
}

void *usb_camera_start_device(uint32_t device_id, uint32_t width,
                              uint32_t height, bool_t is_mirror)
{
    ret_t ret = RET_OK;
    usb_camera_device_t *usb_camera_device =
        usb_camera_device_init(device_id, width, height, is_mirror);

    ret = usb_camera_try_init_v4l2(usb_camera_device);

    if (ret != RET_OK &&
        (usb_camera_device->cap.capabilities & V4L2_CAP_VIDEO_CAPTURE) == 0) {
        printf("HIGHGUI ERROR: V4L2: device /dev/video%d is unable to capture "
               "video memory.\r\n",
               device_id);
        usb_camera_close_device(usb_camera_device);
        return NULL;
    }

    if (usb_camera_device->inp.index > 0) {
        memset(&usb_camera_device->inp, 0, sizeof(v4l2_input));
        usb_camera_device->inp.index = CHANNEL_NUMBER;
        if (-1 == ioctl(usb_camera_device->device_handle, VIDIOC_ENUMINPUT,
                        &usb_camera_device->inp)) {
            printf("HIGHGUI ERROR: V4L2: Aren't able to set channel number\r\n");
            usb_camera_close_device(usb_camera_device);
            return NULL;
        }
    }

    memset(&usb_camera_device->form, 0, sizeof(v4l2_format));
    usb_camera_device->form.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (-1 == ioctl(usb_camera_device->device_handle, VIDIOC_G_FMT,
                    &usb_camera_device->form)) {
        printf("HIGHGUI ERROR: V4L2: Could not obtain specifics of capture "
               "window.\r\n");
        usb_camera_close_device(usb_camera_device);
        return NULL;
    }

    if (usb_camera_auto_setup_mode_v4l2(usb_camera_device) != RET_OK) {
        printf("usb_camera_auto_setup_mode_v4l2 fail \r\n");
        usb_camera_close_device(usb_camera_device);
        return NULL;
    }

    if (usb_camera_get_buffer_number(usb_camera_device) != RET_OK) {
        printf("usb_camera_get_buffer_number fail \r\n");
        usb_camera_close_device(usb_camera_device);
        return NULL;
    }

    if (usb_camera_buffer_init(usb_camera_device) != RET_OK) {
        printf("usb_camera_buffer_init fail \r\n");
        usb_camera_close_device(usb_camera_device);
        return NULL;
    }

    if (usb_camera_start(usb_camera_device) != RET_OK) {
        printf("usb_camera_start fail \r\n");
        usb_camera_close_device(usb_camera_device);
        return NULL;
    }

    return usb_camera_device;
}

void *usb_camera_open_device(uint32_t device_id, bool_t is_mirror)
{
    return usb_camera_start_device(device_id, USB_CAMERA_DEFAULT_WIDTH,
                                   USB_CAMERA_DEFAULT_HEIGHT, is_mirror);
}

void *usb_camera_open_device_with_width_and_height(uint32_t device_id,
        bool_t is_mirror,
        uint32_t width,
        uint32_t height)
{
    return usb_camera_start_device(device_id, width, height, is_mirror);
}

void *usb_camera_set_ratio(void *p_usb_camera_device, uint32_t width,
                           uint32_t height)
{
    uint32_t device_id = 0;
    bool_t is_mirror = FALSE;
    usb_camera_device_t *usb_camera_device = NULL;

    return_value_if_fail(p_usb_camera_device != NULL, NULL);

    usb_camera_device = (usb_camera_device_t *)p_usb_camera_device;

    return_value_if_fail(
        usb_camera_check_ratio(p_usb_camera_device, width, height) == RET_OK,
        NULL);

    usb_camera_device = (usb_camera_device_t *)p_usb_camera_device;

    is_mirror = usb_camera_device->base.is_mirror;
    device_id = usb_camera_device->base.device_info.camera_id;
    usb_camera_close_device(usb_camera_device);
    sleep(1);
    return usb_camera_start_device(device_id, width, height, is_mirror);
}

ret_t usb_camera_close_device(void *p_usb_camera_device)
{
    usb_camera_device_t *usb_camera_device = NULL;
    return_value_if_fail(p_usb_camera_device != NULL, RET_BAD_PARAMS);

    usb_camera_device = (usb_camera_device_t *)p_usb_camera_device;

    usb_camera_device->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (-1 == ioctl(usb_camera_device->device_handle, VIDIOC_STREAMOFF,
                    &usb_camera_device->type)) {
        printf("Unable to stop the stream.");
    }

    for (unsigned int n_buffers_ = 0; n_buffers_ < usb_camera_device->req.count;
         ++n_buffers_) {
        if (-1 == munmap(usb_camera_device->buffers[n_buffers_].start,
                         usb_camera_device->buffers[n_buffers_].length)) {
            printf("munmap");
        }
    }

    if (usb_camera_device->image_data_buffers.start) {
        free(usb_camera_device->image_data_buffers.start);
        usb_camera_device->image_data_buffers.start = NULL;
        usb_camera_device->image_data_buffers.length = 0;
    }

    if (usb_camera_device->device_handle != -1) {
        close(usb_camera_device->device_handle);
    }
    TKMEM_FREE(p_usb_camera_device);

    return RET_OK;
}

static ret_t color_format_conver(unsigned char *src, unsigned long palette,
                                 unsigned int width, unsigned int height,
                                 unsigned int bpp, unsigned char *dst,
                                 bitmap_format_t format, unsigned int dst_len,
                                 unsigned char is_mirror,
                                 unsigned char is_reverse)
{
    uint32_t error = -1;
    if (palette == V4L2_PIX_FMT_BGR24 && format == BITMAP_FMT_BGRA8888) {
        error = color_format_conversion_data_fun(COLOR_FORMAT_BGR2BGRA, src, width,
                height, bpp, dst, dst_len,
                is_mirror, is_reverse);
    } else if (palette == V4L2_PIX_FMT_BGR24 && format == BITMAP_FMT_RGBA8888) {
        error = color_format_conversion_data_fun(COLOR_FORMAT_BGR2RGBA, src, width,
                height, bpp, dst, dst_len,
                is_mirror, is_reverse);
    } else if (palette == V4L2_PIX_FMT_BGR24 && format == BITMAP_FMT_BGR565) {
        error = color_format_conversion_data_fun(COLOR_FORMAT_BGR2BGR565, src,
                width, height, bpp, dst, dst_len,
                is_mirror, is_reverse);
    } else if (palette == V4L2_PIX_FMT_BGR24 && format == BITMAP_FMT_RGB565) {
        error = color_format_conversion_data_fun(COLOR_FORMAT_BGR2RGB565, src,
                width, height, bpp, dst, dst_len,
                is_mirror, is_reverse);
    } else if (palette == V4L2_PIX_FMT_YUYV && format == BITMAP_FMT_BGRA8888) {
        error = color_format_conversion_data_fun(COLOR_FORMAT_YUYV2BGRA, src, width,
                height, bpp, dst, dst_len,
                is_mirror, is_reverse);
    } else if (palette == V4L2_PIX_FMT_YUYV && format == BITMAP_FMT_BGR565) {
        error = color_format_conversion_data_fun(COLOR_FORMAT_YUYV2BGR565, src,
                width, height, bpp, dst, dst_len,
                is_mirror, is_reverse);
    } else if (palette == V4L2_PIX_FMT_YUYV && format == BITMAP_FMT_RGBA8888) {
        error = color_format_conversion_data_fun(COLOR_FORMAT_YUYV2RGBA, src, width,
                height, bpp, dst, dst_len,
                is_mirror, is_reverse);
    } else if (palette == V4L2_PIX_FMT_YUYV && format == BITMAP_FMT_RGB565) {
        error = color_format_conversion_data_fun(COLOR_FORMAT_YUYV2RGB565, src,
                width, height, bpp, dst, dst_len,
                is_mirror, is_reverse);
    } else if (palette == V4L2_PIX_FMT_UYVY && format == BITMAP_FMT_BGRA8888) {
        error = color_format_conversion_data_fun(COLOR_FORMAT_UYVY2BGRA, src, width,
                height, bpp, dst, dst_len,
                is_mirror, is_reverse);
    } else if (palette == V4L2_PIX_FMT_UYVY && format == BITMAP_FMT_BGR565) {
        error = color_format_conversion_data_fun(COLOR_FORMAT_UYVY2BGR565, src,
                width, height, bpp, dst, dst_len,
                is_mirror, is_reverse);
    } else if (palette == V4L2_PIX_FMT_UYVY && format == BITMAP_FMT_RGBA8888) {
        error = color_format_conversion_data_fun(COLOR_FORMAT_UYVY2RGBA, src, width,
                height, bpp, dst, dst_len,
                is_mirror, is_reverse);
    } else if (palette == V4L2_PIX_FMT_UYVY && format == BITMAP_FMT_RGB565) {
        error = color_format_conversion_data_fun(COLOR_FORMAT_UYVY2RGB565, src,
                width, height, bpp, dst, dst_len,
                is_mirror, is_reverse);
    } else {
        printf("color_format_conversion fail \r\n");
        error = 1;
    }

    return error == 0 ? RET_OK : RET_FAIL;
}

ret_t usb_camera_get_video_image_data(void *p_usb_camera_device,
                                      unsigned char *data,
                                      bitmap_format_t format,
                                      uint32_t data_len)
{
    ret_t ret = RET_OK;
    usb_camera_device_t *usb_camera_device = NULL;
    return_value_if_fail(p_usb_camera_device != NULL, RET_BAD_PARAMS);

    usb_camera_device = (usb_camera_device_t *)p_usb_camera_device;

    ret = usb_camera_read_image_data_v4l2(usb_camera_device);

    if (ret == RET_OK &&
        usb_camera_device->image_data_buffers.length ==
        usb_camera_device->base.device_ratio.width *
        usb_camera_device->base.device_ratio.height *
        usb_camera_device->base.device_ratio.bpp) {
        return color_format_conver(
                   (unsigned char *)usb_camera_device->image_data_buffers.start,
                   usb_camera_device->palette, usb_camera_device->base.device_ratio.width,
                   usb_camera_device->base.device_ratio.height,
                   usb_camera_device->base.device_ratio.bpp, data, format, data_len,
                   usb_camera_device->base.is_mirror, TRUE);
    } else {
        printf("usb_camera_get_video_image_data -- buffer sizes do not match \r\n");
        return RET_FAIL;
    }
}
