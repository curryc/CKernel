    /**
 * @file kernel_main.cpp
 * @brief
 * @author curryc (chencurry5@gmail.com)
 * @date 2025-09-29
 */
#include "pmm.h"
#include "io.h"
#include "iostream"
#include "vgaprint.h"


// 声明内核主函数
extern "C" void kernel_main();

void test()
{
    IO::get_instance().put_char('h');
}

void test_vga()
{
    vga_init();
    vga_setcolor(VGA_WHITE);
    vga_printf("Hello from CKERNEL!\n");
    return;
}

/**
 * @brief 内核主函数
 */
void kernel_main()
{
    test_vga();
    PMM::get_instance().init();
    // test();
    // 无限循环
    while (1)
    {
        __asm__ volatile("hlt");
    }
}
