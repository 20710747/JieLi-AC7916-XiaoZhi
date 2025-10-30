
#ifndef __ROBOT_DEBUG_H__
#define __ROBOT_DEBUG_H__

/* --------------- 底层统一出口 --------------- */


void dbg_printf_info(const char *tag, const char *fmt, ...);

#define __DBG_OUT(tag, fmt, ...)                                                \
    do {                                                                        \
        /*extern void dbg_printf_info(const char *, const char *, ...);*/          \
        dbg_printf_info(tag, fmt, ##__VA_ARGS__);                              \
    } while (0)


/* --------------- 原宏兼容 --------------- */
#define DBG_PRINT(T, D)                     __DBG_OUT(T, "%d", (int)(D))
#define UDBG_PRINT(T, D)                    __DBG_OUT(T, "%u", (unsigned)(D))
#define DBG2_PRINT(T, D1, D2)               __DBG_OUT(T, "%d %d", (int)(D1), (int)(D2))
#define DBGS_PRINT(T, D, S)                 robot_main_debugs_output(T, D, S)   /* 数组特殊处理 */
#define HEX_PRINT(T, H)                     __DBG_OUT(T, "0x%02X", (unsigned)(H))
#define HEXS_PRINT(T, H, S)                 robot_main_hexs_output(T, H, S)   /* 数组特殊处理 */
#define HEXA_PRINT(T, H)                    robot_main_hexs_output_without_len(T, H)
#define STR_PRINT(S)                        robot_main_str_output(S, 0)
#define DATA_PRINT(D, S)                    robot_main_data_output(D, S)      /* 若存在 */
#define STRDBG_PRINT(T, S)                  __DBG_OUT(T, "%s", S)
#define DBG_PRINT_UINT32(TAG, DATA)         __DBG_OUT(TAG, "%u", (unsigned)(DATA))

/* 为了风格统一，再封装 3 个常用级别 */


#define DEBUG_LEVEL_NONE    0x00
#define DEBUG_LEVEL_ERR     0x01
#define DEBUG_LEVEL_WARN    0x02
#define DEBUG_LEVEL_INFO    0x03
#define DEBUG_LEVEL_DEBUG   0x04
#define DEBUG_LEVEL_VERBOSE 0x05

#define ROBOT_DEBUG_LEVEL DEBUG_LEVEL_DEBUG

#if ROBOT_DEBUG_LEVEL >= DEBUG_LEVEL_ERR
#define ERR_PRINTF(fmt, ...)   dbg_printf_info("[ERR]",fmt, ## __VA_ARGS__)
#else
#define ERR_PRINTF(...) ((void)0)
#endif

#if ROBOT_DEBUG_LEVEL >= DEBUG_LEVEL_WARN
#define WARN_PRINTF(fmt, ...)   dbg_printf_info("[WARN]",fmt, ## __VA_ARGS__)
#else
#define WARN_PRINTF(...) ((void)0)
#endif

#if ROBOT_DEBUG_LEVEL >= DEBUG_LEVEL_INFO
#define INFO_PRINTF(fmt, ...)   dbg_printf_info("[INFO]",fmt, ## __VA_ARGS__)
#else
#define INFO_PRINTF(...) ((void)0)
#endif


#if ROBOT_DEBUG_LEVEL >= DEBUG_LEVEL_DEBUG
#define DBG_PRINTF(fmt, ...)   dbg_printf_info("[DEBUG]",fmt, ## __VA_ARGS__)
#else
#define DBG_PRINTF(...) ((void)0)
#endif



#if ROBOT_DEBUG_LEVEL >= DEBUG_LEVEL_VERBOSE
#define VERBOSE_PRINTF(fmt, ...) dbg_printf_info("[VERBOSE]",fmt, ## __VA_ARGS__)
#else
#define VERBOSE_PRINTF(...) ((void)0)
#endif

#endif // __ROBOT_DEBUG_H__
