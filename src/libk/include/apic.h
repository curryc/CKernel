/**
 * @file apic.h
 * @brief apic 接口
 * @author cbw (chenboven@qq.com)
 * @date 2025-11-12
 */

#ifndef CKERNEL_APIC_H
#define CKERNEL_APIC_H
#include "cstdint"


class APIC
{
public:
    static APIC &get_instance();
    bool init(void);


    // 包含pic的相关代码
    /**
     * @brief pic初始化
     * 会初始化pic片状态，并屏蔽所有中断
     */
    void pic_init();
    void enable_irq(uint8_t irq_num);
    void send_eoi(uint8_t irq_num);

private:
    /* -------------- 寄存器偏移（xAPIC MMIO）-------------- */
    static constexpr uint32_t LAPIC_ID = 0x020;
    static constexpr uint32_t LAPIC_VER = 0x030;
    static constexpr uint32_t LAPIC_TPR = 0x080;
    static constexpr uint32_t LAPIC_EOI = 0x0B0;
    static constexpr uint32_t LAPIC_SIVR = 0x0F0;
    static constexpr uint32_t LAPIC_ICR_LOW = 0x300;
    static constexpr uint32_t LAPIC_ICR_HI = 0x310;
    static constexpr uint32_t LAPIC_LVT_TIMER = 0x320;
    static constexpr uint32_t LAPIC_LVT_LINT0 = 0x350;
    static constexpr uint32_t LAPIC_LVT_LINT1 = 0x360;
    static constexpr uint32_t LAPIC_LVT_ERROR = 0x370;

    /* -------------- MSR -------------- */
    static constexpr uint32_t MSR_IA32_APIC_BASE = 0x1B;
    static constexpr uint64_t APIC_BASE_GLOBAL_EN = (1ULL << 11);
    static constexpr uint64_t APIC_BASE_X2APIC_EN = (1ULL << 10);

    /* -------------- IO-APIC -------------- */
    static constexpr uint32_t IOREGSEL = 0x00;
    static constexpr uint32_t IOWIN = 0x10;
    static constexpr uint32_t IOAPICID = 0x00;
    static constexpr uint32_t IOAPICVER = 0x01;
    static constexpr uint32_t IOREDTBL = 0x10; // 首条重定向表寄存器

    /// 8259A 相关定义
    /// Master (IRQs 0-7)
    static constexpr const uint32_t IO_PIC1 = 0x20;
    /// Slave  (IRQs 8-15)
    static constexpr const uint32_t IO_PIC2 = 0xA0;
    static constexpr const uint32_t IO_PIC1C = IO_PIC1 + 1;
    static constexpr const uint32_t IO_PIC2C = IO_PIC2 + 1;
    /// End-of-interrupt command code
    static constexpr const uint32_t PIC_EOI = 0x20;

    /* ---------- PIC 端口 ---------- */
    static constexpr uint16_t PIC1_COMMAND = 0x20;
    static constexpr uint16_t PIC1_DATA = 0x21;
    static constexpr uint16_t PIC2_COMMAND = 0xA0;
    static constexpr uint16_t PIC2_DATA = 0xA1;

private:
    /**
     * @brief 首先初始化本地APIC
     * @return int 
     */
    int local_init();
    /**
     * @brief 初始化 IO-APIC
     * @return int 
     */
    int io_init();
};

// MADT
struct ACPIMADT
{
    /* 从真实 MADT 里解析，这里只给出两个结果 */
    static uint64_t lapic_addr;  // Local-APIC 物理地址
    static uint64_t ioapic_addr; // 第一个 IO-APIC 物理地址
    static uint8_t ioapic_id;    // 第一个 IO-APIC ID
    /* 入口：在 ACPI 找到 MADT 后把表头指针丢进来 */
    static void parse(void *madt);
};

#endif