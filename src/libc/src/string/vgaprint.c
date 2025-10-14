/**
 * @file vgaprint.c
 * @brief 简单的vga输出，主要用于调试
 * @author cbw (chenboven@qq.com)
 * @date 2025-10-14
 */
#include "stddef.h"
#include "stdint.h"
#include "vgaprint.h"

static uint16_t *const VGA_MEM = (uint16_t *)0xB8000;
static const int VGA_WIDTH  = 80;
static const int VGA_HEIGHT = 25;

static int cursor_x = 0;
static int cursor_y = 0;
static enum vga_color current_color = VGA_WHITE;

static inline uint16_t vga_entry(char c, enum vga_color fg)
{
    uint8_t color = fg | (VGA_BLACK << 4);
    return (uint16_t)c | (uint16_t)color << 8;
}

static void vga_scroll(void)
{
    /* 向上滚一行 */
    for (int y = 0; y < VGA_HEIGHT - 1; ++y)
        for (int x = 0; x < VGA_WIDTH; ++x)
            VGA_MEM[y * VGA_WIDTH + x] = VGA_MEM[(y + 1) * VGA_WIDTH + x];
    /* 清空最后一行 */
    for (int x = 0; x < VGA_WIDTH; ++x)
        VGA_MEM[(VGA_HEIGHT - 1) * VGA_WIDTH + x] = vga_entry(' ', current_color);
    cursor_y = VGA_HEIGHT - 1;
    cursor_x = 0;
}

static void vga_putchar(char c)
{
    if (c == '\n') {
        cursor_x = 0;
        ++cursor_y;
    } else {
        VGA_MEM[cursor_y * VGA_WIDTH + cursor_x] = vga_entry(c, current_color);
        if (++cursor_x == VGA_WIDTH) {
            cursor_x = 0;
            ++cursor_y;
        }
    }
    if (cursor_y >= VGA_HEIGHT)
        vga_scroll();
}

void vga_puts(const char *s)
{
    while (*s)
        vga_putchar(*s++);
}

void vga_init(void)
{
    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; ++i)
        VGA_MEM[i] = vga_entry(' ', current_color);
    cursor_x = cursor_y = 0;
}

void vga_setcolor(enum vga_color fg)
{
    current_color = fg;
}

#include "port.h"
static void serial_putc(char c)
{
    while (!(port_inb(0x3F8 + 5) & 0x20));   /* 等待 THR 空 */
    port_outb(0x3F8, c);
    if (c == '\n') serial_putc('\r');   /* 回车换行 */
}


/* 极简 printf，只支持 %d %u %x %s %c */
void vga_printf(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);

    char buf[32];
    while (*fmt) {
        if (*fmt != '%') {
            vga_putchar(*fmt++);
            continue;
        }
        ++fmt;
        switch (*fmt++) {
        case 'd': {
            int v = va_arg(ap, int);
            int neg = v < 0;
            if (neg) v = -v;
            char *p = buf + sizeof(buf) - 1;
            *p = '\0';
            do { *--p = '0' + (v % 10); v /= 10; } while (v);
            if (neg) *--p = '-';
            vga_puts(p);
            break;
        }
        case 'u': {
            unsigned int v = va_arg(ap, unsigned int);
            char *p = buf + sizeof(buf) - 1;
            *p = '\0';
            do { *--p = '0' + (v % 10); v /= 10; } while (v);
            vga_puts(p);
            break;
        }
        case 'x': {
            unsigned int v = va_arg(ap, unsigned int);
            char *p = buf + sizeof(buf) - 1;
            *p = '\0';
            do { *--p = "0123456789abcdef"[v & 0xf]; v >>= 4; } while (v);
            vga_puts(p);
            break;
        }
        case 's':
            vga_puts(va_arg(ap, char *));
            break;
        case 'c':
            vga_putchar(va_arg(ap, int));
            break;
        default:
            vga_putchar('%');
            vga_putchar(*(fmt - 1));
        }
    }
    va_end(ap);
    for (char *p = buf; *p; ++p) serial_putc(*p);
}
