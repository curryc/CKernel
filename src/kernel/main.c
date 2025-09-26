// kernel/main.c
#include <stdint.h>
#include <stddef.h>
#include "pmm.h"

// 声明内核主函数
void kernel_main();

// 使用直接映射的VGA地址
static uint16_t* const VGA = (uint16_t*)0xB8000;

static inline uint16_t vga_entry(char c, uint8_t color) {
    return (uint16_t)c | (uint16_t)color << 8;
}

void kernel_main() {
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

    // 初始化物理页分配器
    pmm_init();
    
    // 测试分配页面
    uint32_t page1 = pmm_alloc_page();
    uint32_t page2 = pmm_alloc_page();
    
    // 显示测试结果
    if (page1 != 0 && page2 != 0) {
        // 显示 "PMM OK"
        VGA[0] = vga_entry('P', 0x0F);
        VGA[1] = vga_entry('M', 0x0F);
        VGA[2] = vga_entry('M', 0x0F);
        VGA[3] = vga_entry(' ', 0x0F);
        VGA[4] = vga_entry('O', 0x0F);
        VGA[5] = vga_entry('K', 0x0F);
    } else {
        // 显示 "PMM ERR"
        VGA[0] = vga_entry('P', 0x0F);
        VGA[1] = vga_entry('M', 0x0F);
        VGA[2] = vga_entry('M', 0x0F);
        VGA[3] = vga_entry(' ', 0x0F);
        VGA[4] = vga_entry('E', 0x0F);
        VGA[5] = vga_entry('R', 0x0F);
        VGA[6] = vga_entry('R', 0x0F);
    }
    
    // 测试释放页面
    pmm_free_page(page1);
    pmm_free_page(page2);
    
    // 无限循环
    while (1) {
        __asm__ volatile ("hlt");
    }
}