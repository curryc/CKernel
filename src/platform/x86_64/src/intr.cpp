/**
 * @file interrupts.cpp
 * @brief 中断子系统实现（修正版）
 * @date 2025-10-12
 */

#include "intr.h"
#include "port.h"
#include "cassert"
#include "cpu.h"
#include "gdt.h"

// 声明中断处理函数 0 ~ 19 属于 CPU 的异常中断
// ISR:中断服务程序(interrupt service routine)
/// 0 #DE 除 0 异常
extern "C" void isr0(void);
/// 1 #DB 调试异常
extern "C" void isr1(void);
/// 2 NMI
extern "C" void isr2(void);
/// 3 BP 断点异常
extern "C" void isr3(void);
/// 4 #OF 溢出
extern "C" void isr4(void);
/// 5 #BR 对数组的引用超出边界
extern "C" void isr5(void);
/// 6 #UD 无效或未定义的操作码
extern "C" void isr6(void);
/// 7 #NM 设备不可用(无数学协处理器)
extern "C" void isr7(void);
/// 8 #DF 双重故障(有错误代码)
extern "C" void isr8(void);
/// 9 协处理器跨段操作
extern "C" void isr9(void);
/// 10 #TS 无效TSS(有错误代码)
extern "C" void isr10(void);
/// 11 #NP 段不存在(有错误代码)
extern "C" void isr11(void);
/// 12 #SS 栈错误(有错误代码)
extern "C" void isr12(void);
/// 13 #GP 常规保护(有错误代码)
extern "C" void isr13(void);
/// 14 #PF 页故障(有错误代码)
extern "C" void isr14(void);
/// 15 没有使用
extern "C" void isr15(void);
/// 16 #MF 浮点处理单元错误
extern "C" void isr16(void);
/// 17 #AC 对齐检查
extern "C" void isr17(void);
/// 18 #MC 机器检查
extern "C" void isr18(void);
/// 19 #XM SIMD(单指令多数据)浮点异常
extern "C" void isr19(void);
extern "C" void isr20(void);
/// 21 ~ 31 Intel 保留
/// 32 ~ 255 用户自定义异常
/// 0x80 用于实现系统调用
extern "C" void isr128(void);

/// IRQ:中断请求(Interrupt Request)
/// 电脑系统计时器
extern "C" void irq0(void);
/// 键盘
extern "C" void irq1(void);
/// 与 IRQ9 相接，MPU-401 MD 使用
extern "C" void irq2(void);
/// 串口设备
extern "C" void irq3(void);
/// 串口设备
extern "C" void irq4(void);
/// 建议声卡使用
extern "C" void irq5(void);
/// 软驱传输控制使用
extern "C" void irq6(void);
/// 打印机传输控制使用
extern "C" void irq7(void);
/// 即时时钟
extern "C" void irq8(void);
/// 与 IRQ2 相接，可设定给其他硬件
extern "C" void irq9(void);
/// 建议网卡使用
extern "C" void irq10(void);
/// 建议 AGP 显卡使用
extern "C" void irq11(void);
/// 接 PS/2 鼠标，也可设定给其他硬件
extern "C" void irq12(void);
/// 协处理器使用
extern "C" void irq13(void);
/// IDE0 传输控制使用
extern "C" void irq14(void);
/// IDE1 传输控制使用
extern "C" void irq15(void);

// 汇编函数声明
extern "C"
{
    /**
     * @brief 加载IDT
     * @param idtr IDT寄存器地址
     * @return int64_t 成功返回0，失败返回-1
     */
    int64_t idt_load(uint64_t idtr);

    void isr_default_handler(INTERRUPTS::intr_context_t *)
    {
        while (1)
        {
            ;
        }
        return;
    }
    /**
     * @brief ISR处理函数（由汇编调用）
     * @param irq_num 中断号
     * @param context 中断上下文
     * @param err_code 错误码
     */
    void isr_handler(uint8_t irq_num, INTERRUPTS::intr_context_t *context, uint64_t err_code)
    {

        // info("RIP: 0x%lx  CS: 0x%lx  RFLAGS: 0x%lx\n", context->rip, context->cs, context->rflags);
        INTERRUPTS::get_instance().call_isr(irq_num, context);
    }

    /**
     * @brief IRQ处理函数（由汇编调用）
     * @param irq_num IRQ号
     * @param context 中断上下文
     */
    void irq_handler(uint8_t irq_num, INTERRUPTS::intr_context_t *context)
    {
        INTERRUPTS::get_instance().call_irq(irq_num, context);
    }
}
static void handler_default(uint8_t intr_num, INTERRUPTS::intr_context_t *)
{
    while (1)
    {
        ;
    }
    return;
}


// 定义 IDT 表
INTERRUPTS::idt_entry_t INTERRUPTS::idt[EXCP_MAX];

// 定义 IDTR
INTERRUPTS::idtr_t INTERRUPTS::idtr;

// 定义中断处理函数指针数组
INTERRUPTS::intr_handler_t INTERRUPTS::interrupt_handlers[INTERRUPT_MAX];

INTERRUPTS &INTERRUPTS::get_instance(void)
{
    static INTERRUPTS intr;
    return intr;
}

void INTERRUPTS::pic_init()
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

    /* 屏蔽所有 IRQ，稍后由驱动打开 */
    PORT::port_outb(PIC1_DATA, 0xFF);
    PORT::port_outb(PIC2_DATA, 0xFF);
}

/* ---------- 构造 IDT 表项 ---------- */
void INTERRUPTS::make_idt_entry(uint8_t vec, void (*handler)(), uint8_t type, uint8_t dpl, uint8_t _p)
{
    uint64_t addr = reinterpret_cast<uint64_t>(handler);
    idt[vec].offset_low = addr & 0xFFFF;
    idt[vec].selector = 0x08; // 假设平坦模型代码段
    idt[vec].ist = 0;
    idt[vec].zero0 = 0;
    idt[vec].type = type; // 0xE=中断门, 0xF=陷阱门
    idt[vec].zero1 = 0;
    idt[vec].dpl = dpl;
    idt[vec].p = _p;
    idt[vec].offset_mid = (addr >> 16) & 0xFFFF;
    idt[vec].offset_high = (addr >> 32) & 0xFFFFFFFF;
    idt[vec].reserved = 0;
}

/* ---------- 主初始化 ---------- */
bool INTERRUPTS::init()
{
    // 地址合法性检查
    uint64_t idt_addr = reinterpret_cast<uint64_t>(&idt);
    if (idt_addr & 0xF)
    {
        // info("IDT not 16-byte aligned!\n");
        return false;
    }
    if (idt_addr < 0x100000 || idt_addr > 0xFFFFFFFFFFFF)
    {
        // err("IDT address out of range!\n");
        return false;
    }

    // 清空 IDT / 表项清零
    for (auto &e : idt)
        e = {};

    // 构造 IDT
    // External(hardware generated) interrupts.
    // 64-ia-32-architectures-software-developer-vol-3a-manual#6.3.1
#define SET_EXCP(n) make_idt_entry(n, reinterpret_cast<void (*)()>(isr##n), GDT::TYPE_SYSTEM_64_INTERRUPT_GATE, CPU::DPL0, GDT::SEGMENT_PRESENT)
    SET_EXCP(0);  // INT_DIVIDE_ERROR
    SET_EXCP(1);  // INT_DEBUG
    SET_EXCP(2);  // INT_NMI
    SET_EXCP(3);  // INT_BREAKPOINT
    SET_EXCP(4);  // INT_OVERFLOW
    SET_EXCP(5);  // INT_BOUND
    SET_EXCP(6);  // INT_INVALID_OPCODE
    SET_EXCP(7);  // INT_DEVICE_NOT_AVAIL
    SET_EXCP(8);  // INT_DOUBLE_FAULT
    SET_EXCP(9);  // INT_COPROCESSOR
    SET_EXCP(10); // INT_INVALID_TSS
    SET_EXCP(11); // INT_SEGMENT
    SET_EXCP(12); // INT_STACK_FAULT
    SET_EXCP(13); // INT_GENERAL_PROTECT
    SET_EXCP(14); // INT_PAGE_FAULT
    SET_EXCP(15);
    SET_EXCP(16); // INT_X87_FPU
    SET_EXCP(17); // INT_ALIGNMENT
    SET_EXCP(18); // INT_MACHINE_CHECK
    SET_EXCP(19); // INT_SIMD_FLOAT
    SET_EXCP(20); // INT_VIRTUAL_EXCE
#undef SET_EXCP

#define SET_IRQ(i) make_idt_entry(32 + i, reinterpret_cast<void (*)()>(irq##i), GDT::TYPE_SYSTEM_64_INTERRUPT_GATE, CPU::DPL0, GDT::SEGMENT_PRESENT)
    SET_IRQ(0);  // 电脑系统计时器
    SET_IRQ(1);  // 键盘
    SET_IRQ(2);  // 与 IRQ9 相接，MPU-401 MD 使用
    SET_IRQ(3);  // 串口设备
    SET_IRQ(4);  // 串口设备
    SET_IRQ(5);  // 建议声卡使用
    SET_IRQ(6);  // 软驱传输控制使用
    SET_IRQ(7);  // 打印机传输控制使用
    SET_IRQ(8);  // 即时时钟
    SET_IRQ(9);  // 与 IRQ2 相接，可设定给其他硬件
    SET_IRQ(10); // 建议网卡使用
    SET_IRQ(11); // 建议 AGP 显卡使用
    SET_IRQ(12); // 接 PS/2 鼠标，也可设定给其他硬件
    SET_IRQ(13); // 协处理器使用
    SET_IRQ(14); // SATA 主硬盘
    SET_IRQ(15); // SATA 从硬盘
#undef SET_IRQ

    // 统一指向默认桩
    for (uint32_t i = 0; i < INTERRUPT_MAX; i++)
    {
        register_intr_handler(i, handler_default);
    }

    // 加载 IDT
    idtr.base = idt_addr;
    idtr.limit = sizeof(idt) - 1;
    if (idt_load(reinterpret_cast<uint64_t>(&idtr)) != 0)
    {
        // err("Failed to load IDT\n");
        return false;
    }

    // 初始化 PIC
    pic_init();

    // info("Interrupt subsystem initialized.\n");
    return true;
}
int32_t INTERRUPTS::call_isr(uint8_t _no, intr_context_t* _intr_context)
{
    if (_no < EXCP_MAX && interrupt_handlers[_no] != nullptr)
    {
        interrupt_handlers[_no](_no, _intr_context);
        return 0;
    }
    return -1; // 未找到处理函数
}

int32_t INTERRUPTS::call_irq(uint8_t _no, intr_context_t* _intr_context)
{
    if (_no < INTERRUPT_MAX && interrupt_handlers[_no] != nullptr)
    {
        interrupt_handlers[_no](_no, _intr_context);
        return 0;
    }
    return -1; // 未找到处理函数
}

/* ---------- 运行时装载 ---------- */
void INTERRUPTS::enable() { 
    __asm__ volatile("sti"); 
}
void INTERRUPTS::disable() { 
    __asm__ volatile("cli"); 
}

void INTERRUPTS::register_intr_handler(uint8_t intr_num, intr_handler_t handler)
{
    interrupt_handlers[intr_num] = handler;
    return;
}

void INTERRUPTS::send_eoi(uint8_t irq_num)
{
    if (irq_num >= 8)
        PORT::port_outb(PIC2_COMMAND, PIC_EOI);
    PORT::port_outb(PIC1_COMMAND, PIC_EOI);
}
