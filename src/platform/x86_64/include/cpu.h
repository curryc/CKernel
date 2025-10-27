/**
 * @file cpu.h
 * @brief
 * @author cbw (chenboven@qq.com)
 * @date 2025-10-24
 */

#ifndef CKERNEL_CPU_H
#define KERNEL_CPU_H

#include "stdint.h"

/**
 * @brief 停止CPU
 */
static void halt(void)
{
    while (1)
    {
        asm volatile("hlt");
    }
}
/**
 * @brief 调用CPUID
 * @param  func
 * @param  a
 * @param  b
 * @param  c
 * @param  d
 */
static inline void cpuid(uint32_t func, uint32_t *a, uint32_t *b,
                         uint32_t *c, uint32_t *d)
{
    asm volatile("cpuid"
                 : "=a"(*a), "=b"(*b), "=c"(*c), "=d"(*d)
                 : "a"(func));
}

/**
 * @brief 获取CPU时间
 * @return uint64_t 
 */
static inline uint64_t rdtsc(void)
{
    uint32_t lo, hi;
    asm volatile("rdtsc" : "=a"(lo), "=d"(hi));
    return ((uint64_t)hi << 32) | lo;
}

/**
 * @brief 暂停CPU
 */
static inline void cpu_pause(void)
{
    __asm__ volatile ("pause" ::: "memory");
}
#endif