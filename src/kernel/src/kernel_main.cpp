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
#include "cpu.h"


// 声明内核主函数
extern "C" void kernel_main();

void test()
{
    
    // 内核信息初始化
    if (!BOOT_INFO::init()) {
        vga_printf("main:Failed to initialize boot info!\n");
        halt();
    }
    // 初始化PMM
    if (!PMM::get_instance().init()) {
        vga_printf("main:Failed to initialize PMM!\n");
        halt();
    }
    // 清屏，开始使用TUI
    // vga_init();
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


    halt();
}
