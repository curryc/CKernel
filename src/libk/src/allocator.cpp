/**
 * @file allocator.cpp
 * @brief 
 * @author cbw (chenboven@qq.com)
 * @date 2025-10-13
 */

 #include "allocator.h"

 ALLOCATOR::ALLOCATOR(const char *_tag, unsigned long _addr, unsigned long _len)
{
    tag = _tag;
    allocator_start_addr = static_cast<uintptr_t>(_addr);
    allocator_length = static_cast<size_t>(_len);
    allocator_free_count = allocator_length;
    allocator_used_count = 0;
}

// 析构函数应该声明为 virtual
ALLOCATOR::~ALLOCATOR(void)
{
    return;
}