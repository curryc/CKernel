/**
 * @file multiboot2.cpp
 * @brief
 * @author cbw (chenboven@qq.com)
 * @date 2025-10-11
 */

#include "multiboot2.h"
#include "cassert"
#include "common.h"

MULTIBOOT2 &MULTIBOOT2::get_instance(void)
{
    static MULTIBOOT2 instance;
    return instance;
}

bool MULTIBOOT2::multiboot2_init(void)
{
    uintptr_t addr = BOOT_INFO::boot_info_addr;
    vga_printf("multiboot2_init:Boot info address: 0x%x\n", addr);
    vga_printf("multiboot2_init:Magic number: 0x%x\n", BOOT_INFO::multiboot2_magic);
    
    // 判断魔数是否正确
    if (BOOT_INFO::multiboot2_magic != MULTIBOOT2_BOOTLOADER_MAGIC) {
        vga_printf("multiboot2_init:Invalid magic number!\n");
        return false;
    }
    
    if ((reinterpret_cast<uintptr_t>(addr) & 7) != 0) {
        vga_printf("multiboot2_init:Address not aligned!\n");
        return false;
    }
    
    BOOT_INFO::boot_info_size = *(uint32_t *)addr;
    vga_printf("multiboot2_init:Boot info size: %d\n", BOOT_INFO::boot_info_size);
    return true;
}

void MULTIBOOT2::multiboot2_iter(bool (*_fun)(const iter_data_t *, void *), void *_data)
{
    if (!BOOT_INFO::boot_info_addr) {
        return;
    }
    
    uintptr_t addr = BOOT_INFO::boot_info_addr;
    assert((addr & 7) == 0); // 确保8字节对齐
    
    // 验证最小大小
    if (BOOT_INFO::boot_info_size < sizeof(uint32_t) * 2) {
        return;
    }
    
    iter_data_t *tag = (iter_data_t *)(addr + 8);
    
    // 添加边界检查
    uintptr_t end_addr = addr + BOOT_INFO::boot_info_size;
    
    while ((uintptr_t)tag < end_addr && tag->type != MULTIBOOT_TAG_TYPE_END) {
        // 验证当前tag在有效范围内
        if ((uintptr_t)tag + sizeof(iter_data_t) > end_addr || 
            (uintptr_t)tag + tag->size > end_addr) {
            break;
        }
        
        if (_fun(tag, _data) == true) {
            return;
        }
        
        tag = (iter_data_t *)((uint8_t *)tag + common::ALIGN(tag->size, 8));
        
        // 确保指针前进后仍在有效范围内
        if ((uintptr_t)tag >= end_addr) {
            break;
        }
    }
}

bool MULTIBOOT2::get_memory(const iter_data_t *_iter_data, void *_data)
{
    if (_iter_data->type != MULTIBOOT2::MULTIBOOT_TAG_TYPE_MMAP)
    {
        return false;
    }
    resource_t *resource = (resource_t *)_data;
    resource->type |= resource_t::MEM;
    resource->name = (char *)"available phy memory";
    resource->mem.addr = 0x0;
    resource->mem.len = 0;
    MULTIBOOT2::multiboot_mmap_entry_t *mmap = ((MULTIBOOT2::multiboot_tag_mmap_t *)_iter_data)->entries;
    for (; (uint8_t *)mmap < (uint8_t *)_iter_data + _iter_data->size;
         mmap = (MULTIBOOT2::
                     multiboot_mmap_entry_t *)((uint8_t *)mmap + ((MULTIBOOT2::multiboot_tag_mmap_t *)
                                                                      _iter_data)
                                                                     ->entry_size))
    {
        // 如果是可用内存或地址小于 1M
        // 这里将 0~1M 的空间全部算为可用，在 c++ 库可用后进行优化
        if (mmap->type == MULTIBOOT_MEMORY_AVAILABLE || mmap->addr < 1 * common::MB)
        {
            // 长度+
            resource->mem.len += mmap->len;
        }
    }
    return true;
}

namespace BOOT_INFO
{
    // 地址
    uintptr_t boot_info_addr;
    // 长度
    size_t boot_info_size;
    // 魔数
    uint32_t multiboot2_magic;

    bool inited = false;

    bool init(void)
    {
        auto res = MULTIBOOT2::get_instance().multiboot2_init();
        if (inited == false)
        {
            inited = true;
            vga_printf("boot_info_init:BOOT_INFO init.\n");
        }
        else
        {
            vga_printf("boot_info_init:BOOT_INFO reinit.\n");
        }
        return res;
    }

    resource_t get_memory(void)
    {
        if (!BOOT_INFO::inited) {
            vga_printf("BOOT_INFO not inited.\n");
            resource_t empty_resource;
            return empty_resource;
        }
        resource_t resource;
                while (1)
        {
            __asm__ volatile("hlt");
        }
        MULTIBOOT2::get_instance().multiboot2_iter(MULTIBOOT2::get_memory,
                                                   &resource);
        while (1)
        {
            __asm__ volatile("hlt");
        }
        return resource;
    }
}; // namespace BOOT_INFO