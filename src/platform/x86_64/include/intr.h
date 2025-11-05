/**
 * @file intr.h
 * @brief 中断处理相关定义和接口
 * @author Your Name
 * @date 2025-10-12
 */

#ifndef CKERNEL_INTR_H
#define CKERNEL_INTR_H

#include "cstdint"


class INTERRUPTS
{
public:
    /**
     * @brief 错误码结构
     */
    struct error_code_t
    {
        uint32_t ext : 1;
        uint32_t idt : 1;
        uint32_t ti : 1;
        uint32_t sec_idx : 28;
    };

    /**
     * @brief 缺页错误码结构
     */
    struct page_fault_error_code_t
    {
        uint32_t p : 1;
        uint32_t wr : 1;
        uint32_t us : 1;
        uint32_t rsvd : 1;
        uint32_t id : 1;
        uint32_t pk : 1;
        uint32_t reserved1 : 9;
        uint32_t sgx : 1;
        uint32_t reserved2 : 16;
    };
    // 中断上下文结构体，与汇编代码中保存的寄存器一致
    struct intr_context_t
    {
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
private:
    // 系统调用
    static constexpr const uint32_t IRQ128               = 128;

    // IDT条目结构 (符合x86_64规范)
    struct idt_entry_t
    {
        uint16_t offset_low;  // 偏移低16位
        uint16_t selector;    // 代码段选择子
        uint64_t ist : 3;     // 中断栈表
        uint64_t zero0 : 5;   // 填充 0
        uint64_t type : 4;    // 类型
        uint64_t zero1 : 1;   // 填充 0
        uint64_t dpl : 2;     // 权限
        uint64_t p : 1;       // 存在位
        uint16_t offset_mid;  // 偏移中16位
        uint32_t offset_high; // 偏移高32位
        uint32_t reserved;    // 保留必须为0
    } __attribute__((packed));

    // IDTR结构
    struct idtr_t
    {
        uint16_t limit;
        uint32_t base;
    } __attribute__((packed));

public:
    /**
     * @brief Get the instance object
     * @return INTERRUPTS 
     */
    static INTERRUPTS& get_instance(void);
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
     * @brief 中断处理函数类型
     * @param irq_num 中断号
     * @param context 中断上下文
     */
    typedef void (*intr_handler_t)(uint8_t intr_num, intr_context_t *context);

    /**
     * @brief 注册中断处理函数
     * @param irq_num 中断号
     * @param handler 处理函数指针
     */
    void register_intr_handler(uint8_t intr_num, intr_handler_t handler);

    /**
     * @brief 调用中断处理函数
     * @param  _no
     * @param  _intr_context
     * @return int32_t 
     */
    int32_t call_irq(uint8_t _no, intr_context_t* _intr_context);

    /**
     * @brief 调用异常处理函数
     * @param  _no
     * @param  _intr_context
     * @return int32_t 
     */
    int32_t call_isr(uint8_t _no, intr_context_t* _intr_context);

    /**
     * @brief 发送EOI信号到PIC
     * @param irq_num IRQ号
     */
    void send_eoi(uint8_t irq_num);

private:
    /// 最大中断数
    static constexpr const uint32_t INTERRUPT_MAX = 32;
    /// 最大异常数
    static constexpr const uint32_t EXCP_MAX = 256;
    // 中断处理函数指针数组
    static intr_handler_t           interrupt_handlers[INTERRUPT_MAX];
    // 256 项 IDT
    static idt_entry_t              idt[EXCP_MAX]; 
    // IDTR
    static idtr_t                   idtr;
    /**
     * @brief 生成 IDT 项
     * @param  vec
     * @param  handler
     * @param  type
     * @param  dpl
     * @param  _p
     */
    void make_idt_entry(uint8_t vec,
         void (*handler)(), 
         uint8_t type, 
         uint8_t dpl = 0, 
         uint8_t _p = 1);

    
    /* ---------- PIC 端口 ---------- */
    static constexpr uint16_t PIC1_COMMAND = 0x20;
    static constexpr uint16_t PIC1_DATA = 0x21;
    static constexpr uint16_t PIC2_COMMAND = 0xA0;
    static constexpr uint16_t PIC2_DATA = 0xA1;
    static constexpr uint8_t PIC_EOI = 0x20;
    /**
     * @brief 初始化 PIC
     */
    void pic_init();


}; // class INTERRUPTS
#endif // CKERNEL_INTERRUPTS_H
