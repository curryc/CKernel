#ifndef CKERNEL_C_TIME_H
#define CKERNEL_C_TIME_H

#include "stdint.h"

/* 宏定义 */
#define PIT_CONST 1193 /* PIT 每毫秒计数值 */
#define DELAY_MS 10    /* 校准延迟时间（毫秒） */

#define HPET_BASE 0xFED00000ull /* QEMU 默认值 */
#define HPET_GEN_CAP (volatile uint64_t *)(HPET_BASE + 0x00)
#define HPET_GEN_CONF (volatile uint64_t *)(HPET_BASE + 0x10)
#define HPET_MAIN_CNT (volatile uint64_t *)(HPET_BASE + 0xF0)

#ifdef __cplusplus
extern "C"
{
#endif

    /* 纯 C 接口 */
    int plat_time_init(void);
    uint64_t plat_get_time_ns(void);

#ifdef __cplusplus
}
#endif

#endif