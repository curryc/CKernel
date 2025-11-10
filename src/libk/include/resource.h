/**
 * @file resource.h
 * @brief 定义内核的各种资源
 * @author cbw (chenboven@qq.com)
 * @date 2025-10-11
 */

#ifndef CKERNEL_RESOURCE_H
#define CKERNEL_RESOURCE_H

#include "cstdint"
#include "cassert"
#include "cstddef"
#include "iostream"

struct resource_t
{
    /// 资源类型
    enum : uint8_t
    {
        /// 内存
        MEM = 1 << 0,
        /// 中断号
        INTR_NO = 1 << 1,
    };

    uint8_t type;
    /// 资源名称
    char *name;

    /// 内存信息
    struct
    {
        uintptr_t addr;
        size_t len;
    } mem;

    /// 中断号
    uint8_t intr_no;

    resource_t(void)
    {
        type = 0;
        name = nullptr;
        mem.addr = 0;
        mem.len = 0;
        intr_no = 0;
    }

    /**
     * @brief resource_t 输出
     * @param  _os             输出流
     * @param  _res            要输出的 resource_t
     * @return std::ostream&   输出流
     */
    // friend std::ostream &operator<<(std::ostream &_os, const resource_t &_res)
    // {
    //     _os << _res.name << ": ";
    //     if (_res.type & MEM)
    //     {
    //         _os << "MEM(0x" << std::hex << _res.mem.addr << ", 0x" << _res.mem.len << std::dec << ")";
    //     }
    //     if (_res.type & INTR_NO)
    //     {
    //         _os << ", INTR_NO(0x" << std::hex << (int)_res.intr_no << std::dec << ")";
    //     }
    //     return _os;
    // }
};

#endif