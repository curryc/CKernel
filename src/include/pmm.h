#ifndef PMM_H
#define PMM_H

#include <stdint.h>

// 初始化物理内存管理器
void pmm_init();

// 分配一个物理页 (返回4KB对齐的物理地址)
uint32_t pmm_alloc_page();

// 释放一个物理页
void pmm_free_page(uint32_t address);

#endif