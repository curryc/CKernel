    /**
 * @file kernel_main.cpp
 * @brief
 * @author curryc (chencurry5@gmail.com)
 * @date 2025-09-29
 */
#include "pmm.h"
#include "io.h"
#include "vgaprint.h"
#include "interrupts.h"   
#include "boot_info.h"
#include "cpu.h"

#include "iostream"
#include "ctime"


// 声明内核主函数
extern "C" void kernel_main();

void pmm_test()
{
    info("Starting PMM test...\n");

    // 获取 PMM 实例
    PMM &pmm = PMM::get_instance();

    // 手动管理分配地址的数组
    uintptr_t allocated_addresses[10]; // 假设最多分配 10 次
    size_t allocated_count = 0;

    // 随机分配测试
    size_t alloc_count = 10; // 测试分配 10 次
    for (size_t i = 0; i < alloc_count; ++i)
    {
        // 使用时间戳生成伪随机页数
        uint64_t timestamp = time(NULL);
        size_t pages_to_alloc = (timestamp % 5) + 1; // 1 到 5 页
        uintptr_t addr = pmm.alloc_pages(pages_to_alloc);

        if (addr != 0) // 检查分配是否成功
        {
            info("Allocated %zu pages at address: 0x%x\n", pages_to_alloc, addr);
            allocated_addresses[allocated_count++] = addr; // 记录分配地址
        }
        else
        {
            info("Failed to allocate %zu pages.\n", pages_to_alloc);
        }
    }

    // 随机释放测试
    size_t free_count = allocated_count / 2; // 释放一半的分配
    for (size_t i = 0; i < free_count; ++i)
    {
        // 使用时间戳生成伪随机索引
        uint64_t timestamp = time(NULL);
        size_t index = timestamp % allocated_count;

        uintptr_t addr = allocated_addresses[index];

        // 假设每次释放一页
        pmm.free_page(addr);
        info("Freed page at address: 0x%x\n", addr);

        // 将最后一个地址移到当前索引位置，并减少已分配数量
        allocated_addresses[index] = allocated_addresses[allocated_count - 1];
        allocated_count--;
    }

    // 打印当前内存状态
    size_t used_pages = pmm.get_used_pages_count();
    size_t free_pages = pmm.get_free_pages_count();
    info("PMM Test Completed. Used Pages: %zu, Free Pages: %zu\n", used_pages, free_pages);
}

void test()
{
    // 内核信息初始化

    if (!BOOT_INFO::init()) {
        vga_printf("main:Failed to initialize boot info!\n");
        CPU::halt();
    }

    // 初始化PMM
    if (!PMM::get_instance().init()) {
        vga_printf("main:Failed to initialize PMM!\n");
        CPU::halt();
    }
    // 清屏，开始使用TUI
    vga_init();
    // pmm_test();// PMM测试

    // 初始化中断
    if (!INTERRUPTS::init()) {
        vga_printf("main:Failed to initialize interrupts!\n");
        CPU::halt();
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


    CPU::halt();
}
