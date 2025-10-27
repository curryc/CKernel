/**
 * @file time.h
 * @brief 
 * @author cbw (chenboven@qq.com)
 * @date 2025-10-27
 */

#ifndef CKERNEL_TIME_H
#define CKERNEL_TIME_H

#include "stddef.h" // 用于 size_t
#include "stdint.h"

#ifdef __cplusplus
extern "C" {
#endif
/* 定义时间相关的类型 */
typedef long time_t;          // 表示日历时间（通常是自 UNIX 时间戳以来的秒数）
typedef long clock_t;         // 表示处理器时间

/* 定义时区结构体 */
struct tm {
    int tm_sec;   // 秒 (0-60)
    int tm_min;   // 分钟 (0-59)
    int tm_hour;  // 小时 (0-23)
    int tm_mday;  // 月份中的日期 (1-31)
    int tm_mon;   // 月份 (0-11, 0 表示一月)
    int tm_year;  // 年份 (自 1900 年起的年数)
    int tm_wday;  // 星期几 (0-6, 0 表示星期日)
    int tm_yday;  // 一年中的第几天 (0-365)
    int tm_isdst; // 夏令时标志 (-1: 不确定, 0: 不使用夏令时, >0: 使用夏令时)
};

/* 定义 clock() 函数的时间单位常量 */
#define CLOCKS_PER_SEC 1000000L // 每秒的时钟周期数

/* 主要函数声明 */

/**
 * @brief 获取当前日历时间。
 * @param timer 指向存储时间的变量，如果为 NULL，则返回值直接表示当前时间。
 * @return 成功返回当前日历时间，失败返回 (time_t)(-1)。
 */
time_t time(time_t *timer);

/**
 * @brief 初始化时间模块。
 */
int time_init(void);

uint64_t time_test();

/**
 * @brief 将日历时间转换为本地时间。
 * @param timer 日历时间。
 * @return 返回指向静态分配的 tm 结构体指针，表示本地时间。
 */
struct tm *localtime(const time_t *timer);

/**
 * @brief 将日历时间转换为 UTC 时间。
 * @param timer 日历时间。
 * @return 返回指向静态分配的 tm 结构体指针，表示 UTC 时间。
 */
struct tm *gmtime(const time_t *timer);

/**
 * @brief 将 tm 结构体表示的时间转换为日历时间。
 * @param timeptr 指向 tm 结构体的指针。
 * @return 成功返回对应的日历时间，失败返回 (time_t)(-1)。
 */
time_t mktime(struct tm *timeptr);

/**
 * @brief 格式化时间输出。
 * @param buffer 存储格式化字符串的缓冲区。
 * @param format 格式化字符串。
 * @param timeptr 指向 tm 结构体的指针。
 * @return 成功返回 buffer 的地址，失败返回 NULL。
 */
char *strftime(char *buffer, size_t maxsize, const char *format, const struct tm *timeptr);

/**
 * @brief 获取处理器时间。
 * @return 返回处理器时间，单位为 CLOCKS_PER_SEC。
 */
clock_t clock(void);

/**
 * @brief 将时间字符串解析为 tm 结构体。
 * @param s 时间字符串。
 * @param format 格式化字符串。
 * @param timeptr 指向 tm 结构体的指针。
 * @return 成功返回指向第一个未解析字符的指针，失败返回 NULL。
 */
// char *strptime(const char *s, const char *format, struct tm *timeptr);

#ifdef __cplusplus
}
#endif

#endif /* _TIME_H */