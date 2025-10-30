#include "robot_main.h"
#include "robot_debug.h"
#include "robot_adapter.h"
#include "robot_uart.h"

/***********************************************************************/
/* --------------- ���� --------------- */
#define DBG_BUF_SZ      256
#define DBG_TIME_EN     1          /* 1=��ϵͳ�δ�ʱ��� */

/* --------------- �ڲ����� --------------- */
static char uart_debug_string[DBG_BUF_SZ];

/* --------------- �ײ���� --------------- */
static void dbg_out(const char *s)
{
#if CONFIG_DEBUG_CHANEL == 0
    printf("TBZ LOG : %s", s);
#else
    MAIN_WRITE(s,strlen(s));
#endif
}

/* --------------- ͨ�ø�ʽ�� API --------------- */
/* �ײ���� */
extern void dbg_out(const char *s);

/* ���޷���������ָ������ת�ַ��� */
static char *utoa(unsigned int val, char *p, int base, int min_width)
{
    char tmp[12];               /* 32 λ��� 10 λ + ���� + '\0' */
    char *t = tmp + sizeof(tmp);
    char *start = p;  // ������ʼλ��

    *--t = '\0';
    do
    {
        *--t = "0123456789ABCDEF"[val % base];
        val /= base;
    }
    while (val);

    int len = strlen(t);
    int zeros_to_add = (min_width > len) ? (min_width - len) : 0;

    // ���ǰ����
    for (int i = 0; i < zeros_to_add; i++) {
        *p++ = '0';
    }

    // �������ֲ���
    strcpy(p, t);

    // ���ؽ���λ�ã���ʼλ�� + ǰ�������� + ���ֳ���
    return start + zeros_to_add + len;
}

/* ��������ָ������ת�ַ��� */
static char *itoa(int val, char *p, int base)
{
    if (val < 0 && base == 10)
    {
        *p++ = '-';
        val = -val;
    }
    return utoa((unsigned)val, p, base, 0);
}

/* �� double ת�ɹ̶� 4 λС���ַ��������� sprintf %f */
static char *ftoa(double val, char *p)
{
    if (val < 0)
    {
        *p++ = '-';
        val = -val;
    }
    int intg = (int)val;
    int frac = (int)((val - intg) * 10000 + 0.5);
    if (frac >= 10000)
    {
        intg++;
        frac -= 10000;
    }
    p = utoa(intg, p, 10, 0);
    *p++ = '.';
    /* �� 0 */
    if (frac < 1000) *p++ = '0';
    if (frac < 100)  *p++ = '0';
    if (frac < 10)   *p++ = '0';
    p = utoa(frac, p, 10, 0);
    return p;
}

void dbg_printf_raw(const char *fmt, ...)
{
    char *b = uart_debug_string;

    va_list ap;
    va_start(ap, fmt);
    while (*fmt)
    {
        if (*fmt != '%')
        {
            *b++ = *fmt++;
            continue;
        }
        ++fmt;
        switch (*fmt++)
        {
        case 'd':
            b = itoa(va_arg(ap, int), b, 10);
            break;
        case 'u':
            b = utoa(va_arg(ap, unsigned), b, 10,0);
            break;
        case 'x':
            b = utoa(va_arg(ap, unsigned), b, 16,2);//����16�������֣�������2������
            break;
        case 'f':
            b = ftoa(va_arg(ap, double), b);
            break;
        case 's':
        {
            const char *s = va_arg(ap, const char *);
            while (*s) *b++ = *s++;
            break;
        }
        case 'c':
            *b++ = (char)va_arg(ap, int);
            break;
        default:
            *b++ = '?';
            break;
        }
    }
    va_end(ap);
    /* ���Զ��� \r\n����ȫ�ɵ����߾��� */
    *b = '\0';
    dbg_out(uart_debug_string);
}

void dbg_printf_info(const char *tag, const char *fmt, ...)
{
    char *b = uart_debug_string;

    //*b++ = '[';
    b = strcpy(b, tag);
    b += strlen(tag);
    //*b++ = ']';
    *b++ = ' ';

    va_list ap;
    va_start(ap, fmt);
    while (*fmt)
    {
        if (*fmt != '%')
        {
            *b++ = *fmt++;
            continue;
        }
        /* ���� '%' */
        ++fmt;
        switch (*fmt++)
        {
        case 'd':
            b = itoa(va_arg(ap, int), b, 10);
            break;
        case 'u':
            b = utoa(va_arg(ap, unsigned), b, 10, 0);
            break;
        case 'x':
            b = utoa(va_arg(ap, unsigned), b, 16, 0);
            break;
        case 'f':
            b = ftoa(va_arg(ap, double), b);
            break;
        case 's':
        {
            const char *s = va_arg(ap, const char *);
            while (*s) *b++ = *s++;
            break;
        }
        case 'c':
            *b++ = (char)va_arg(ap, int);
            break;
        default:
            *b++ = '?';
            break;
        }
    }
    va_end(ap);

    *b++ = ' ';
    *b++ = '#';
    *b++ = '\r';
    *b++ = '\n';
    *b   = '\0';
    dbg_out(uart_debug_string);
}


/* --------------- �ɽӿ�͸�� --------------- */
void robot_main_str_output(char *s, uint16_t sz)
{
    if (!s) return;
    if (!sz) sz = strlen(s);
    dbg_out(s);
}

/* ���оɺ���ֱ�ӵ���ͨ�� API�������� */
void robot_main_debug_output(char *tag, int16_t d)
{
    //char fmt[32];
    //sprintf(fmt,"%s %%d",tag);
    //DBG_PRINT
    DBG_PRINT(tag, d);
}

void robot_main_debug_output_uint32(char *tag, uint32_t d)
{
    char fmt[32];
    sprintf(fmt,"%s %%u",tag);
    DBG_PRINTF(fmt, d);
}

void robot_main_udebug_output(char *tag, uint16_t d)
{
    char fmt[64];
    sprintf(fmt,"%s %%u",tag);
    DBG_PRINTF(fmt, d);
}

void robot_main_debugu32_output(char *tag, uint32_t d)
{
    char fmt[64];
    sprintf(fmt,"%s %%d(0x%%x)",tag);
    DBG_PRINTF(fmt, d, d);
}

void robot_main_debug32_output(char *tag, int32_t d)
{
    char fmt[64];
    sprintf(fmt,"%s %%d(0x%%x)",tag);
    DBG_PRINTF(fmt, d, d);
}

/*
void robot_main_debug_output_str(char *tag, char *s)
{
    DBG_PRINTF(tag, "%s", s);
}
*/
void robot_main_debug_string_output(char *tag, char *s)
{
    char fmt[64];
    sprintf(fmt,"%s %%s",tag);
    DBG_PRINTF(fmt, s);
}

void robot_main_debugf64_output(char *tag, double v)
{
    #if 0
    int i = (int)v;
    int f = (int)((v < 0 ? -v : v) * 10000) % 10000;
    DBG_PRINTF(tag, "%d.%04d", i, f);
    #endif
}

void robot_main_debug2_output(char *tag, int16_t d1, int16_t d2)
{
    char fmt[32];
    sprintf(fmt,"%s %%d %%d",tag);
    DBG_PRINTF(tag, fmt, d1, d2);
}

void robot_main_hex_output(char *tag, uint8_t hex)
{
    char fmt[32];
    sprintf(fmt,"%s 0x%%x",tag);
    DBG_PRINTF(fmt, hex);
}

/* ����������þ�ʵ�֣��ڲ��Ե� dbg_printf_raw */
void robot_main_debugs_output(char *tag, int16_t *d, uint16_t sz)
{
    dbg_printf_raw("%s :\r\n", tag);
    for (int i = 0; i < sz; ++i)
        dbg_printf_raw("  %4d", d[i]);
    dbg_printf_raw("  #\r\n");
}

static void _hex_dump(const char *tag, const uint8_t *d, uint16_t sz, int hdr)
{
    if (tag && strcasecmp(tag, "null"))
        hdr ? dbg_printf_raw("%s(%d) :", tag, sz)
        : dbg_printf_raw("%s :", tag);
    if(sz>=8)
        dbg_printf_raw("\r\n");
    for (int i = 0; i < sz; ++i)
    {
        dbg_printf_raw("%x ", d[i]);
        if ((i + 1) % 24 == 0) dbg_printf_raw("#\r\n");
    }
    if (sz % 24) dbg_printf_raw("#\r\n");
}

void robot_main_hexs_output(char *tag, uint8_t *d, uint16_t sz)
{
    _hex_dump(tag, d, sz, 1);
}

void robot_main_hexs_output_without_len(char *tag, uint8_t *d)
{
    _hex_dump(tag, d, strlen((char *)d), 0);
}

/***********************************************************************/
