/**
 * @file multiboot2.cpp
 * @brief
 * @author cbw (chenboven@qq.com)
 * @date 2025-10-11
 */

#include "multiboot2.h"
#include "cassert"
#include "common.h"
#include "vgaprint.h"

MULTIBOOT2 &MULTIBOOT2::get_instance(void)
{
    static MULTIBOOT2 instance;
    return instance;
}

bool MULTIBOOT2::multiboot2_init(void)
{
    info("multiboot2_init: Starting initialization...\n");
    info("multiboot2_init: Magic number: 0x%lx\n", BOOT_INFO::multiboot2_magic);
    uintptr_t addr = BOOT_INFO::boot_info_addr;
    info("multiboot2_init: Boot info addr: 0x%lx\n", addr);
    
    // 判断魔数是否正确
    if (BOOT_INFO::multiboot2_magic != MULTIBOOT2_BOOTLOADER_MAGIC) {
        err("multiboot2_init: Invalid magic number! Expected: 0x%x, Got: 0x%lx\n", 
                   MULTIBOOT2_BOOTLOADER_MAGIC, BOOT_INFO::multiboot2_magic);
        return false;
    }
    
    // 修复地址对齐检查
    if ((addr & 7) != 0) {
        err("multiboot2_init: Address not aligned! addr=0x%lx\n", addr);
        return false;
    }
    
    // 空指针检查
    if (addr == 0) {
        err("multiboot2_init: Boot info address is NULL!\n");
        return false;
    }
    
    // 检查地址是否在合理的内存范围内
    if (addr < 0x100000 || addr > 0x1000000) {  // 1MB到16MB之间
        err("multiboot2_init: Boot info address out of range! addr=0x%lx\n", addr);
        return false;
    }
    
    info("multiboot2_init: Reading boot info size...\n");
    
    // 内存读取 - 逐字节读取
    uint32_t size = 0;
    uint8_t* size_ptr = (uint8_t*)addr;
    for (int i = 0; i < 4; i++) {
        size |= (uint32_t)(size_ptr[i]) << (i * 8);
    }
    
    BOOT_INFO::boot_info_size = size;
    info("multiboot2_init: Boot info size: %d bytes\n", BOOT_INFO::boot_info_size);
    info("multiboot2_init: Boot info end: 0x%lx\n", BOOT_INFO::boot_info_addr + BOOT_INFO::boot_info_size);
    
    // 验证大小是否合理
    if (BOOT_INFO::boot_info_size < 8 || BOOT_INFO::boot_info_size > 65536) {
        info("multiboot2_init: Boot info size unreasonable: %d\n", BOOT_INFO::boot_info_size);
        return false;
    }
    
    info("multiboot2_init: Initialization successful!\n");
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

    info("multiboot2_get_memory: Memory tag found\n");

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

    info("multiboot2_get_memory: Tag size: %d, Entry size: %d\n", tag_size, entry_size);

    // 验证基本参数
    if (entry_size == 0 || tag_size < sizeof(multiboot_tag_mmap_t)) {
        err("multiboot2_get_memory: Invalid mmap tag parameters\n");
        return false;
    }

    // 指向第一个内存映射项
    MULTIBOOT2::multiboot_mmap_entry_t *mmap = (MULTIBOOT2::multiboot_mmap_entry_t *)
        ((uint8_t*)mmap_tag + sizeof(multiboot_tag_mmap_t));

    // 计算实际可以访问的条目数量
    size_t max_entries = (tag_size - sizeof(multiboot_tag_mmap_t)) / entry_size;

    // 遍历所有内存映射项 - 使用更安全的遍历条件
    for (size_t i = 0; i < max_entries; i++) {
        // 检查是否还有足够的空间读取完整的条目
        if ((uint8_t*)mmap + sizeof(MULTIBOOT2::multiboot_mmap_entry_t) > tag_end) {
            err("multiboot2_get_memory: Reached tag boundary, stopping\n");
            break;
        }

        // 验证内存区域的有效性
        if (mmap->len == 0) {
            mmap = (multiboot_mmap_entry_t*)((uint8_t*)mmap + entry_size);
            continue;
        }

        // 如果是可用内存
        if (mmap->type == MULTIBOOT2::MULTIBOOT_MEMORY_AVAILABLE) {
            // 累加可用内存大小
            resource->mem.len += mmap->len;
        }
        // 移动到下一个内存映射项
        mmap = (multiboot_mmap_entry_t*)((uint8_t*)mmap + entry_size);
    }

    info("multiboot2_get_memory: Total available memory: %u bytes\n", resource->mem.len);
    
    return true;
}



bool MULTIBOOT2::get_framebuffer(const iter_data_t *_iter_data, void *_data)
{
    if (_iter_data->type != MULTIBOOT2::MULTIBOOT_TAG_TYPE_FRAMEBUFFER) {
        return false;
    }

    // 设置资源类型和名称
    resource_t *resource = (resource_t *)_data;
    resource->type |= resource_t::FRAMEBUFFER;
    resource->name = (char *)"Framebuffer";
    resource->mem.addr = 0x0;
    resource->mem.len = 0;

    // 获取 ACPI 标签
    multiboot_tag_framebuffer *framebuffer_tag = (multiboot_tag_framebuffer *)_iter_data;

    resource->fb_info_t.base = framebuffer_tag->framebuffer_addr;
    resource->fb_info_t.width = framebuffer_tag->framebuffer_width;
    resource->fb_info_t.height = framebuffer_tag->framebuffer_height;
    resource->fb_info_t.pitch = framebuffer_tag->framebuffer_pitch;
    resource->fb_info_t.bpp = framebuffer_tag->framebuffer_bpp;

    /* 计算缓冲区总大小 */
    resource->fb_info_t.size = resource->fb_info_t.pitch * resource->fb_info_t.height;

    /* 如果是文本模式（bpp=16 且 addr=0xB8000） */
    if (resource->fb_info_t.bpp == 16 && resource->fb_info_t.base == 0xB8000)
    {
        resource->fb_info_t.cols = 80; // 标准 VGA 文本
        resource->fb_info_t.rows = 25;
    }
    else
    { // 图形模式，按 8×16 字体估算
        resource->fb_info_t.cols = resource->fb_info_t.width / 8;
        resource->fb_info_t.rows = resource->fb_info_t.height / 16;
    }

    return true;
}

bool MULTIBOOT2::get_acpi(const iter_data_t *_iter_data, void *_data)
{
    if (_iter_data->type != MULTIBOOT2::MULTIBOOT_TAG_TYPE_ACPI_NEW &&
        _iter_data->type != MULTIBOOT2::MULTIBOOT_TAG_TYPE_ACPI_OLD) {
        return false;
    }

    // 设置资源类型和名称
    resource_t *resource = (resource_t *)_data;
    resource->type |= resource_t::ACPI;
    resource->name = (char *)"ACPI Tables";
    resource->mem.addr = 0x0;
    resource->mem.len = 0;

    // 获取 ACPI 标签
    multiboot_tag_old_acpi_t *acpi_old_tag = nullptr;
    multiboot_tag_new_acpi_t *acpi_new_tag = nullptr;

    if (_iter_data->type == MULTIBOOT2::MULTIBOOT_TAG_TYPE_ACPI_NEW) {
        acpi_new_tag = (multiboot_tag_new_acpi_t *)_iter_data;
        resource->acpi.rsdp = (uintptr_t)acpi_new_tag->rsdp;
        // info("multiboot2_get_acpi: Found RSDP (new): 0x%lx\n", resource->acpi.rsdp);
    } else if (_iter_data->type == MULTIBOOT2::MULTIBOOT_TAG_TYPE_ACPI_OLD) {
        acpi_old_tag = (multiboot_tag_old_acpi_t *)_iter_data;
        resource->acpi.rsdp = (uintptr_t)acpi_old_tag->rsdp;
        // info("multiboot2_get_acpi: Found RSDP (old): 0x%lx\n", resource->acpi.rsdp);
    }

    // 验证 RSDP 地址是否有效
    if (resource->acpi.rsdp == 0) {
        err("multiboot2_get_acpi: Invalid RSDP address\n");
        return false;
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
        }
        return res;
    }

    resource_t get_memory(void)
    {
        if (!BOOT_INFO::inited) {
            info("BOOT_INFO not inited.\n");
            resource_t empty_resource;
            return empty_resource;
        }
        
        resource_t resource;
        
        MULTIBOOT2::get_instance().multiboot2_iter(MULTIBOOT2::get_memory,
                                                   &resource);
        return resource;
    }


    resource_t get_acpi(void)
    {
        if (!BOOT_INFO::inited) {
            info("BOOT_INFO not inited.\n");
            resource_t empty_resource;
            return empty_resource;
        }
        
        resource_t resource;
        
        MULTIBOOT2::get_instance().multiboot2_iter(MULTIBOOT2::get_acpi,
                                                   &resource);
        return resource;
    }

    resource_t get_framebuffer(void){
        if (!BOOT_INFO::inited) {
            info("BOOT_INFO not inited.\n");
            resource_t empty_resource;
            return empty_resource;
        }
        
        resource_t resource;
        
        MULTIBOOT2::get_instance().multiboot2_iter(MULTIBOOT2::get_framebuffer,
                                                   &resource);
        return resource;
    }
}; // namespace BOOT_INFO
