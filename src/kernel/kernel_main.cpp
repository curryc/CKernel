/**
 * @file kernel_main.cpp
 * @brief 
 * @author curryc (chencurry5@gmail.com)
 * @date 2025-09-29
 */
#include "pmm.h"
#include "io.h"

// 声明内核主函数
extern "C" void kernel_main();

void test(){
    printf("hello world\n");
}

/**
 * @brief 内核主函数
 */
void kernel_main() {
    test();

    // 无限循环
    while (1) {
        __asm__ volatile ("hlt");
    }
    
}
