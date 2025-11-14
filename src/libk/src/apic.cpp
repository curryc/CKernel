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

void APIC::enable_irq(uint8_t irq_num)
{
    uint16_t port;
    uint8_t mask;

    irq_num = irq_num - 32;
    if (irq_num >= 8) {
        port = PIC2_DATA;
        irq_num -= 8;
    } else {
        port = PIC1_DATA;
    }

    mask = PORT::port_inb(port);
    mask &= ~(1 << irq_num);
    PORT::port_outb(port, mask);
}

// int APIC::local_init() {
//     /* 1. 确保在页表里映射了 lapic_addr，4KB，uncached */
//     // map_mmio(ACPIMADT::lapic_addr, 0x1000);
    

//     /* 2. 启用 APIC（MSR） */
//     uint64_t base = CPU::rdmsr(MSR_IA32_APIC_BASE);
//     if (!(base & APIC_BASE_GLOBAL_EN)) {
//         base |= APIC_BASE_GLOBAL_EN;
//         CPU::wrmsr(MSR_IA32_APIC_BASE, base);
//     }

//     /* 3. 基本初始化 */
//     lapic_write(LAPIC_TPR,  0);               // 任务优先级 0
//     lapic_write(LAPIC_SIVR, 0x1FF);           // 打开 APIC，向量 0xFF
//     lapic_write(LAPIC_LVT_TIMER, 1<<16);      // mask timer
//     lapic_write(LAPIC_LVT_LINT0, 1<<16);      // mask LINT0
//     lapic_write(LAPIC_LVT_LINT1, 1<<16);      // mask LINT1
//     lapic_write(LAPIC_LVT_ERROR, 1<<16);      // mask error
//     lapic_write(LAPIC_EOI, 0);                // 清 EOI
//     return 0;
// }

// int APIC::io_init() {
//     /* 1. 映射 IO-APIC MMIO，通常 0x20 字节就够，但按页 4KB 映射 */
//     // map_mmio(ACPIMADT::ioapic_addr, 0x1000);

//     /* 2. 读版本号（可选） */
//     uint32_t ver = ioapic_read(IOAPICVER);

//     /* 3. 获取重定向表项数 */
//     uint8_t entries = ((ver >> 16) & 0xFF) + 1;

//     /* 4. 全部屏蔽，并设成物理模式、固定交付、向量 0x20+irq */
//     for (uint8_t i = 0; i < entries; ++i) {
//         uint32_t low  = 0x20 + i;   // 向量号
//         uint32_t high = 0;          // destination 0 (物理模式)
//         ioapic_write(IOREDTBL + 2*i,     low  | (1<<16)); // mask=1
//         ioapic_write(IOREDTBL + 2*i + 1, high);
//     }
//     return 0;
// }
/*  common SDT header  */
struct acpi_sdt_header {
    char     signature[4];
    uint32_t      length;
    uint8_t       revision;
    uint8_t       checksum;
    char     oem_id[6];
    char     oem_table_id[8];
    uint32_t      oem_revision;
    uint32_t      creator_id;
    uint32_t      creator_revision;
} __attribute__((packed));

/*  MADT entry types  */
enum : uint8_t {
    MADT_LAPIC = 0,
    MADT_IOAPIC = 1,
    MADT_IOAPIC_SRC_OVERRIDE = 2,
    MADT_LAPIC_NMI = 4,
};
static void* physmap(uint32_t phys)
{
    return (void*)(uint64_t)phys;   // 1:1 mapping in lower 1 GB
}
static bool acpi_checksum(const void* data, size_t len)
{
    uint8_t sum = 0;
    const uint8_t* p = (const uint8_t*)data;
    for (size_t i = 0; i < len; ++i) sum += p[i];
    return sum == 0;
}
void APIC::acpi_init_v1(const uintptr_t rsdp_ptr)
{
    const rsdp_v1* rsdp = (const rsdp_v1*)rsdp_ptr;

    if (memcmp(rsdp->signature, "RSD PTR ", 8) != 0) {
        info("ACPI: RSDP signature bad\n");
        return;
    }
    if (!acpi_checksum(rsdp, sizeof(rsdp_v1))) {
        info("ACPI: RSDP checksum bad\n");
        return;
    }

    uint32_t rsdt_phys = rsdp->rsdt_addr;
    const acpi_sdt_header* rsdt = (const acpi_sdt_header*)rsdt_phys;

    if (memcmp(rsdt->signature, "RSDT", 4) != 0) {
        info("ACPI: RSDT signature bad\n");
        return;
    }
    if (!acpi_checksum(rsdt, rsdt->length)) {
        info("ACPI: RSDT checksum bad\n");
        return;
    }

    /* 遍历 RSDT 指针数组找 MADT */
    uint32_t* entry_ptr = (uint32_t*)((uint8_t*)rsdt + sizeof(acpi_sdt_header));
    info("hello world\n");
    uint32_t  entry_cnt = (rsdt->length - sizeof(acpi_sdt_header)) / sizeof(uint32_t);

    const acpi_sdt_header* madt = nullptr;
    for (uint32_t i = 0; i < entry_cnt; ++i) {
        const acpi_sdt_header* h = (const acpi_sdt_header*)entry_ptr[i];
        if (memcmp(h->signature, "APIC", 4) == 0) {
            madt = h;
            break;
        }
    }
    if (!madt) {
        info("ACPI: MADT not found\n");
        return;
    }
    if (!acpi_checksum(madt, madt->length)) {
        info("ACPI: MADT checksum bad\n");
        return;
    }

    info("ACPI: MADT found, length=%u\n", madt->length);

    /* 遍历 MADT entries */
    const uint8_t* ptr = (const uint8_t*)madt + sizeof(acpi_sdt_header) + 8; // skip local-apic-addr & flags
    const uint8_t* end = (const uint8_t*)madt + madt->length;

    while (ptr < end) {
        uint8_t type = *ptr;
        uint8_t len  = *(ptr + 1);

        if (type == MADT_IOAPIC) {
            struct ioapic_rec {
                uint8_t  type;
                uint8_t  len;
                uint8_t  ioapic_id;
                uint8_t  reserved;
                uint32_t ioapic_addr;   // ← 你要的地址
                uint32_t gsi_base;
            } __attribute__((packed));
            const ioapic_rec* io = (const ioapic_rec*)ptr;
            info("ACPI: I/O APIC id=%u addr=0x%x gsi_base=%u\n",
                       io->ioapic_id, io->ioapic_addr, io->gsi_base);
            /* 这里可以把地址存到全局变量里 */
        }

        ptr += len;
    }
}

/* ---------- 总入口 ---------- */
bool APIC::init(void) {
    pic_init();
    resource_t resource = BOOT_INFO::get_acpi();
    uintptr_t rsdp_ptr = resource.acpi.rsdp;
    acpi_init_v1(rsdp_ptr);
    // if (local_init() < 0) return -1;
    // if (io_init () < 0) return -1;
    return true;
}

APIC& APIC::get_instance() {
    static APIC apic;
    return apic;
}