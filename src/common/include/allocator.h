/**
 * @file allocator.h
 * @brief
 * @author cbw (chenboven@qq.com)
 * @date 2025-10-13
 */

#ifndef CKERNEL_ALLOCATOR_H
#define CKERNEL_ALLOCATOR_H

#include "cstddef"
#include "cstdint"

/**
 * @brief 内存分配器抽象类
 */
class ALLOCATOR
{
private:
protected:
    /// 分配器名称
    const char *tag;
    /// 当前管理的内存区域地址
    uintptr_t allocator_start_addr;
    /// 当前管理的内存区域长度
    size_t allocator_length;
    /// 当前管理的内存区域空闲长度
    size_t allocator_free_count;
    /// 当前管理的内存区域已使用长度
    size_t allocator_used_count;

public:
    /**
     * @brief 构造内存分配器
     * @param  _tag           分配器名
     * @param  _addr           要管理的内存开始地址
     * @param  _len            要管理的内存长度，单位以具体实现为准
     */
    ALLOCATOR(const char* _tag, unsigned long _addr, unsigned long _len);
    virtual ~ALLOCATOR(void);

    /**
     * @brief 分配 _len 页
     * @param  _len            页数
     * @return uintptr_t       分配到的地址
     */
    virtual uintptr_t alloc(size_t _len) = 0;

    /**
     * @brief 在指定地址分配 _len 长度
     * @param  _addr           指定的地址
     * @param  _len            长度
     * @return true            成功
     * @return false           失败
     */
    virtual bool alloc(uintptr_t _addr, size_t _len) = 0;

    /**
     * @brief 释放 _len 长度
     * @param  _addr           地址
     * @param  _len            长度
     */
    virtual void free(uintptr_t _addr, size_t _len) = 0;

    /**
     * @brief 已使用数量
     * @return size_t          数量
     */
    virtual size_t get_used_count(void) const = 0;

    /**
     * @brief 空闲数量
     * @return size_t          数量
     */
    virtual size_t get_free_count(void) const = 0;
};

#endif