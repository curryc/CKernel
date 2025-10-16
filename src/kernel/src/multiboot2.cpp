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
    vga_printf("multiboot2_init:Magic number: 0x%lx\n", BOOT_INFO::multiboot2_magic);
    uintptr_t addr = BOOT_INFO::boot_info_addr;
    vga_printf("multiboot2_init:Boot info address: 0x%lx\n", addr);
    vga_printf("multiboot2_init:Boot info address: 0x%lx\n", addr);
    vga_printf("multiboot2_init:Boot info address: 0x%lx\n", addr);
    vga_printf("multiboot2_init:Boot info address: 0x%lx\n", addr);
    vga_printf("multiboot2_init:Boot info address: 0x%lx\n", addr);
    vga_printf("multiboot2_init:Boot info address: 0x%lx\n", addr);
    
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
    vga_printf("multiboot2_init:end addr: 0x%lx\n", BOOT_INFO::boot_info_addr + BOOT_INFO::boot_info_size);
    return true;
}

void MULTIBOOT2::multiboot2_iter(bool (*_fun)(const iter_data_t *, void *), void *_data)
{
    if (!BOOT_INFO::boot_info_addr) {
        vga_printf("multiboot2_iter:Boot info not inited.\n");
        return;
    }
    uintptr_t addr = BOOT_INFO::boot_info_addr;
    assert((addr & 7) == 0); // 确保8字节对齐
    
    // 跳过头部的8字节(total_size和reserved)，指向第一个标签
    iter_data_t *tag = (iter_data_t *)(addr + 8);
    
    // 遍历直到结束
    for (; tag->type != MULTIBOOT_TAG_TYPE_END;
         tag = (iter_data_t*)((uint8_t*)tag + common::ALIGN(tag->size, 8))) {
        if (_fun(tag, _data) == true) {
            return;
        }
    }
    return;
}

bool MULTIBOOT2::get_memory(const iter_data_t *_iter_data, void *_data)
{
    if (_iter_data->type != MULTIBOOT2::MULTIBOOT_TAG_TYPE_MMAP) {
        return false;
    }

    vga_printf("multiboot2_get_memory:Memory tag found\n");

    resource_t *resource = (resource_t *)_data;
    resource->type |= resource_t::MEM;
    resource->name = (char *)"available phy memory";
    resource->mem.addr = 0x0;
    resource->mem.len = 0;

    // 获取 mmap 标签
    multiboot_tag_mmap_t *mmap_tag = (multiboot_tag_mmap_t *)_iter_data;


    uint32_t size = mmap_tag->size;
    uint32_t *end_addr = (uint32_t *)mmap_tag + size;
    uint32_t entry_size = mmap_tag->entry_size;

    vga_printf("Tag size: %d, Entry size: %d\n", size, entry_size);

        while (1)
        {
            __asm__ volatile("hlt");
        }
    MULTIBOOT2::multiboot_mmap_entry_t *mmap = ((MULTIBOOT2::multiboot_tag_mmap_t *)_iter_data)->entries;
    for (; (uint32_t *)mmap < end_addr;
         mmap = (multiboot_mmap_entry_t *)((uint8_t *)mmap + size))
    {
        while (1)
        {
            __asm__ volatile("hlt");
        }
        // 如果是可用内存或地址小于 1M
        // 这里将 0~1M 的空间全部算为可用，在 c++ 库可用后进行优化
        if (mmap->type == MULTIBOOT_MEMORY_AVAILABLE || mmap->addr < 1 * common::MB)
        {
            // 长度+
            resource->mem.len += mmap->len;
        }
    }

    vga_printf("Total available memory: %ubytes\n", resource->mem.len);
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
        MULTIBOOT2::get_instance().multiboot2_iter(MULTIBOOT2::get_memory,
                                                   &resource);
        while (1)
        {
            __asm__ volatile("hlt");
        }
        return resource;
    }
}; // namespace BOOT_INFO