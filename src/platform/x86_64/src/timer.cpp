/**
 * @file timer.cpp
 * @brief 时钟中断
 * @author cbw (chenboven@qq.com)
 * @date 2025-11-05
 */
#include "intr.h"
#include "cstdio"

static volatile uint64_t g_ticks = 0;

/**
 * @brief 时钟中断
 */
void timer_intr(uint8_t, INTERRUPTS::intr_context_t*) {
    ++g_ticks;
    INTERRUPTS::get_instance().send_eoi(INTERRUPTS::IRQ0);
    info("timer_intr:%ld\n", g_ticks);
    return;
}

TIMER& TIMER::get_instance(void) {
    /// 定义全局 TIMER 对象
    static TIMER timer;
    return timer;
}

bool TIMER::init(void) {
    // 注册中断函数
    INTERRUPTS::get_instance().register_intr_handler(INTERRUPTS::IRQ0, timer_intr);
    // 开启时钟中断
    INTERRUPTS::get_instance().enable_irq(INTERRUPTS::IRQ0);
    // 发送eoi
    INTERRUPTS::get_instance().send_eoi(INTERRUPTS::IRQ0);
    return true;
}
