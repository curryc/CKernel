/**
 * @file interrupts.h
 * @brief 中断处理相关定义和接口
 * @author Your Name
 * @date 2025-10-12
 */

#ifndef CKERNEL_INTERRUPTS_H
#define CKERNEL_INTERRUPTS_H

#include "cstdint"

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
/// 声明加载 IDTR 的函数
extern "C" void idt_load(uint64_t);

// 中断上下文结构体，与汇编代码中保存的寄存器一致
struct intr_context_t {
    // 由pusha_64保存的寄存器
    uint64_t r15;
    uint64_t r14;
    uint64_t r13;
    uint64_t r12;
    uint64_t r11;
    uint64_t r10;
    uint64_t r9;
    uint64_t r8;
    uint64_t rbp;
    uint64_t rdi;
    uint64_t rsi;
    uint64_t rdx;
    uint64_t rcx;
    uint64_t rbx;
    uint64_t rax;

    // 由中断自动压栈的内容
    uint64_t rip;
    uint64_t cs;
    uint64_t rflags;
    uint64_t rsp;
    uint64_t ss;
};


namespace INTERRUPTS
{

    // IDT条目结构
    struct idt_entry_t
    {
        uint16_t offset_low;
        uint16_t selector;
        uint8_t ist;
        uint8_t type_attr;
        uint16_t offset_mid;
        uint32_t offset_high;
        uint32_t reserved;
    } __attribute__((packed));

    // IDTR结构
    struct idtr_t
    {
        uint16_t limit;
        uint64_t base;
    } __attribute__((packed));
    /**
     * @brief 初始化中断系统
     * @return true 成功初始化
     * @return false 初始化失败
     */
    bool init();

    /**
     * @brief 启用中断
     */
    void enable();

    /**
     * @brief 禁用中断
     */
    void disable();

    /**
     * @brief ISR处理函数类型
     * @param irq_num 中断号
     * @param context 中断上下文
     * @param err_code 错误码（如果没有则为0）
     */
    typedef void (*isr_handler_t)(uint8_t irq_num, intr_context_t* context, uint64_t err_code);

    /**
     * @brief 注册ISR处理函数
     * @param irq_num 中断号
     * @param handler 处理函数指针
     */
    void register_isr_handler(uint8_t irq_num, isr_handler_t handler);

    /**
     * @brief IRQ处理函数类型
     * @param irq_num 中断号
     * @param context 中断上下文
     */
    typedef void (*irq_handler_t)(uint8_t irq_num, intr_context_t* context);

    /**
     * @brief 注册IRQ处理函数
     * @param irq_num IRQ号
     * @param handler 处理函数指针
     */
    void register_irq_handler(uint8_t irq_num, irq_handler_t handler);

    /**
     * @brief 发送EOI信号到PIC
     * @param irq_num IRQ号
     */
    void send_eoi(uint8_t irq_num);

} // namespace INTERRUPTS


// 汇编函数声明
extern "C" {
    /**
     * @brief 加载IDT
     * @param idtr IDT寄存器地址
     */
    void idt_load(uint64_t idtr);
    
    /**
     * @brief ISR处理函数（由汇编调用）
     * @param irq_num 中断号
     * @param context 中断上下文
     * @param err_code 错误码
     */
    void isr_handler(uint8_t irq_num, intr_context_t* context, uint64_t err_code);
    
    /**
     * @brief IRQ处理函数（由汇编调用）
     * @param irq_num IRQ号
     * @param context 中断上下文
     */
    void irq_handler(uint8_t irq_num, intr_context_t* context);
}

#endif // CKERNEL_INTERRUPTS_H