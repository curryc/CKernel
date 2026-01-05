/**
 * @file apic.cpp
 * @brief
 * @author cbw (chenboven@qq.com)
 * @date 2025-11-12
 */
#include "apic.h"
#include "cpu.h"
#include "port.h"
#include "boot_info.h"
#include "cstring"
#include "cstdio"
#include "common.h"
#include "vmm.h"
#include "intr.h"

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
static constexpr uint32_t LAPIC_TIMER_DIV   = 0x3E0;
static constexpr uint32_t LAPIC_TIMER_ICR   = 0x380;
static constexpr uint32_t LAPIC_TIMER_CCR   = 0x390;

/* -------------- MSR -------------- */
static constexpr uint32_t MSR_IA32_APIC_BASE = 0x1B;
static constexpr uint64_t APIC_BASE_GLOBAL_EN = (1ULL << 11);
static constexpr uint64_t APIC_BASE_X2APIC_EN = (1ULL << 10);

/* -------------- IO-APIC -------------- */
static constexpr uint8_t IOAPIC_IOREGSEL = 0x00;
static constexpr uint8_t IOAPIC_IOWIN = 0x10;
static constexpr uint8_t IOAPIC_ID = 0x00;
static constexpr uint8_t IOAPIC_VER = 0x01;
static constexpr uint8_t IOREDTBL = 0x10;

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

static bool acpi_checksum(const void *data, size_t len)
{
    uint8_t sum = 0;
    const uint8_t *p = (const uint8_t *)data;
    for (size_t i = 0; i < len; ++i)
        sum += p[i];
    return sum == 0;
}

void APIC::lapic_write(uint32_t reg, uint32_t val)
{
    *(volatile uint32_t *)((uint8_t *)lapic_mmio + reg) = val;
}
uint32_t APIC::lapic_read(uint32_t reg)
{
    return *(volatile uint32_t *)((uint8_t *)lapic_mmio + reg);
}
void APIC::ioapic_write(uint8_t idx, uint32_t val)
{
    *(volatile uint32_t *)((uint8_t *)ioapic_mmio + IOAPIC_IOREGSEL) = idx;
    *(volatile uint32_t *)((uint8_t *)ioapic_mmio + IOAPIC_IOWIN) = val;
}
uint32_t APIC::ioapic_read(uint8_t idx)
{
    *(volatile uint32_t *)((uint8_t *)ioapic_mmio + IOAPIC_IOREGSEL) = idx;
    return *(volatile uint32_t *)((uint8_t *)ioapic_mmio + IOAPIC_IOWIN);
}

void APIC::pic_init()
{
    /* ICW1：级联，需要 ICW4 */
    PORT::port_outb(PIC1_COMMAND, 0x11);
    PORT::port_outb(PIC2_COMMAND, 0x11);

    /* ICW2：中断向量偏移 0x20 / 0x28 */
    PORT::port_outb(PIC1_DATA, 0x20);
    PORT::port_outb(PIC2_DATA, 0x28);

    /* ICW3：主片 IRQ2 级联从片；从片 ID=2 */
    PORT::port_outb(PIC1_DATA, 0x04);
    PORT::port_outb(PIC2_DATA, 0x02);

    /* ICW4：8086 模式 */
    PORT::port_outb(PIC1_DATA, 0x01);
    PORT::port_outb(PIC2_DATA, 0x01);

    /* 屏蔽所有 IRQ*/
    PORT::port_outb(PIC1_DATA, 0xFF);
    PORT::port_outb(PIC2_DATA, 0xFF);
}


bool APIC::local_init()
{
    /* 从 MSR 拿到物理基址 */
    uint64_t base = CPU::rdmsr(MSR_IA32_APIC_BASE);
    if (!(base & APIC_BASE_GLOBAL_EN))
    {
        base |= APIC_BASE_GLOBAL_EN;
        CPU::wrmsr(MSR_IA32_APIC_BASE, base);
    }
    base &= 0xFFFFF000; // 4K 对齐
    madt_info.lapic_base = base;

    // 映射 4K  uncached MMIO
    lapic_mmio = (void *)base; // 假设 1:1 已映射，否则走 VMM::mmap
    VMM::get_instance().mmap(VMM::get_instance().get_pgd(), base, base, VMM::VMM_PAGE_VALID | VMM::VMM_PAGE_READABLE | VMM::VMM_PAGE_WRITABLE);

    lapic_write(LAPIC_TPR, 0);             // 任务优先级 0
    lapic_write(LAPIC_SIVR, 0x1FF);        // 开启 APIC，向量 0xFF
    lapic_write(LAPIC_LVT_TIMER, 0); // no mask timer
    lapic_write(LAPIC_LVT_LINT0, 1 << 16); // mask LINT0
    lapic_write(LAPIC_LVT_LINT1, 1 << 16); // mask LINT1
    lapic_write(LAPIC_LVT_ERROR, 1 << 16); // mask error
    lapic_write(LAPIC_EOI, 0);             // 清 EOI

    lapic_write(LAPIC_LVT_TIMER, 0x00020020); // vector=0x20, periodic
    lapic_write(LAPIC_TIMER_DIV, 0x3);        // div16
    lapic_write(LAPIC_TIMER_ICR, 100000);     // 1 MHz 下 100 kHz

    return true;
}

bool APIC::io_init()
{
    if (!madt_info.ioapic_base) // MADT 未解析到
        return false;

    /* 1. 映射 IO-APIC MMIO（32 字节足够，按 4K 页） */
    ioapic_mmio = (void *)madt_info.ioapic_base;
    VMM::get_instance().mmap(VMM::get_instance().get_pgd(), madt_info.ioapic_base, madt_info.ioapic_base, VMM::VMM_PAGE_VALID | VMM::VMM_PAGE_READABLE | VMM::VMM_PAGE_WRITABLE);

    /* 2. 读版本号并计算重定向表项数 */
    uint32_t ver = ioapic_read(IOAPIC_VER);
    uint8_t entries = ((ver >> 16) & 0xFF) + 1;

    /* 3. 全部屏蔽，固定交付，物理模式，向量 0x20+irq */
    for (uint8_t i = 0; i < entries; ++i)
    {
        uint32_t low = 0x20 + i;                         // 向量号
        uint32_t high = 0;                               // destination = 0
        ioapic_write(IOREDTBL + 2 * i, low | (1 << 16)); // mask=1
        ioapic_write(IOREDTBL + 2 * i + 1, high);
    }
    return true;
}

void APIC::timer_init(uint32_t hz)
{
    /* 0. 先停 Timer（mask 掉，防止旧周期干扰） */
    lapic_write(LAPIC_LVT_TIMER, 1 << 16);   // bit16 = mask
    // init_8254_pit(100);

    /* 1. 分频 */
    lapic_write(LAPIC_TIMER_DIV, 0x3);       // divide by 16

    /* 2. 装初始计数 */
    uint32_t ticks = 1000000 / hz;           // 1 MHz 近似
    lapic_write(LAPIC_TIMER_ICR, ticks);

    /* 3. 最后“点火”：向量 0x20 + periodic + 不屏蔽 */
    lapic_write(LAPIC_LVT_TIMER, 0x20 | (1 << 17)); // 0x20020
}

void APIC::acpi_parse_madt(const void *madt)
{
    const uint8_t *ptr = (const uint8_t *)madt + 44; // skip header+lapic-addr+flags
    const uint8_t *end = (const uint8_t *)madt + ((const acpi_sdt_header *)madt)->length;

    while (ptr < end)
    {
        uint8_t type = *ptr;
        uint8_t len = *(ptr + 1);

        if (type == 1) // IOAPIC
        {
            struct
            {
                uint8_t type, len, id, rsvd;
                uint32_t addr, gsi_base;
            } __attribute__((packed)) *rec = (decltype(rec))ptr;
            madt_info.ioapic_base = rec->addr;
            madt_info.ioapic_id = rec->id;
            madt_info.ioapic_gsi_base = rec->gsi_base;
            break; // 只拿第一个 IO-APIC
        }
        ptr += len;
    }
}

bool APIC::init()
{
    /* 0. 关 8259A，保证不冲突 */
    pic_init();

    /* 1. 取 RSDP（保持原句不动） */
    resource_t resource = BOOT_INFO::get_acpi();
    uintptr_t rsdp = resource.acpi.rsdp;

    /* 2. 简单验证 RSDP 并找 MADT */
    const auto *rsdp_ptr = (const rsdp_v1 *)rsdp;
    if (memcmp(rsdp_ptr->signature, "RSD PTR ", 8) ||
        !acpi_checksum(rsdp_ptr, sizeof(rsdp_v1)))
        return false;

    const auto *rsdt = (const acpi_sdt_header *)(uint64_t)rsdp_ptr->rsdt_addr;
    if (memcmp(rsdt->signature, "RSDT", 4) ||
        !acpi_checksum(rsdt, rsdt->length))
        return false;

    const acpi_sdt_header *madt = nullptr;
    uint32_t *entry = (uint32_t *)((uint8_t *)rsdt + 36);
    uint32_t count = (rsdt->length - 36) / 4;
    for (uint32_t i = 0; i < count; ++i)
    {
        const auto *h = (const acpi_sdt_header *)(uint64_t)entry[i];
        if (memcmp(h->signature, "APIC", 4) == 0 &&
            acpi_checksum(h, h->length))
        {
            madt = h;
            break;
        }
    }
    if (!madt)
        return false;

    acpi_parse_madt(madt);

    /* 3. 分阶段初始化 */
    if (!local_init())
        return false;
    if (!io_init())
        return false;

    timer_init(common::DEFAULT_SYS_FREQ);

    return true;
}

APIC &APIC::get_instance()
{
    static APIC instance;
    return instance;
}

void INTERRUPTS::test(){

    // /* --- debug：回读 RTE2 --- */
    // uint32_t lo = APIC::get_instance().ioapic_read(0x10 + 2*2);
    // uint32_t hi = APIC::get_instance().ioapic_read(0x10 + 2*2 + 1);
    // info("RTE2 hi=%08x lo=%08x  (mask=%u vector=%u)\n",hi, lo, (lo>>16)&1, lo&0xff);
}



void INTERRUPTS::enable_irq(uint8_t gsi)
{
    if (gsi >= 24) return;

    /* ---- 1. 8259A 兼容段（若还想用）---- */
    if (gsi < 16) {
        uint16_t port = gsi < 8 ? PIC1_DATA : PIC2_DATA;
        uint8_t  bit  = gsi & 7;
        uint8_t  mask = PORT::port_inb(port);
        mask &= ~(1 << bit);
        PORT::port_outb(port, mask);
    }

    /* IO-APIC 段 */
    uint8_t  rte_idx = gsi;
    uint32_t vector  = 0x20 + gsi;
    uint32_t high    = 0;                  // dest = 0
    uint32_t low     = vector | (0 << 16); // mask=0

    APIC::get_instance().ioapic_write(0x10 + rte_idx*2 + 1, high);
    APIC::get_instance().ioapic_write(0x10 + rte_idx*2,     low);
    info("RTE%d hi=%08x lo=%08x  (mask=%u vector=%u)\n",
         rte_idx, high, low, (low>>16)&1, low&0xff);   
}

void INTERRUPTS::send_eoi(uint8_t irq_num){
    APIC::get_instance().lapic_write(LAPIC_EOI, 0);
}