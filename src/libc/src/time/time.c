/**
 * @file time.c
 * @brief 
 * @author cbw (chenboven@qq.com)
 * @date 2025-10-27
 */

#include "time.h"
#include "c_time.h"
#include "string.h"
#include "stdio.h"
#include "stdarg.h"

/* 静态变量，用于存储静态分配的时间结构体 */
static struct tm static_tm;

/**
 * @brief 获取当前日历时间。
 * @param timer 指向存储时间的变量，如果为 NULL，则返回值直接表示当前时间。
 * @return 成功返回当前日历时间，失败返回 (time_t)(-1)。
 */
time_t time(time_t *timer)
{
    return plat_get_time_ns();
    uint64_t ns = plat_get_time_ns();
    if (ns == 0)
        return (time_t)(-1); // 如果平台时间不可用，返回错误

    time_t seconds = (time_t)(ns / 1000000000ull);
    if (timer)
        *timer = seconds;
    return seconds;
}

/**
 * @brief 初始化时间模块。
 */
int time_init(void)
{
    return plat_time_init();
}

uint64_t time_test(){
    return 0;
}

/**
 * @brief 将日历时间转换为本地时间。
 * @param timer 日历时间。
 * @return 返回指向静态分配的 tm 结构体指针，表示本地时间。
 */
struct tm *localtime(const time_t *timer)
{
    // 这里假设我们不支持时区和夏令时，简单地将时间分解为 tm 结构
    if (!timer)
        return NULL;

    time_t t = *timer;
    memset(&static_tm, 0, sizeof(struct tm));

    // 计算年份、月份、日期等
    static_tm.tm_sec = t % 60;
    t /= 60;
    static_tm.tm_min = t % 60;
    t /= 60;
    static_tm.tm_hour = t % 24;
    t /= 24;

    // 简化处理：从 1970 年开始计算日期
    int year = 1970;
    while (t >= (year % 4 == 0 ? 366 : 365))
    {
        t -= (year % 4 == 0 ? 366 : 365);
        year++;
    }
    static_tm.tm_year = year - 1900;

    // 处理月份和日期
    int month_days[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if ((year % 4 == 0) && (year % 100 != 0 || year % 400 == 0))
        month_days[1] = 29; // 闰年

    for (int month = 0; month < 12; ++month)
    {
        if (t < month_days[month])
        {
            static_tm.tm_mon = month;
            static_tm.tm_mday = (int)t + 1;
            break;
        }
        t -= month_days[month];
    }

    return &static_tm;
}

/**
 * @brief 将日历时间转换为 UTC 时间。
 * @param timer 日历时间。
 * @return 返回指向静态分配的 tm 结构体指针，表示 UTC 时间。
 */
struct tm *gmtime(const time_t *timer)
{
    // 没有时区支持，gmtime 和 localtime 是相同的
    return localtime(timer);
}

/**
 * @brief 将 tm 结构体表示的时间转换为日历时间。
 * @param timeptr 指向 tm 结构体的指针。
 * @return 成功返回对应的日历时间，失败返回 (time_t)(-1)。
 */
time_t mktime(struct tm *timeptr)
{
    if (!timeptr)
        return (time_t)(-1);

    int year = timeptr->tm_year + 1900;
    int month = timeptr->tm_mon;
    int day = timeptr->tm_mday;
    int hour = timeptr->tm_hour;
    int min = timeptr->tm_min;
    int sec = timeptr->tm_sec;

    // 计算从 1970 年到指定年份的天数
    int days = 0;
    for (int y = 1970; y < year; ++y)
        days += (y % 4 == 0 ? 366 : 365);

    // 添加当年的天数
    int month_days[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if ((year % 4 == 0) && (year % 100 != 0 || year % 400 == 0))
        month_days[1] = 29; // 闰年

    for (int m = 0; m < month; ++m)
        days += month_days[m];
    days += (day - 1);

    // 转换为秒
    time_t seconds = (days * 24 * 60 * 60) + (hour * 60 * 60) + (min * 60) + sec;
    return seconds;
}

/**
 * @brief 格式化时间输出。
 * @param buffer 存储格式化字符串的缓冲区。
 * @param maxsize 缓冲区大小。
 * @param format 格式化字符串。
 * @param timeptr 指向 tm 结构体的指针。
 * @return 成功返回 buffer 的地址，失败返回 NULL。
 */
char *strftime(char *buffer, size_t maxsize, const char *format, const struct tm *timeptr)
{
    if (!buffer || !format || !timeptr || maxsize == 0)
        return NULL;

    size_t idx = 0; // 当前写入位置
    const char *fmt = format;

    while (*fmt && idx < maxsize - 1) // 留出空间给终止符 '\0'
    {
        if (*fmt != '%') // 非格式化字符直接复制
        {
            buffer[idx++] = *fmt++;
            continue;
        }

        fmt++; // 跳过 '%'

        // 处理格式化字符
        switch (*fmt)
        {
        case 'Y': // 年份 (4 位)
            idx += _vsnprintf(buffer + idx, maxsize - idx, "%04d", timeptr->tm_year + 1900);
            break;
        case 'm': // 月份 (2 位)
            idx += _vsnprintf(buffer + idx, maxsize - idx, "%02d", timeptr->tm_mon + 1);
            break;
        case 'd': // 日期 (2 位)
            idx += _vsnprintf(buffer + idx, maxsize - idx, "%02d", timeptr->tm_mday);
            break;
        case 'H': // 小时 (24 小时制, 2 位)
            idx += _vsnprintf(buffer + idx, maxsize - idx, "%02d", timeptr->tm_hour);
            break;
        case 'M': // 分钟 (2 位)
            idx += _vsnprintf(buffer + idx, maxsize - idx, "%02d", timeptr->tm_min);
            break;
        case 'S': // 秒 (2 位)
            idx += _vsnprintf(buffer + idx, maxsize - idx, "%02d", timeptr->tm_sec);
            break;
        default: // 未知格式符，直接复制 '%' 和当前字符
            buffer[idx++] = '%';
            if (*fmt)
                buffer[idx++] = *fmt;
            break;
        }

        fmt++; // 处理下一个字符
    }

    buffer[idx] = '\0'; // 添加字符串终止符
    return (idx > 0 && idx < maxsize) ? buffer : NULL;
}

/**
 * @brief 获取处理器时间。
 * @return 返回处理器时间，单位为 CLOCKS_PER_SEC。
 */
clock_t clock(void)
{
    uint64_t ns = plat_get_time_ns();
    if (ns == 0)
        return (clock_t)(-1); // 如果平台时间不可用，返回错误

    return (clock_t)(ns / (1000000000ull / CLOCKS_PER_SEC));
}