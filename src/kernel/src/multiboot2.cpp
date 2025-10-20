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
    vga_printf("multiboot2_init: Starting initialization...\n");
    vga_printf("multiboot2_init: Magic number: 0x%lx\n", BOOT_INFO::multiboot2_magic);
    uintptr_t addr = BOOT_INFO::boot_info_addr;
    vga_printf("multiboot2_init: Boot info addr: 0x%lx\n", addr);
    
    // 判断魔数是否正确
    if (BOOT_INFO::multiboot2_magic != MULTIBOOT2_BOOTLOADER_MAGIC) {
        vga_printf("multiboot2_init: Invalid magic number! Expected: 0x%x, Got: 0x%lx\n", 
                   MULTIBOOT2_BOOTLOADER_MAGIC, BOOT_INFO::multiboot2_magic);
        return false;
    }
    
    // 修复地址对齐检查 - addr已经是uintptr_t，不需要reinterpret_cast
    if ((addr & 7) != 0) {
        vga_printf("multiboot2_init: Address not aligned! addr=0x%lx\n", addr);
        return false;
    }
    
    // 添加空指针检查
    if (addr == 0) {
        vga_printf("multiboot2_init: Boot info address is NULL!\n");
        return false;
    }
    
    // 检查地址是否在合理的内存范围内
    if (addr < 0x100000 || addr > 0x1000000) {  // 1MB到16MB之间
        vga_printf("multiboot2_init: Boot info address out of range! addr=0x%lx\n", addr);
        return false;
    }
    
    vga_printf("multiboot2_init: Reading boot info size...\n");
    
    // 更安全的内存读取 - 逐字节读取
    uint32_t size = 0;
    uint8_t* size_ptr = (uint8_t*)addr;
    for (int i = 0; i < 4; i++) {
        size |= (uint32_t)(size_ptr[i]) << (i * 8);
    }
    
    BOOT_INFO::boot_info_size = size;
    vga_printf("multiboot2_init: Boot info size: %d bytes\n", BOOT_INFO::boot_info_size);
    vga_printf("multiboot2_init: Boot info end: 0x%lx\n", BOOT_INFO::boot_info_addr + BOOT_INFO::boot_info_size);
    
    // 验证大小是否合理
    if (BOOT_INFO::boot_info_size < 8 || BOOT_INFO::boot_info_size > 65536) {
        vga_printf("multiboot2_init: Boot info size unreasonable: %d\n", BOOT_INFO::boot_info_size);
        return false;
    }
    
    vga_printf("multiboot2_init: Initialization successful!\n");
    return true;
}

void MULTIBOOT2::multiboot2_iter(bool (*_fun)(const iter_data_t*, void*),
                                 void* _data) {
    uintptr_t    addr = BOOT_INFO::boot_info_addr;
    // 下一字节开始为 tag 信息
    iter_data_t* tag  = (iter_data_t*)(addr + 8);
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

    uint32_t tag_size = mmap_tag->size;
    uint8_t *tag_end = (uint8_t*)mmap_tag + tag_size;
    uint32_t entry_size = mmap_tag->entry_size;

    vga_printf("Tag size: %d, Entry size: %d\n", tag_size, entry_size);

    while (1)
    {
        __asm__ volatile("hlt");
    }
    // 验证基本参数
    if (entry_size == 0 || tag_size < sizeof(multiboot_tag_mmap_t)) {
        vga_printf("Invalid mmap tag parameters\n");
        return false;
    }

    // 指向第一个内存映射项
    // entries是一个灵活数组成员，需要正确计算偏移
    MULTIBOOT2::multiboot_mmap_entry_t *mmap = (MULTIBOOT2::multiboot_mmap_entry_t *)
        ((uint8_t*)mmap_tag + sizeof(multiboot_tag_mmap_t));

    // 计算实际可以访问的条目数量
    size_t max_entries = (tag_size - sizeof(multiboot_tag_mmap_t)) / entry_size;
    vga_printf("Max entries: %zu\n", max_entries);

    // 遍历所有内存映射项 - 使用更安全的遍历条件
    for (size_t i = 0; i < max_entries; i++) {
        // 检查是否还有足够的空间读取完整的条目
        if ((uint8_t*)mmap + sizeof(MULTIBOOT2::multiboot_mmap_entry_t) > tag_end) {
            vga_printf("Reached tag boundary, stopping\n");
            break;
        }

        vga_printf("Entry %zu: Base addr: 0x%lx, Length: 0x%lx, Type: %u\n", 
                   i, mmap->addr, mmap->len, mmap->type);

        // 验证内存区域的有效性
        if (mmap->len == 0) {
            vga_printf("  -> Skipping zero-length region\n");
            mmap = (multiboot_mmap_entry_t*)((uint8_t*)mmap + entry_size);
            continue;
        }

        // 如果是可用内存
        if (mmap->type == MULTIBOOT_MEMORY_AVAILABLE) {
            // 累加可用内存大小
            resource->mem.len += mmap->len;
            vga_printf("  -> Added to available memory\n");
        }
        // 移动到下一个内存映射项
        mmap = (multiboot_mmap_entry_t*)((uint8_t*)mmap + entry_size);
    }

    vga_printf("Total available memory: %zu bytes\n", resource->mem.len);
    
    // 移除无限循环，让函数正常返回
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
        return resource;
    }
}; // namespace BOOT_INFO
