# VGA
VGA 是 Video Graphics Array 的缩写，是一种早期的视频显示标准，最早由 IBM 在 1987 年提出。它定义了分辨率、颜色深度、刷新率等显示规范，也规定了显存的布局和访问方式。
为了能在内核的早期阶段在屏幕上输出信息，需要一个简单的VGA文本输出驱动，能在不需要其他任何硬件的情况下，将字符信息输出到屏幕上。
VGA的属性约定：
1. 文本模式（Text Mode）：80×25 字符，每个字符占 2 字节（1 字节 ASCII + 1 字节颜色属性）
2. 显存地址：固定在物理地址 0xB8000
3. 颜色属性字节：高 4 位是背景色，低 4 位是前景色（文字颜色）

了解了这些属性，就可以开始编写VGA文本输出驱动了，用最基础的C语言即可实现这一功能。

#  VGA文本输出
**基本常量定义**
```
enum vga_color {
    VGA_BLACK = 0,
    VGA_GREEN = 2,
    VGA_RED   = 4,
    VGA_WHITE = 7,
};

static uint16_t *const VGA_MEM = (uint16_t *)0xB8000;
static const int VGA_WIDTH = 80;
static const int VGA_HEIGHT = 25;
```
**字符和颜色的打包函数**

通过字符和颜色生成一个16位字符，通过位运算打包，VGA通过此16位字符写入显存，在屏幕上显示字符。
```
static inline uint16_t vga_entry(char c, enum vga_color fg)
{
    uint8_t color = fg | (VGA_BLACK << 4);
    return (uint16_t)c | (uint16_t)color << 8;
}
```
**输出一个字符**

查看是否需要环行，如果横向字符满了，也需要换行。如果纵向满了，则需要滚动屏幕。
```
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
```

**格式化输出字符串**

通过将字符串中的字符和后续的参数进行解码，形成标准字符串。
通过获取函数的形参列表，遍历第一个形参字符串中的字符，如果没有遇到占位符，即直接输出此字符，否则需要通过后面的形参进行格式化。
```
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

        // 格式化
    }
}
```

对于格式化的实现，参考原c语言中printf的实现，例如：
```
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
            }
        }
```
如果访问到了%，开始判断后续的字符来确定格式。比如这里的"%ld"，取后面的形参，将此参数按照规则转化为字符串即可。

**滚动屏幕**

将所有的字符向上移动一行，即所有的字符向前移动VGA_WIDTH个字符，然后将最后一行清空。
```
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
```

**串口输出**

将字符输出到串口方便调试，这样GDB调试的时候将会输出到控制台。
```
#include "port.h"
static void serial_putc(char c)
{
    while (!(port_inb(0x3F8 + 5) & 0x20))
        ; /* 等待 THR 空 */
    port_outb(0x3F8, c);
    if (c == '\n')
        serial_putc('\r'); /* 回车换行 */
}
```
串口（Serial Port）是一种 串行通信接口，早期用于鼠标、调制解调器、调试输出等。
通过IO端口与CPU进行通信，串口有自己的基地址，通过特定的汇编指令即可进行输入输出。
本项目通过C语言将汇编语言进行包装，比如这里的port_inb:
```
uint8_t port_inb(const uint32_t _port) {
    uint8_t __val;
    __asm__ volatile ("inb %w1, %b0"
                    : "=a"(__val)
                    : "Nd"(_port));
    return __val;
}
```

