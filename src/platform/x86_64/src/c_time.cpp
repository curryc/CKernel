#include "c_time.h"
#include "cpu.h"

/* 全局状态 */
static uint64_t tsc_freq = 0;
static int tsc_ok = 0;

extern "C" {

int plat_time_init(void)
{
    uint32_t a, b, c, d;
    CPU::cpuid(0x80000007, &a, &b, &c, &d);
    if (!(d & (1u << 8))) return -22;

    uint64_t tsc0 = CPU::rdtsc();
    uint64_t start = *HPET_MAIN_CNT;
    uint64_t cap = *HPET_GEN_CAP;
    uint32_t period_fs = (uint32_t)(cap >> 32);
    uint64_t hpet_freq = 1000000000000000ull / period_fs;
    uint64_t ticks = (hpet_freq * 10) / 1000;

    while (*HPET_MAIN_CNT - start < ticks)
        CPU::cpu_pause();

    uint64_t tsc1 = CPU::rdtsc();
    tsc_freq = (tsc1 - tsc0) * 1000 / 10;
    tsc_ok = 0;
    return tsc_ok;
}

uint64_t plat_get_time_ns(void)
{
    if (!tsc_ok) return CPU::rdtsc();
    return (CPU::rdtsc() * 1000000000ull) / tsc_freq;
}

}   // extern "C"