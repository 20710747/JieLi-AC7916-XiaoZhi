#include "app_config.h"
#include "system/includes.h"
#include "os/os_api.h"

#ifdef USE_GUIX_UI_DEMO
static void guix_main_task(void *priv)
{
    user_lcd_init();
    while (!storage_device_ready()) {//等待sd文件系统挂载完成
        os_time_dly(2);
        puts("waitting sd on... ");
    }
    int gx_main(int argc, char **argv);
    gx_main(0, NULL);

}

static int guix_main_task_init(void)
{
    puts("guix_main_task_init \n\n");
    return thread_fork("guix_main_task", 28, 1024, 0, 0, guix_main_task, NULL);
}
late_initcall(guix_main_task_init);
#endif

