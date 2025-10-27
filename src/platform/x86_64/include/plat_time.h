/**
 * @file time.h
 * @brief 
 * @author cbw (chenboven@qq.com)
 * @date 2025-10-24
 */

 #ifndef CKERNEL_TIME_H
 #define CKERNEL_TIME_H

#include "stdint.h"

/* 宏定义 */
#define PIT_CONST       1193   /* PIT 每毫秒计数值 */
#define DELAY_MS        10      /* 校准延迟时间（毫秒） */

#define HPET_BASE        0xFED00000ull   /* QEMU 默认值 */
#define HPET_GEN_CAP     (volatile uint64_t *)(HPET_BASE + 0x00)
#define HPET_GEN_CONF    (volatile uint64_t *)(HPET_BASE + 0x10)
#define HPET_MAIN_CNT    (volatile uint64_t *)(HPET_BASE + 0xF0)

/* 全局状态 */
uint64_t        tsc_freq = 0;
int             tsc_ok = 0;

/**
 * @brief 获取时间（纳秒）
 * @return uint64_t 
 */
uint64_t plat_get_time_ns(void);

/**
 * @brief 时间初始化，对PIT通道进行10ms校准，算出TSC频率，方便后去获取准确时间
 */
int     plat_time_init(void);

 #endif