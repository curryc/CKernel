#include "time.h"
#include "port.h"
#include "cpu.h"

/**
 * @brief 通过忙等待矫正TSC频率
 * @param  ms
 */
static void pit_mdelay(uint32_t ms)
{
    port_outb(0x43, 0x30); /* 通道 0，模式 0，二进制 */
    for (uint32_t i = 0; i < ms; ++i) {
        uint16_t cnt = PIT_CONST;
        port_outb(0x40, cnt & 0xFF);
        port_outb(0x40, cnt >> 8);

        /* 等待计数到 0 */
        while ((port_inb(0x61) & 0x20) == 0)
            ;
    }
}

static uint64_t calibrate_tsc(void)
{
    uint64_t tsc0, tsc1;

    tsc0 = rdtsc();
    pit_mdelay(DELAY_MS);
    tsc1 = rdtsc();

    return (tsc1 - tsc0) * 1000 / DELAY_MS; /* Hz */
}

void plat_time_init(void)
{
    // 检查CPU是否支持不变的TSC
    uint32_t a, b, c, d;
    cpuid(0x80000007, &a, &b, &c, &d);
    if (!(d & (1u << 8))) {
        return; /* 老 CPU，放弃 */
    }
    tsc_freq = calibrate_tsc();
    tsc_ok = (tsc_freq != 0 ? 1 : 0);
}

uint64_t plat_get_time_ns(void)
{
    if (!tsc_ok) return 0;
    return (rdtsc() * 1000000000ull) / tsc_freq;
}