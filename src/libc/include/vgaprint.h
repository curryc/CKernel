/**
 * @file vgaprint.h
 * @brief 定义用于屏幕打印的函数
 * @author cbw (chenboven@qq.com)
 * @date 2025-10-14
 */
#ifndef VGAPRINT_H
#define VGAPRINT_H

#include <stdarg.h>
#include <stddef.h>

/* 颜色可选 */
enum vga_color {
    VGA_BLACK = 0,
    VGA_GREEN = 2,
    VGA_RED   = 4,
    VGA_WHITE = 7,
};

#ifdef __cplusplus
extern "C" {
#endif
void vga_init(void);                    /* 第一次调用清屏 */
void vga_setcolor(enum vga_color fg);   /* 默认 VGA_WHITE */
void vga_puts(const char *s);           /* 原始字符串 */
void vga_printf(const char *fmt, ...);  /* 格式化打印 */


#ifdef __cplusplus
}
#endif
#endif