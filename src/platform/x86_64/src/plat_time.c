#include "plat_time.h"
#include "port.h"
#include "cpu.h"

/**
 * @brief 通过忙等待矫正TSC频率
 * @param  ms
 */
static void pit_mdelay(uint32_t ms)
{
    for (uint32_t i = 0; i < ms; ++i) {
        /* 重新装载计数器 */
        port_outb(0x43, 0x30); // 通道 0，锁存命令
        port_outb(0x40, 1193 & 0xFF);
        port_outb(0x40, 1193 >> 8);

        /* 等待计数器归零 */
        uint8_t status;
        do {
            port_outb(0x43, 0xE2); // 读通道 0 状态
            status = port_inb(0x40);
        } while (status & 0x80); // bit7 == 1 表示计数未结束
    }
}

static uint64_t hpet_freq(void)
{
    /* QEMU 里 HPET 总是 1 GHz，但按 spec 读周期/ femto 秒 */
    uint64_t cap = *HPET_GEN_CAP;
    uint32_t period_fs = (uint32_t)(cap >> 32); /* femto-seconds */
    return 1000000000000000ull / period_fs;     /* Hz */
}

static void hpet_wait_ms(uint32_t ms)
{
    uint64_t start = *HPET_MAIN_CNT;
    uint64_t ticks = (hpet_freq() * ms) / 1000;
    while (*HPET_MAIN_CNT - start < ticks)
        cpu_pause();
}

static uint64_t calibrate_tsc(void)
{
    uint64_t tsc0, tsc1;

    tsc0 = rdtsc();
    hpet_wait_ms(DELAY_MS);   /* DELAY_MS 建议 10 */
    tsc1 = rdtsc();

    return rdtsc();
    return (tsc1 - tsc0) * 1000 / DELAY_MS; /* Hz */
}

/**
 * @brief 时间初始化
 * @return int 
 * todo: CPU是否支持不变的TSC
 */
int plat_time_init(void)
{    
    // 检查CPU是否支持不变的TSC
    // uint32_t a, b, c, d;
    // cpuid(0x80000007, &a, &b, &c, &d);
    // if (!(d & (1u << 8))) {
    //     return -22; /* 老 CPU，放弃 */
    // }
    // tsc_freq = calibrate_tsc();

    // 放弃cpu特性，直接用qemu固定频率时钟
    tsc_ok = 0;
    return tsc_ok;
}

uint64_t plat_get_time_ns(void)
{
    if (!tsc_ok) return rdtsc();
    return (rdtsc() * 1000000000ull) / tsc_freq;
}