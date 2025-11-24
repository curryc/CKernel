/**
 * @file vgaprint.c
 * @brief 简单的vga输出，主要用于调试
 * @author cbw (chenboven@qq.com)
 * @date 2025-10-14
 */
#include "stddef.h"
#include "stdint.h"
#include "io.h"
#include "vgaprint.h"
#include "vmm.h"

#define VGA_VIRT  0xFFFFFFFF000B8000
#define VGA_PHYS  0x000B8000


static uint16_t* VGA_MEM = (uint16_t *)0xB8000;
static const int VGA_WIDTH = 80;
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
    if (c == '\n')
    {
        cursor_x = 0;
        ++cursor_y;
    }
    else
    {
        VGA_MEM[cursor_y * VGA_WIDTH + cursor_x] = vga_entry(c, current_color);
        if (++cursor_x == VGA_WIDTH)
        {
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

void vga_init(bool virt = false)
{
    if (virt)
    {
        VMM::get_instance().mmap(
            VMM::get_instance().get_pgd(),
            (uintptr_t)VGA_VIRT,
            (uintptr_t)VGA_PHYS,
            VMM::VMM_PAGE_VALID | VMM::VMM_PAGE_WRITABLE);
        VGA_MEM = (uint16_t*)VGA_VIRT;
    }
    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; ++i)
        VGA_MEM[i] = vga_entry(' ', current_color);
    cursor_x = cursor_y = 0;
}

void vga_setcolor(enum vga_color fg)
{
    current_color = fg;
}
void serial_putc(char c)
{
    while (!(IO::get_instance().inb(0x3F8 + 5) & 0x20))
        ; /* 等待 THR 空 */
    IO::get_instance().outb(0x3F8, c);
    if (c == '\n')
        serial_putc('\r'); /* 回车换行 */
}


void vga_printf(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);

    char buf[32];
    while (*fmt)
    {
        if (*fmt != '%')
        {
            vga_putchar(*fmt++);
            serial_putc(*(fmt - 1)); // 同步输出到串口
            continue;
        }
        ++fmt;
        switch (*fmt++)
        {
        case 'd':
        {
            int v = va_arg(ap, int);
            int neg = v < 0;
            if (neg)
                v = -v;
            char *p = buf + sizeof(buf) - 1;
            *p = '\0';
            do
            {
                *--p = '0' + (v % 10);
                v /= 10;
            } while (v);
            if (neg)
                *--p = '-';
            vga_puts(p);
            // 输出到串口
            for (char *q = p; *q; ++q)
                serial_putc(*q);
            break;
        }
        case 'u':
        {
            unsigned int v = va_arg(ap, unsigned int);
            char *p = buf + sizeof(buf) - 1;
            *p = '\0';
            do
            {
                *--p = '0' + (v % 10);
                v /= 10;
            } while (v);
            vga_puts(p);
            // 输出到串口
            for (char *q = p; *q; ++q)
                serial_putc(*q);
            break;
        }
        case 'x':
        {
            unsigned int v = va_arg(ap, unsigned int);
            char *p = buf + sizeof(buf) - 1;
            *p = '\0';
            do
            {
                *--p = "0123456789abcdef"[v & 0xf];
                v >>= 4;
            } while (v);
            vga_puts(p);
            // 输出到串口
            for (char *q = p; *q; ++q)
                serial_putc(*q);
            break;
        }
        case 'p':
        {
            // 处理指针地址 (%p)
            unsigned long v = (unsigned long)va_arg(ap, void *);
            char *p = buf + sizeof(buf) - 1;
            *p = '\0';
            // 地址通常显示为8位十六进制数
            for (int i = 0; i < 8; ++i)
            {
                *--p = "0123456789abcdef"[v & 0xf];
                v >>= 4;
            }
            vga_puts("0x");
            vga_puts(p);
            // 输出到串口
            serial_putc('0');
            serial_putc('x');
            for (char *q = p; *q; ++q)
                serial_putc(*q);
            break;
        }
        case 'l':
        {
            // 处理长整型 (l + d/u/x)
            switch (*fmt++)
            {
            case 'd':
            {
                long v = va_arg(ap, long);
                int neg = v < 0;
                if (neg)
                    v = -v;
                char *p = buf + sizeof(buf) - 1;
                *p = '\0';
                do
                {
                    *--p = '0' + (v % 10);
                    v /= 10;
                } while (v);
                if (neg)
                    *--p = '-';
                vga_puts(p);
                // 输出到串口
                for (char *q = p; *q; ++q)
                    serial_putc(*q);
                break;
            }
            case 'u':
            {
                unsigned long v = va_arg(ap, unsigned long);
                char *p = buf + sizeof(buf) - 1;
                *p = '\0';
                do
                {
                    *--p = '0' + (v % 10);
                    v /= 10;
                } while (v);
                vga_puts(p);
                // 输出到串口
                for (char *q = p; *q; ++q)
                    serial_putc(*q);
                break;
            }
            case 'x':
            {
                unsigned long v = va_arg(ap, unsigned long);
                char *p = buf + sizeof(buf) - 1;
                *p = '\0';
                do
                {
                    *--p = "0123456789abcdef"[v & 0xf];
                    v >>= 4;
                } while (v);
                vga_puts(p);
                // 输出到串口
                for (char *q = p; *q; ++q)
                    serial_putc(*q);
                break;
            }
            }
            break;
        }
        case 'c':
        {
            int c = va_arg(ap, int);
            vga_putchar(c);
            serial_putc(c);
            break;
        }
        case 's':
        {
            char *str = va_arg(ap, char *);
            vga_puts(str);
            for (char *p = str; *p; ++p)
                serial_putc(*p);
            break;
        }
        default:
            vga_putchar('%');
            vga_putchar(*(fmt - 1));
            serial_putc('%');
            serial_putc(*(fmt - 1));
        }
    }
    va_end(ap);
}
