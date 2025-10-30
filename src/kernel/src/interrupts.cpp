/**
 * @file interrupts.cpp
 * @brief 中断处理实现
 * @author Your Name
 * @date 2025-10-12
 */

#include "interrupts.h"
#include "port.h"
#include "cassert"

namespace INTERRUPTS
{
    // IDT表
    static idt_entry_t idt[256];

    // IDTR
    static idtr_t idtr;

    // ISR处理函数指针数组
    static isr_handler_t isr_handlers[256] = {nullptr};

    // IRQ处理函数指针数组
    static irq_handler_t irq_handlers[16] = {nullptr};

    // PIC端口定义
    constexpr const uint16_t PIC1_COMMAND = 0x20;
    constexpr const uint16_t PIC1_DATA = 0x21;
    constexpr const uint16_t PIC2_COMMAND = 0xA0;
    constexpr const uint16_t PIC2_DATA = 0xA1;

    // PIC命令
    constexpr const uint8_t PIC_EOI = 0x20;

    bool init()
    {
        // 首先验证IDT内存地址是否有效
        uint64_t idt_addr = reinterpret_cast<uint64_t>(&idt[0]);

        // 验证IDT内存对齐 - IDT必须16字节对齐
        if (idt_addr & 0xF)
        {
            info("IDT not properly aligned!\n");
            return false;
        }

        // 验证IDT内存范围是否有效（简单检查）
        if (idt_addr < 0x100000 || idt_addr > 0xFFFFFFFFFFFF)
        {
            err("IDT address out of valid range!\n");
            return false;
        }

        // 初始化IDT
        idtr.limit = sizeof(idt_entry_t) * INTERRUPT_MAX - 1;
        idtr.base = idt_addr;

        info("IDTR: base=0x%lx, limit=0x%x\n", idtr.base, idtr.limit);

        // 清空IDT
        for (int i = 0; i < 256; i++)
        {
            idt[i] = {};
        }

        // 获取代码段选择子（假设为0x08）
        uint16_t code_selector = 0x08;

        

        // 在加载IDT之前验证IDTR
        if (idtr.base == 0 || idtr.limit == 0)
        {
            err("Invalid IDTR: base=0x%lx, limit=0x%x\n", idtr.base, idtr.limit);
            return false;
        }

        // 加载IDT
        info("Loading IDT...\n");
        int64_t result = idt_load(reinterpret_cast<uint64_t>(&idtr));

        if (result != 0)
        {
            err("Failed to load IDT: error code %ld\n", result);
            return false;
        }

        info("IDT loaded successfully!\n");

        // 验证IDT是否被正确加载
        uint64_t cs;
        __asm__ volatile("mov %%cs, %0" : "=r"(cs));
        info("Current CS: 0x%lx\n", cs);

        // 检查特权级别
        if ((cs & 0x3) != 0)
        {
            info("Not in ring 0, cannot access I/O ports!\n");
            return false;
        }

        info("Interrupts system initialized.\n");
        return true;
    }

    void enable()
    {
        __asm__ volatile("sti");
    }

    void disable()
    {
        __asm__ volatile("cli");
    }

    void register_isr_handler(uint8_t irq_num, isr_handler_t handler)
    {
        if (irq_num < 256)
        {
            isr_handlers[irq_num] = handler;
        }
    }

    void register_irq_handler(uint8_t irq_num, irq_handler_t handler)
    {
        if (irq_num < 16)
        {
            irq_handlers[irq_num] = handler;
        }
    }

    void send_eoi(uint8_t irq_num)
    {
        if (irq_num >= 8)
        {
            PORT::port_outb(PIC2_COMMAND, PIC_EOI);
        }
        PORT::port_outb(PIC1_COMMAND, PIC_EOI);
    }

} // namespace INTERRUPTS

// C函数实现，供汇编代码调用
extern "C"
{
    void isr_handler(uint8_t isr_num, intr_context_t *context, uint64_t err_code)
    {
        if (INTERRUPTS::isr_handlers[isr_num])
        {
            INTERRUPTS::isr_handlers[isr_num](isr_num, context, err_code);
        }
        else
        {
            info("Unhandled exception #%d\n", isr_num);
            if (err_code != 0)
            {
                info("Error code: 0x%lx\n", err_code);
            }

            // 打印寄存器状态
            info("RAX: 0x%lx RBX: 0x%lx RCX: 0x%lx RDX: 0x%lx\n",
                 context->rax, context->rbx, context->rcx, context->rdx);
            info("RSI: 0x%lx RDI: 0x%lx RBP: 0x%lx RSP: 0x%lx\n",
                 context->rsi, context->rdi, context->rbp, context->rsp);
            info("R8:  0x%lx R9:  0x%lx R10: 0x%lx R11: 0x%lx\n",
                 context->r8, context->r9, context->r10, context->r11);
            info("R12: 0x%lx R13: 0x%lx R14: 0x%lx R15: 0x%lx\n",
                 context->r12, context->r13, context->r14, context->r15);
            info("RIP: 0x%lx CS:  0x%lx RFLAGS: 0x%lx\n",
                 context->rip, context->cs, context->rflags);

            while (1)
            {
                __asm__ volatile("hlt");
            }
        }
    }

    void irq_handler(uint8_t irq_num, intr_context_t *context)
    {
        // 调用注册的IRQ处理函数
        if (INTERRUPTS::irq_handlers[irq_num - 32])
        {
            INTERRUPTS::irq_handlers[irq_num - 32](irq_num - 32, context);
        }

        // 发送EOI信号
        INTERRUPTS::send_eoi(irq_num - 32);
    }
}
