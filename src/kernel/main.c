// kernel/main.c
#include <stdint.h>
#include <stddef.h>

// 声明内核主函数
void kernel_main(void* mb_info);

// 使用直接映射的VGA地址
static uint16_t* const VGA = (uint16_t*)0xB8000;

static inline uint16_t vga_entry(char c, uint8_t color) {
    return (uint16_t)c | (uint16_t)color << 8;
}

void kernel_main(void* mb_info) {
    // 显示 "HELLO" 表示内核已运行
    VGA[0] = vga_entry('H', 0x0F);
    VGA[1] = vga_entry('E', 0x0F);
    VGA[2] = vga_entry('L', 0x0F);
    VGA[3] = vga_entry('L', 0x0F);
    VGA[4] = vga_entry('O', 0x0F);
    
    // 无限循环
    while (1) {
        __asm__ volatile ("hlt");
    }
}