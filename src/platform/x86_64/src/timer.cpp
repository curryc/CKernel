/**
 * @file timer.cpp
 * @brief 时钟中断
 * @author cbw (chenboven@qq.com)
 * @date 2025-11-05
 */
#include "intr.h"
#include "cstdio"
#include "port.h"

static volatile uint64_t g_ticks = 0;

/**
 * @brief 时钟中断
 */
void timer_intr(uint8_t, INTERRUPTS::intr_context_t *)
{
    ++g_ticks;
    INTERRUPTS::get_instance().send_eoi(INTERRUPTS::IRQ0);
    if (g_ticks % 1000 == 0)
    {
        info("1 second tick\n");
    }
    return;
}

void init_pit(uint32_t hz) {
    uint32_t div = 1193180 / hz;
    PORT::port_outb(0x43, 0x36);
    PORT::port_outb(0x40, div & 0xFF);
    PORT::port_outb(0x40, (div >> 8) & 0xFF);
}

TIMER& TIMER::get_instance(void) {
    /// 定义全局 TIMER 对象
    static TIMER timer;
    return timer;
}



bool TIMER::init(void) {
    init_pit(1000);

    // 注册中断函数
    INTERRUPTS::get_instance().register_intr_handler(INTERRUPTS::IRQ0, timer_intr);
    // 开启时钟中断
    INTERRUPTS::get_instance().enable_irq(INTERRUPTS::IRQ0);
    // 开启中断
    INTERRUPTS::get_instance().enable();

    uint64_t rflags;
    __asm__ volatile("pushfq; pop %0" : "=r"(rflags));
    info("EFLAGS: 0x%lx, IF=%d\n", rflags, (rflags & (1 << 9)) ? 1 : 0);
    return true;
}
