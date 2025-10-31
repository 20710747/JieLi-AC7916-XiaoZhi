# AC79 SDK 项目启动流程详解

## 📚 目录
1. [启动流程总览](#启动流程总览)
2. [启动阶段详解](#启动阶段详解)
3. [关键数据结构](#关键数据结构)
4. [初始化机制](#初始化机制)
5. [应用程序启动](#应用程序启动)
6. [任务列表](#任务列表)
7. [启动流程图](#启动流程图)

---

## 1️⃣ 启动流程总览

```
┌──────────────────────────────────────────────────┐
│           硬件复位 / 上电启动                    │
└────────────────┬─────────────────────────────────┘
                 │
                 ▼
┌──────────────────────────────────────────────────┐
│   阶段1: 引导加载 (Bootloader)                   │
│   - 从Flash加载代码到内存                        │
│   - _start 入口点                                │
└────────────────┬─────────────────────────────────┘
                 │
                 ▼
┌──────────────────────────────────────────────────┐
│   阶段2: main() 函数 (apps/common/system/init.c)│
│   ├─ setup_arch()       // CPU架构初始化         │
│   ├─ os_init()          // 操作系统初始化        │
│   ├─ task_create()      // 创建app_core任务     │
│   └─ os_start()         // 启动RTOS调度器        │
└────────────────┬─────────────────────────────────┘
                 │
                 ▼
┌──────────────────────────────────────────────────┐
│   阶段3: app_task_handler() 任务                 │
│   ├─ sys_timer_init()         // 系统定时器      │
│   ├─ board_early_init()       // 板级早期初始化  │
│   ├─ __do_initcall(early)     // 早期初始化回调  │
│   ├─ __do_initcall(platform)  // 平台初始化      │
│   ├─ board_init()             // 板级初始化      │
│   ├─ __do_initcall(initcall)  // 模块初始化      │
│   ├─ __do_initcall(module)    // 模块初始化回调  │
│   ├─ app_core_init()          // 应用核心初始化  │
│   ├─ __do_initcall(late)      // 后期初始化回调  │
│   └─ app_main()               // 应用主函数      │
└────────────────┬─────────────────────────────────┘
                 │
                 ▼
┌──────────────────────────────────────────────────┐
│   阶段4: app_main() 应用启动                     │
│   ├─ init_intent()            // 初始化意图      │
│   ├─ start_app()              // 启动应用        │
│   └─ 进入应用主循环                              │
└────────────────┬─────────────────────────────────┘
                 │
                 ▼
┌──────────────────────────────────────────────────┐
│   阶段5: 主循环 (事件驱动)                      │
│   - 处理系统事件                                 │
│   - 处理按键事件                                 │
│   - 处理网络事件                                 │
│   - 处理蓝牙事件                                 │
└──────────────────────────────────────────────────┘
```

---

## 2️⃣ 启动阶段详解

### 🔧 阶段1: Bootloader和链接脚本

**位置**: `cpu/wl82/sdk_ld_sfc.c` / `cpu/wl82/sdk_ld_sdram.c`

**内存布局**:
```c
MEMORY
{
    rom(rx)       : ORIGIN = 0x2000120, LENGTH = __FLASH_SIZE__  // Flash ROM
    sdram(rwx)    : ORIGIN = 0x4000120, LENGTH = SDRAM_SIZE      // SDRAM
    ram0(rwx)     : ORIGIN = 0x1c00000, LENGTH = RAM0_SIZE       // 内部RAM
    boot_info(rwx): ORIGIN = ..., LENGTH = BOOT_INFO_SIZE        // 启动信息
    cache_ram(rw) : ORIGIN = 0x1f20000, LENGTH = CACHE_RAM_SIZE  // Cache RAM
}
```

**关键配置**:
- `BOOT_INFO_SIZE = 52` 字节
- `RAM0_SIZE` = 约8MB内部RAM
- `SDRAM_SIZE` = 2MB外部SDRAM（可选）
- `TLB_SIZE` = MMU页表大小（如启用MMU）

**链接段**:
```
.text     → 代码段 (Flash或SDRAM)
.data     → 初始化数据段 (SDRAM)
.bss      → 未初始化数据段 (SDRAM)
.stack    → 堆栈段 (RAM0)
```

---

### 🚀 阶段2: main() 函数

**位置**: `apps/common/system/init.c:264`

```c
int main()
{
    // 1. 禁用中断
    __local_irq_disable();

    // 2. CPU架构初始化
    setup_arch();

    // 3. 启用第二个CPU核心（双核模式）
    #if CPU_CORE_NUM == 1
        EnableOtherCpu();
    #endif

    // 4. 操作系统初始化
    os_init();

    // 5. 创建app核心任务
    task_create(app_task_handler, NULL, "app_core");

    // 6. 启动RTOS调度器
    os_start();

    // 7. 使能中断
    __local_irq_enable();

    return 0;
}
```

#### 📌 2.1 setup_arch() - CPU架构初始化

**位置**: `cpu/wl82/setup.c:6`

```c
void setup_arch()
{
    // 1. 看门狗初始化
    wdt_init(0x0f);

    // 2. 时钟早期初始化
    clk_early_init();

    // 3. 中断控制器初始化
    interrupt_init();

    // 4. 调试串口初始化 (如启用DEBUG)
    #ifdef CONFIG_DEBUG_ENABLE
        debug_uart_init();
        log_early_init(10 * 1024);  // 10KB日志缓冲
    #endif

    // 5. 获取系统复位原因
    system_reset_reason_get();

    // 6. 打印芯片信息
    printf("WL82(AC791N) CHIP_ID: 0x%x", JL_INTEST->CHIP_ID);
    
    // 7. 打印内存布局
    printf("sys_clk = %d, sdram_clk = %d", clk_get("sys"), clk_get("sdram"));
    printf("SDRAM_SIZE = %d, DATA_SIZE = %d, BSS_SIZE = %d", ...);
    printf("RAM_SIZE = %d, HEAP_SIZE = %d", ...);

    // 8. 可移动代码初始化
    code_movable_init();

    // 9. Debug系统初始化
    debug_init();

    // 10. CRC16互斥锁初始化
    __crc16_mutex_init();

    // 11. P33 IO锁存初始化
    p33_io_latch_init();
}
```

**时钟配置**:
- `sys_clk`: 系统时钟（通常320MHz）
- `sdram_clk`: SDRAM时钟
- `hsb_clk`: 高速总线时钟
- `lsb_clk`: 低速总线时钟
- `sfc_clk`: SPI Flash控制器时钟

---

### 🎯 阶段3: app_task_handler() - 应用任务

**位置**: `apps/common/system/init.c:180`

```c
static void app_task_handler(void *p)
{
    // ====== 第一部分: 系统级初始化 ======
    
    // 1. 系统定时器初始化
    sys_timer_init();
    sys_timer_task_init();

    // 2. RTOS栈检查 (如启用)
    #ifdef RTOS_STACK_CHECK_ENABLE
        sys_timer_add(NULL, rtos_stack_check_func, 60 * 1000);
    #endif

    // 3. 内存泄漏检测 (如启用)
    #ifdef MEM_LEAK_CHECK_ENABLE
        malloc_debug_start();
        sys_timer_add(NULL, malloc_debug_dump, 60 * 1000);
    #endif

    // ====== 第二部分: 板级初始化 ======
    
    // 4. 板级早期初始化 (弱函数,可覆盖)
    board_early_init();

    // 5. 早期初始化回调
    __do_initcall(early_initcall);
    /*
       执行所有通过 early_initcall() 注册的函数:
       - mutex_init()
       - sys_event_init()
       - thread_fork_init()
       - sdfile_init()
       - ntp_client_init()
    */

    // 6. SD卡外部文件系统挂载
    #ifdef CONFIG_SDFILE_EXT_ENABLE
        sdfile_ext_mount_init();
    #endif

    // 7. 平台初始化回调
    __do_initcall(platform_initcall);

    // 8. 板级初始化 (弱函数,可覆盖)
    board_init();

    // ====== 第三部分: 模块初始化 ======

    // 9. RF FCC测试模式检测
    #ifdef RF_FCC_TEST_ENABLE
        if (rf_fcc_test_init()) {
            while (1) os_time_dly(10);  // 进入测试模式
        }
    #endif

    // 10. 通用初始化回调
    __do_initcall(initcall);

    // 11. 模块初始化回调
    __do_initcall(module_initcall);

    // 12. 应用核心初始化
    app_core_init();
    /*
       - 初始化应用管理器
       - 注册所有应用 (通过REGISTER_APPLICATION注册的)
       - 初始化应用事件系统
    */

    // 13. 后期初始化回调
    __do_initcall(late_initcall);
    /*
       执行所有通过 late_initcall() 注册的函数:
       - c_main()
       - robot_init()
       - config_save_flash()
       - wireless_net_init()
       - sdk_meky_check()
    */

    // ====== 第四部分: 应用启动 ======

    // 14. 进入应用主函数
    app_main();  // 由具体应用实现
    /*
       不同的应用有不同实现:
       - wifi_story_machine: 启动音乐播放应用
       - wifi_camera: 启动摄像头应用
       - wifi_ipc: 启动IPC应用
       - demo_xxx: 启动对应Demo应用
    */

    // 15. Finsh Shell初始化 (如启用)
    #ifdef CONFIG_FINSH_ENABLE
        finsh_system_init();
    #endif

    // ====== 第五部分: 主循环 ======

    int res;
    int msg[32];

    // 16. 进入消息循环
    while (1) {
        // 等待任务消息
        res = os_task_pend("taskq", msg, ARRAY_SIZE(msg));
        
        if (res != OS_TASKQ) {
            continue;
        }
        
        // 处理应用核心消息
        app_core_msg_handler(msg);
        /*
           处理各种系统消息:
           - 按键事件 (SYS_KEY_EVENT)
           - 触摸事件 (SYS_TOUCH_EVENT)
           - 设备事件 (SYS_DEVICE_EVENT)
           - 网络事件 (SYS_NET_EVENT)
           - 蓝牙事件 (SYS_BT_EVENT)
        */
    }
}
```

---

### 🎮 阶段4: app_main() - 应用主函数

以**WiFi故事机**为例 (`apps/wifi_story_machine/app_main.c:207`):

```c
void app_main()
{
    struct intent it;

    puts("------------- wifi_story_machine app main-------------\n");

    // 1. 初始化意图结构
    init_intent(&it);
    
    // 2. 设置要启动的应用名称和动作
    it.name = "app_music";  // 应用名称
    it.action = ACTION_MUSIC_PLAY_MAIN;  // 启动动作

    // 3. 启动应用
    start_app(&it);
    /*
       - 查找注册的应用
       - 创建应用实例
       - 执行应用状态机 (CREATE -> START)
       - 初始化应用私有数据
       - 启动应用任务
    */

    // 4. 蓝牙模块初始化 (如启用且无WiFi)
    #if defined CONFIG_BT_ENABLE && !defined CONFIG_WIFI_ENABLE
        bt_ble_module_init();
    #endif
}
```

**其他应用示例**:

**WiFi Camera**:
```c
void app_main()
{
    init_intent(&it);
    it.name = "video_rec";
    it.action = ACTION_VIDEO_REC_MAIN;
    start_app(&it);
}
```

**Demo DevKitBoard**:
```c
void app_main()
{
    init_intent(&it);
    it.name = "app_demo";
    it.action = ACTION_DO_NOTHING;
    start_app(&it);
}
```

---

## 3️⃣ 关键数据结构

### 📦 应用程序结构

**位置**: `include_lib/system/app_core.h:42`

```c
struct application {
    u8 state;                           // 应用状态
    int action;                         // 启动动作
    char *data;                         // 数据指针
    const char *name;                   // 应用名称
    struct list_head entry;             // 链表节点
    void *private_data;                 // 私有数据
    const struct application_operation *ops;  // 操作集
};

struct application_operation {
    // 状态机处理函数
    int (*state_machine)(struct application *, enum app_state, struct intent *);
    // 事件处理函数
    int (*event_handler)(struct application *, struct sys_event *);
};
```

**应用状态枚举**:
```c
enum app_state {
    APP_STA_CREATE,     // 创建
    APP_STA_START,      // 启动
    APP_STA_PAUSE,      // 暂停
    APP_STA_RESUME,     // 恢复
    APP_STA_STOP,       // 停止
    APP_STA_DESTROY,    // 销毁
};
```

### 📝 Intent结构

```c
struct intent {
    const char *name;   // 目标应用名称
    int action;         // 要执行的动作
    const char *data;   // 数据指针
    u32 exdata;         // 扩展数据
};
```

---

## 4️⃣ 初始化机制

### 🔄 Initcall机制

**位置**: `include_lib/system/init.h`

SDK使用**initcall机制**实现模块化初始化，按优先级分为5个阶段：

```c
// 定义initcall类型
typedef int (*initcall_t)(void);

// 1. 早期初始化 (最先执行)
#define early_initcall(fn) \
    const initcall_t __initcall_##fn SEC_USED(.early.initcall) = fn

// 2. 平台初始化
#define platform_initcall(fn) \
    const initcall_t __initcall_##fn SEC_USED(.platform.initcall) = fn

// 3. 普通初始化
#define __initcall(fn) \
    const initcall_t __initcall_##fn SEC_USED(.initcall) = fn

// 4. 模块初始化
#define module_initcall(fn) \
    const initcall_t __initcall_##fn SEC_USED(.module.initcall) = fn

// 5. 后期初始化 (最后执行)
#define late_initcall(fn) \
    const initcall_t __initcall_##fn SEC_USED(.late.initcall) = fn

// 执行initcall宏
#define __do_initcall(prefix) \
    do { \
        initcall_t *init; \
        extern initcall_t prefix##_begin[], prefix##_end[]; \
        for (init = prefix##_begin; init < prefix##_end; init++) { \
            (*init)(); \
        } \
    } while(0)
```

#### 📋 Initcall执行顺序

**在 `app_task_handler()` 中的执行顺序**:

```
1. early_initcall    → 互斥锁、事件系统、线程池、文件系统
2. platform_initcall → 平台设备驱动
3. initcall          → 通用模块
4. module_initcall   → 功能模块
5. late_initcall     → 后期初始化（C++、机器人、配置、网络）
```

#### 🔍 实际注册的Initcall示例

**Early Initcall** (从map文件查看):
```
__initcall_mutex_init           // 互斥锁初始化
__initcall___sd_mutex_init      // SD卡互斥锁
__initcall_app_version_check    // 版本检查
__initcall_sys_event_init       // 系统事件初始化
__initcall_thread_fork_init     // 线程池初始化
__initcall_sdfile_init          // SD文件系统
__initcall_ntp_client_init      // NTP客户端
```

**Late Initcall**:
```
__initcall_c_main               // C++主函数
__initcall_robot_init           // 机器人初始化
__initcall_config_save_flash    // 配置保存
__initcall_wireless_net_init    // 无线网络
__initcall_sdk_meky_check       // SDK密钥检查
```

---

## 5️⃣ 应用程序启动

### 🎯 应用注册机制

**宏定义**: `REGISTER_APPLICATION(app_name)`

```c
// 定义在 include_lib/system/app_core.h:54
#define REGISTER_APPLICATION(at) \
    static struct application at SEC_USED(.app)
```

**使用示例** (从 `apps/demo/demo_DevKitBoard/app_main.c`):

```c
// 1. 定义应用操作集
static const struct application_operation app_demo_ops = {
    .state_machine  = app_demo_state_machine,   // 状态机函数
    .event_handler  = app_demo_event_handler,   // 事件处理函数
};

// 2. 注册应用
REGISTER_APPLICATION(app_demo) = {
    .name   = "app_demo",           // 应用名称
    .ops    = &app_demo_ops,        // 操作集
    .state  = APP_STA_DESTROY,      // 初始状态
};
```

### 🚀 应用启动流程

**函数**: `start_app(struct intent *it)`

```
1. 查找应用
   ├─ 遍历 .app 段中注册的所有应用
   └─ 匹配 intent.name 与应用名称

2. 切换应用
   ├─ 停止当前应用 (如有)
   │  ├─ 调用 state_machine(APP_STA_STOP)
   │  └─ 调用 state_machine(APP_STA_DESTROY)
   └─ 保存为prev_app (用于返回)

3. 启动新应用
   ├─ 调用 state_machine(APP_STA_CREATE, intent)
   │  └─ 应用初始化私有数据
   ├─ 调用 state_machine(APP_STA_START, intent)
   │  └─ 应用启动主要功能
   └─ 设置为current_app

4. 进入应用主循环
   └─ 处理应用事件 (通过event_handler)
```

---

## 6️⃣ 任务列表

SDK中预定义的任务配置 (从 `apps/wifi_story_machine/app_main.c:65`):

| 任务名称 | 优先级 | 栈大小 | 队列大小 | 功能描述 |
|---------|-------|--------|---------|----------|
| **app_core** | 15 | 2048 | 1024 | 应用核心任务 |
| **sys_event** | 29 | 512 | 0 | 系统事件处理 |
| **sys_timer** | 9 | 512 | 128 | 系统定时器 |
| **systimer** | 14 | 256 | 0 | 系统时钟 |
| **audio_server** | 16 | 512 | 64 | 音频服务器 |
| **audio_encoder** | 12 | 384 | 64 | 音频编码器 |
| **opus_encoder** | 13 | 1536 | 0 | Opus编码器 |
| **mp3_encoder** | 13 | 768 | 0 | MP3编码器 |
| **video_server** | 16 | 768 | 128 | 视频服务器 |
| **ui** | 21 | 768 | 256 | UI任务 |
| **tcpip_thread** | 16 | 800 | 0 | TCP/IP协议栈 |
| **tasklet** | 10 | 1400 | 0 | WiFi任务处理 |
| **RtmpMlmeTask** | 17 | 700 | 0 | WiFi MLME任务 |
| **RtmpCmdQTask** | 17 | 300 | 0 | WiFi命令队列 |
| **wl_rx_irq_thread** | 7 | 256 | 0 | WiFi接收中断 |
| **btctrler** | 19 | 512 | 384 | 蓝牙控制器 |
| **btstack** | 18 | 768 | 384 | 蓝牙协议栈 |

**优先级说明**:
- 数字越小，优先级越高
- 0-9: 高优先级 (中断、实时任务)
- 10-20: 中优先级 (业务逻辑)
- 21-30: 低优先级 (UI、事件处理)

---

## 7️⃣ 双核启动流程

AC79支持双核DSP，启动流程如下：

```
┌─────────────────┐
│   上电/复位      │
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│   CPU0 启动     │
│   执行 main()   │
└────────┬────────┘
         │
         ├─── setup_arch()
         ├─── os_init()
         └─── EnableOtherCpu()  // 启动CPU1
                  │
                  ▼
         ┌─────────────────┐
         │   CPU1 启动     │
         │   cpu1_main()   │
         └────────┬────────┘
                  │
                  ├─── interrupt_init()
                  ├─── debug_init()
                  └─── os_start()
```

**CPU1主函数** (`apps/common/system/init.c:99`):

```c
void cpu1_main(void)
{
    cpu1_run_flag = 1;  // 标记CPU1已启动
    
    __local_irq_disable();
    
    // 初始化中断
    interrupt_init();
    
    // 初始化调试
    debug_init();
    
    #if CPU_CORE_NUM > 1
        os_start();  // 启动RTOS (双核模式)
    #else
        puts("\r\n\n cpu1_run... \r\n\n");
    #endif
    
    __local_irq_enable();
    
    // 进入空闲循环
    while (1) {
        __asm__ volatile("idle");
    }
}
```

---

## 8️⃣ 启动流程图（完整版）

```
┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓
┃              AC79 SDK 完整启动流程                  ┃
┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛

 [硬件复位]
      │
      ▼
┌─────────────────────────────────────────────┐
│ Bootloader (Flash启动代码)                 │
│ - 从 0x2000120 加载代码                     │
│ - 初始化SDRAM (如使用)                      │
│ - 跳转到 _start                             │
└──────────────────┬──────────────────────────┘
                   │
                   ▼
┌──────────────────────────────────────────────┐
│ main() [apps/common/system/init.c:264]      │
├──────────────────────────────────────────────┤
│ 1. __local_irq_disable()                    │
│ 2. setup_arch()         ─────────┐          │
│ 3. EnableOtherCpu()               │          │
│ 4. os_init()                      │          │
│ 5. task_create(app_task_handler)  │          │
│ 6. os_start()                     │          │
│ 7. __local_irq_enable()           │          │
└───────────────────┬───────────────┘          │
                    │                           │
        ┌───────────┴─────────────┐            │
        │                         │            │
        ▼                         ▼            ▼
   [CPU0 调度]              [CPU1启动]   ┌──────────────┐
        │                  cpu1_main()   │ setup_arch() │
        │                         │      ├──────────────┤
        ▼                         ▼      │ • wdt_init() │
┌──────────────────┐      [CPU1空闲]    │ • clk_init() │
│ RTOS Scheduler   │                     │ • uart_init()│
│ - FreeRTOS       │                     │ • debug_init│
│ - 任务调度开始   │                     │ • 打印信息   │
└────────┬─────────┘                     └──────────────┘
         │
         ▼
┌──────────────────────────────────────────────┐
│ app_task_handler() [优先级15]               │
├──────────────────────────────────────────────┤
│ ▸ 系统初始化阶段                            │
│   ├─ sys_timer_init()                       │
│   ├─ sys_timer_task_init()                  │
│   └─ [可选] rtos_stack_check / mem_debug    │
├──────────────────────────────────────────────┤
│ ▸ 板级初始化阶段                            │
│   ├─ board_early_init()                     │
│   ├─ __do_initcall(early_initcall) ────┐    │
│   ├─ sdfile_ext_mount_init()           │    │
│   ├─ __do_initcall(platform_initcall)──┤    │
│   └─ board_init()                       │    │
├──────────────────────────────────────────────┤
│ ▸ 模块初始化阶段                            │
│   ├─ rf_fcc_test_init()                 │    │
│   ├─ __do_initcall(initcall) ───────────┤    │
│   ├─ __do_initcall(module_initcall) ────┤    │
│   ├─ app_core_init() ────────────────┐  │    │
│   └─ __do_initcall(late_initcall) ───┤  │    │
├──────────────────────────────────────────────┤
│ ▸ 应用启动阶段                              │
│   └─ app_main() ──────────────────┐     │    │
│                                    │     │    │
│ ▸ 主循环                           │     │    │
│   └─ while(1) {                    │     │    │
│       os_task_pend(msg)            │     │    │
│       app_core_msg_handler(msg)    │     │    │
│     }                              │     │    │
└────────────────────────────────────┘     │    │
         │                                 │    │
         │          ┌──────────────────────┘    │
         │          │                           │
         ▼          ▼                           ▼
   [消息循环]  ┌──────────────┐    ┌─────────────────────┐
         │     │  app_main()  │    │ Initcall Functions  │
         │     ├──────────────┤    ├─────────────────────┤
         │     │具体应用实现  │    │ early_initcall:     │
         ▼     │              │    │ • mutex_init        │
    ┌────────┐ │ 示例1:       │    │ • sys_event_init    │
    │事件分发│ │ wifi_story   │    │ • sdfile_init       │
    ├────────┤ │   init_intent│    │                     │
    │•按键   │ │   start_app  │    │ platform_initcall:  │
    │•触摸   │ │              │    │ • device_drivers    │
    │•设备   │ │ 示例2:       │    │                     │
    │•网络   │ │ wifi_camera  │    │ late_initcall:      │
    │•蓝牙   │ │   video_rec  │    │ • c_main            │
    └────────┘ │   start      │    │ • robot_init        │
               └──────┬───────┘    │ • wireless_net_init │
                      │            └─────────────────────┘
                      ▼
            ┌────────────────┐
            │  start_app()   │
            ├────────────────┤
            │ 1. 查找应用    │
            │ 2. 停止旧应用  │
            │ 3. 创建新应用  │
            │ 4. 启动新应用  │
            └────────┬───────┘
                     │
                     ▼
         ┌────────────────────────┐
         │ 应用状态机             │
         ├────────────────────────┤
         │ CREATE  → 初始化       │
         │ START   → 启动功能     │
         │ PAUSE   → 暂停         │
         │ RESUME  → 恢复         │
         │ STOP    → 停止         │
         │ DESTROY → 销毁         │
         └────────────────────────┘
                     │
                     ▼
         ┌────────────────────────┐
         │ 应用事件循环           │
         │ - event_handler()      │
         │ - 处理系统事件         │
         │ - 处理应用逻辑         │
         └────────────────────────┘
```

---

## 9️⃣ 时间线估算

启动各阶段的大致时间（以WiFi故事机为例）:

| 阶段 | 时间 | 说明 |
|-----|------|------|
| Bootloader | ~50ms | Flash读取和加载 |
| setup_arch() | ~100ms | 硬件初始化 |
| RTOS启动 | ~50ms | 任务创建和调度器启动 |
| early_initcall | ~20ms | 基础模块初始化 |
| platform_initcall | ~50ms | 平台设备初始化 |
| initcall + module | ~100ms | 功能模块初始化 |
| app_core_init | ~30ms | 应用核心初始化 |
| late_initcall | ~80ms | 后期初始化 |
| app_main() | ~200ms | 应用启动（含WiFi初始化） |
| **总计** | **~680ms** | 从上电到应用运行 |

---

## 🔟 关键配置宏

| 宏定义 | 说明 | 位置 |
|-------|------|------|
| `CONFIG_DEBUG_ENABLE` | 启用调试功能 | app_config.h |
| `CONFIG_WIFI_ENABLE` | 启用WiFi功能 | app_config.h |
| `CONFIG_BT_ENABLE` | 启用蓝牙功能 | app_config.h |
| `CONFIG_SFC_ENABLE` | 启用SPI Flash | app_config.h |
| `CPU_CORE_NUM` | CPU核心数量 (1或2) | app_config.h |
| `CONFIG_MMU_ENABLE` | 启用MMU | app_config.h |
| `CONFIG_UI_ENABLE` | 启用UI功能 | app_config.h |
| `RTOS_STACK_CHECK_ENABLE` | 启用栈检查 | app_config.h |

---

## 📚 参考文件列表

### 核心启动文件
- `apps/common/system/init.c` - 主启动文件
- `cpu/wl82/setup.c` - CPU架构初始化
- `cpu/wl82/sdk_ld_sfc.c` - SFC模式链接脚本
- `cpu/wl82/sdk_ld_sdram.c` - SDRAM模式链接脚本

### 应用示例
- `apps/wifi_story_machine/app_main.c` - WiFi故事机
- `apps/wifi_camera/app_main.c` - WiFi摄像头
- `apps/wifi_ipc/app_main.c` - WiFi IPC
- `apps/demo/demo_DevKitBoard/app_main.c` - Demo开发板

### 系统头文件
- `include_lib/system/init.h` - 初始化机制
- `include_lib/system/app_core.h` - 应用核心
- `include_lib/system/os/os_api.h` - OS API

---

## 📝 总结

AC79 SDK的启动流程设计精巧，采用**分层初始化**和**模块化**的架构：

### ✅ 关键特性
1. **双核支持**: CPU0和CPU1独立初始化和运行
2. **Initcall机制**: 5级初始化优先级，模块自动注册
3. **应用框架**: 统一的应用管理和状态机
4. **RTOS集成**: 基于FreeRTOS的多任务调度
5. **灵活配置**: 丰富的宏配置选项

### 🎯 启动流程要点
- `main()` → `setup_arch()` → `app_task_handler()` → `app_main()`
- 各阶段通过initcall机制注册和执行初始化函数
- 应用通过REGISTER_APPLICATION注册，start_app()启动
- 最终进入事件驱动的消息循环

---

**文档版本**: v1.0  
**最后更新**: 2025-10-31  
**适用SDK**: AC79NN_SDK_V1.2.0

