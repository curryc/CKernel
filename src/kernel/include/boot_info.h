/**
 * @file boot_info.h
 * @brief
 * @author cbw (chenboven@qq.com)
 * @date 2025-10-11
 */

#ifndef CKERNEL_BOOT_INFO_H
#define CKERNEL_BOOT_INFO_H

#include "resource.h"
#include "cstdint"

/**
 * @brief 用来承接multiboot2传递的硬件信息
 */
namespace BOOT_INFO
{
    /// 仅仅是声明，定义在具体的实现中

    /// 是否已经初始化过
    extern bool inited;
    /// 地址
    extern "C" uintptr_t boot_info_addr;
    /// 长度
    extern size_t boot_info_size;
    /// 保存 sbi 传递的启动核
    extern "C" size_t dtb_init_hart;
    /// 魔数
    extern "C" uint32_t multiboot2_magic;

    /**
     * @brief 初始化，定义在具体实现中
     * @return true            成功
     * @return false           成功
     */
    extern bool init(void);

    /**
     * @brief 获取物理内存信息
     * @return resource_t      物理内存资源信息
     */
    extern resource_t get_memory(void);

    /**
     * @brief 获取 clint 信息
     * @return resource_t       clint 资源信息
     */
    extern resource_t get_clint(void);

    /**
     * @brief 获取 plic 信息
     * @return resource_t       plic 资源信息
     */
    extern resource_t get_plic(void);
}; // namespace BOOT_INFO
#endif