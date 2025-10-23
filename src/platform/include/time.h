/**
 * @file time.h
 * @brief 时间工具
 * @author cbw (chenboven@qq.com)
 * @date 2025-10-23
 */

#ifndef CKERNEL_TIME_H__
#define CKERNEL_TIME_H__

#ifdef __cplusplus
extern "C" {
#endif
struct tm  {
    int tm_sec;
    int tm_min;
    int tm_hour;
    int tm_mday;
    int tm_mon;
    int tm_year;
    int tm_wday;
    int tm_yday;
    int tm_isdst;
};

#ifndef  __CLOCK_T
#define  __CLOCK_T
typedef long clock_t;
#define CLK_TCK 18.2
#endif


#ifndef  __TIME_T
#define  __TIME_T
typedef long  time_t;
#endif

void time_init(void);
time_t time(time_t *_timer);
clock_t clock(void);
struct tm *gmtime(const time_t *_timer);
struct tm *localtime(const time_t *_timer);


#ifdef __cplusplus
}
#endif

#endif