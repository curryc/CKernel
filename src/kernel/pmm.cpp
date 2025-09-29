// kernel/pmm.c
// 物理页分配器实现

#include "pmm.h"
#include <stdint.h>

// 页大小定义
#define PAGE_SIZE 4096

// 内核结束地址符号 (由链接器提供)
extern uint8_t _kernel_end[];

// 位图相关定义
static uint32_t* pmm_bitmap;
static uint32_t pmm_bitmap_size;
static uint32_t pmm_memory_start;
static uint32_t pmm_max_blocks;

// 已初始化标志
static int pmm_initialized = 0;

// 将地址转换为块号
static inline uint32_t pmm_get_block_number(uint32_t address) {
    return (address - pmm_memory_start) / PAGE_SIZE;
}

// 将块号转换为地址
static inline uint32_t pmm_get_address(uint32_t block_number) {
    return pmm_memory_start + (block_number * PAGE_SIZE);
}

// 初始化物理内存管理器
void pmm_init() {
    // 假设系统有128MB可用内存用于页分配
    uint32_t total_memory = 128 * 1024 * 1024;
    
    // 计算内存起始地址 (页对齐，并在内核之后)
    pmm_memory_start = ((uint32_t)(uintptr_t)_kernel_end + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
    
    // 计算最大块数
    pmm_max_blocks = total_memory / PAGE_SIZE;
    
    // 计算位图大小 (以uint32_t为单位)
    pmm_bitmap_size = (pmm_max_blocks + 31) / 32;
    
    // 将位图放在可用内存的开始处
    pmm_bitmap = (uint32_t*)pmm_memory_start;
    
    // 初始化位图 (所有页标记为已使用，除了实际可用的)
    for (uint32_t i = 0; i < pmm_bitmap_size; i++) {
        pmm_bitmap[i] = 0xFFFFFFFF; // 默认全部标记为已使用
    }
    
    // 计算位图占用的块数
    uint32_t bitmap_blocks = (pmm_bitmap_size * sizeof(uint32_t) + PAGE_SIZE - 1) / PAGE_SIZE;
    
    // 标记位图占用的页为已使用
    for (uint32_t i = 0; i < bitmap_blocks; i++) {
        uint32_t block_number = pmm_get_block_number((uint32_t)(uintptr_t)&pmm_bitmap[i]);
        uint32_t bitmap_index = block_number / 32;
        uint32_t bit_index = block_number % 32;
        pmm_bitmap[bitmap_index] |= (1 << bit_index);
    }
    
    // 标记位图之后的页为可用
    for (uint32_t i = bitmap_blocks; i < pmm_max_blocks; i++) {
        uint32_t block_number = i;
        uint32_t bitmap_index = block_number / 32;
        uint32_t bit_index = block_number % 32;
        pmm_bitmap[bitmap_index] &= ~(1 << bit_index);
    }
    
    pmm_initialized = 1;
}

// 分配一个物理页
uint32_t pmm_alloc_page() {
    if (!pmm_initialized) return 0;
    
    // 查找第一个可用页
    for (uint32_t i = 0; i < pmm_bitmap_size; i++) {
        if (pmm_bitmap[i] != 0xFFFFFFFF) {
            // 找到空闲位
            for (int j = 0; j < 32; j++) {
                if (!(pmm_bitmap[i] & (1 << j)) && (i * 32 + j < pmm_max_blocks)) {
                    // 标记为已使用
                    pmm_bitmap[i] |= (1 << j);
                    // 返回物理地址
                    return pmm_get_address(i * 32 + j);
                }
            }
        }
    }
    
    // 没有可用的页
    return 0;
}

// 释放一个物理页
void pmm_free_page(uint32_t address) {
    if (!pmm_initialized || address < pmm_memory_start) return;
    
    uint32_t block_number = pmm_get_block_number(address);
    if (block_number >= pmm_max_blocks) return;
    
    uint32_t bitmap_index = block_number / 32;
    uint32_t bit_index = block_number % 32;
    
    // 标记为未使用
    pmm_bitmap[bitmap_index] &= ~(1 << bit_index);
}