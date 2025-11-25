/**
 * @brief APIC
 */

#ifndef CKERNEL_APIC_H
#define CKERNEL_APIC_H
#include "cstdint"

class APIC
{
public:
    static APIC &get_instance();
    /**
     * @brief 初始化APIC
     * @return true
     * @return false
     */
    bool init();
    /**
     * @brief 启用irq中断
     * @param  irq_num
     */
    void enable_irq(uint8_t irq_num);

private:
    /**
     * @brief 本地初始化APIC
     * @return true
     * @return false
     */
    bool local_init();
    /**
     * @brief 初始化io apic
     * @return true
     * @return false
     */
    bool io_init();
    /**
     * @brief 初始化定时器
     */
    void timer_init(uint32_t hz);
    /**
     * @brief 初始化8259a
     */
    void pic_init();
    /**
     * @brief 解析madt
     * @param  madt
     */
    void acpi_parse_madt(const void *madt);

public:
    /**
     * @brief 写入APIC寄存器
     * @param  reg
     * @param  val
     */
    void lapic_write(uint32_t reg, uint32_t val);
    /**
     * @brief 读取APIC寄存器
     * @param  reg
     * @return uint32_t
     */
    uint32_t lapic_read(uint32_t reg);
    /**
     * @brief 写入IOAPIC寄存器
     * @param  idx
     * @param  val
     */
    void ioapic_write(uint8_t idx, uint32_t val);
    /**
     * @brief 读取IOAPIC寄存器
     * @param  idx
     * @return uint32_t
     */
    uint32_t ioapic_read(uint8_t idx);

    /**
     * @brief Local APIC MMIO
     */
    void *lapic_mmio = nullptr;
    void *ioapic_mmio = nullptr;

private:
    /**
     * @brief Local APIC ID
     */
    struct
    {
        uint64_t lapic_base;
        uint64_t ioapic_base;
        uint8_t ioapic_id;
        uint8_t ioapic_gsi_base;
    } madt_info{};
    struct rsdp_v1
    {
        char signature[8]; // "RSD PTR "
        uint8_t checksum;
        char oem_id[6];
        uint8_t revision;   // 0 for ACPI 1.0
        uint32_t rsdt_addr; // 32-bit physical address of RSDT
    } __attribute__((packed));
    /*  common SDT header  */
    struct acpi_sdt_header
    {
        char signature[4];
        uint32_t length;
        uint8_t revision;
        uint8_t checksum;
        char oem_id[6];
        char oem_table_id[8];
        uint32_t oem_revision;
        uint32_t creator_id;
        uint32_t creator_revision;
    } __attribute__((packed));

    /*  MADT entry types  */
    enum : uint8_t
    {
        MADT_LAPIC = 0,
        MADT_IOAPIC = 1,
        MADT_IOAPIC_SRC_OVERRIDE = 2,
        MADT_LAPIC_NMI = 4,
    };
};

#endif