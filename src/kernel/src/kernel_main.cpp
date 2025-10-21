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
#include "interrupts.h"   
#include "boot_info.h"


// 声明内核主函数
extern "C" void kernel_main();

void test()
{
    // 初始化
    vga_printf("main:Initializing boot info...\n");
    if (!BOOT_INFO::init()) {
        vga_printf("main:Failed to initialize boot info!\n");
        while (1) {
            __asm__ volatile("hlt");
        }
    }

    // 现在初始化PMM
    vga_printf("main:Initializing physical memory manager...\n");
    if (!PMM::get_instance().init()) {
        vga_printf("main:Failed to initialize PMM!\n");
        while (1) {
            __asm__ volatile("hlt");
        }
    }
    
    vga_printf("Test completed successfully!\n");
    
    while (1) {
        __asm__ volatile("hlt");
    }
}

void test_vga()
{
    vga_init();
    vga_setcolor(VGA_WHITE);
    vga_printf("Hello from CKERNEL!!\n");
    return;
}

/**
 * @brief 内核主函数
 */
void kernel_main()
{
    test_vga();
    test();
    // 无限循环
    while (1)
    {
        __asm__ volatile("hlt");
    }
}
