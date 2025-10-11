/**
 * @file kernel_main.cpp
 * @brief
 * @author curryc (chencurry5@gmail.com)
 * @date 2025-09-29
 */
#include "pmm.h"
#include "io.h"
#include "iostream"


// 声明内核主函数
extern "C" void kernel_main();

void test()
{
    IO::get_instance().put_char('h');
}

// 使用直接映射的VGA地址
static uint16_t *const VGA = (uint16_t *)0xB8000;
static inline uint16_t vga_entry(char c, uint8_t color = 0x0f)
{
    return (uint16_t)c | (uint16_t)color << 8;
}
void test_vga()
{
    VGA[0] = vga_entry('H', 0x0F);
    VGA[1] = vga_entry('E', 0x0F);
    VGA[2] = vga_entry('L', 0x0F);
    VGA[3] = vga_entry('L', 0x0F);
    VGA[4] = vga_entry('O', 0x0F);
    return;
}

/**
 * @brief 内核主函数
 */
void kernel_main()
{
    mmu_init();
    test();
    // test_vga();
    // 无限循环
    while (1)
    {
        __asm__ volatile("hlt");
    }
}
