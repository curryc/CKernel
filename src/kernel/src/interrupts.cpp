/**
 * @file interrupts.cpp
 * @brief 中断处理实现
 * @author Your Name
 * @date 2025-10-12
 */

#include "interrupts.h"
#include "vgaprint.h"
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
        // 初始化IDT
        idtr.limit = sizeof(idt_entry_t) * 256 - 1;
        idtr.base = reinterpret_cast<uint64_t>(&idt[0]);

        // 清空IDT
        for (int i = 0; i < 256; i++)
        {
            idt[i] = {};
        }

        // 获取代码段选择子（假设为0x08）
        uint16_t code_selector = 0x08;

        // 设置ISR条目（0-31为处理器异常）
        for (int i = 0; i < 32; i++)
        {
            extern void *isr0; // 这些符号在汇编中定义
            void **isrs = reinterpret_cast<void **>(&isr0);

            uint64_t handler = reinterpret_cast<uint64_t>(isrs[i]);
            idt[i].offset_low = handler & 0xFFFF;
            idt[i].offset_mid = (handler >> 16) & 0xFFFF;
            idt[i].offset_high = (handler >> 32) & 0xFFFFFFFF;
            idt[i].selector = code_selector;
            idt[i].ist = 0;
            idt[i].type_attr = 0x8E; // 中断门，DPL=0
            idt[i].reserved = 0;
        }

        // 设置IRQ条目（32-47为IRQ0-15）
        for (int i = 0; i < 16; i++)
        {
            extern void *irq0; // 这些符号在汇编中定义
            void **irqs = reinterpret_cast<void **>(&irq0);

            uint64_t handler = reinterpret_cast<uint64_t>(irqs[i]);
            idt[32 + i].offset_low = handler & 0xFFFF;
            idt[32 + i].offset_mid = (handler >> 16) & 0xFFFF;
            idt[32 + i].offset_high = (handler >> 32) & 0xFFFFFFFF;
            idt[32 + i].selector = code_selector;
            idt[32 + i].ist = 0;
            idt[32 + i].type_attr = 0x8E; // 中断门，DPL=0
            idt[32 + i].reserved = 0;
        }

        // 加载IDT
        idt_load(reinterpret_cast<uint64_t>(&idtr));
        // 初始化PIC
        // ICW1: 开始初始化PIC
        PORT::outb(PIC1_COMMAND, 0x11);
        PORT::outb(PIC2_COMMAND, 0x11);

        // ICW2: 设置中断向量偏移
        PORT::outb(PIC1_DATA, 0x20); // IRQ 0-7 映射到 0x20-0x27
        PORT::outb(PIC2_DATA, 0x28); // IRQ 8-15 映射到 0x28-0x2F

        // ICW3: 设置主从关系
        PORT::outb(PIC1_DATA, 0x04); // IRQ2连接从片
        PORT::outb(PIC2_DATA, 0x02); // 从片连接到主片的IRQ2

        // ICW4: 设置8086模式
        PORT::outb(PIC1_DATA, 0x01);
        PORT::outb(PIC2_DATA, 0x01);

        // OCW1: 屏蔽所有IRQ（可选，根据需要开启）
        PORT::outb(PIC1_DATA, 0xFF); // 屏蔽所有IRQ
        PORT::outb(PIC2_DATA, 0xFF); // 屏蔽所有IRQ

        vga_printf("Interrupts system initialized.\n");
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
            PORT::outb(PIC2_COMMAND, PIC_EOI);
        }
        PORT::outb(PIC1_COMMAND, PIC_EOI);
    }

} // namespace INTERRUPTS

// C函数实现，供汇编代码调用
extern "C"
{


    __attribute__((used, externally_visible))
    void isr_handler(uint8_t irq_num, intr_context_t *context, uint64_t err_code)
    {
        if (INTERRUPTS::isr_handlers[irq_num])
        {
            INTERRUPTS::isr_handlers[irq_num](irq_num, context, err_code);
        }
        else
        {
            vga_printf("Unhandled exception #%d\n", irq_num);
            if (err_code != 0)
            {
                vga_printf("Error code: 0x%lx\n", err_code);
            }

            // 打印寄存器状态
            vga_printf("RAX: 0x%lx RBX: 0x%lx RCX: 0x%lx RDX: 0x%lx\n",
                       context->rax, context->rbx, context->rcx, context->rdx);
            vga_printf("RSI: 0x%lx RDI: 0x%lx RBP: 0x%lx RSP: 0x%lx\n",
                       context->rsi, context->rdi, context->rbp, context->rsp);
            vga_printf("R8:  0x%lx R9:  0x%lx R10: 0x%lx R11: 0x%lx\n",
                       context->r8, context->r9, context->r10, context->r11);
            vga_printf("R12: 0x%lx R13: 0x%lx R14: 0x%lx R15: 0x%lx\n",
                       context->r12, context->r13, context->r14, context->r15);
            vga_printf("RIP: 0x%lx CS:  0x%lx RFLAGS: 0x%lx\n",
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