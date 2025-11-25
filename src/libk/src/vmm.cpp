/**
 * @file vmm.cpp
 * @brief 
 * @author cbw (chenboven@qq.com)
 * @date 2025-11-13
 */


#include "cassert"
#include "cpu.h"
#include "cstdint"
// #include "cstdio"
#include "cstring"
#if defined(__i386__) || defined(__x86_64__)
#    include "gdt.h"
#endif
#include "pmm.h"
#include "vmm.h"
#include "boot_info.h"

VMM& VMM::get_instance(void) {
    /// 定义全局 VMM 对象
    static VMM vmm;
    return vmm;
}



bool VMM::init(void) {
#if defined(__i386__) || defined(__x86_64__)
    GDT::init();
#endif
    // 分配一页用于保存页目录
    pt_t pgd_kernel = (pt_t)PMM::get_instance().alloc_page_kernel();
    bzero(pgd_kernel, common::PAGE_SIZE);
    // 映射内核空间
    for (uintptr_t addr = (uintptr_t)common::KERNEL_START_ADDR;
         addr < (uintptr_t)common::KERNEL_START_ADDR + VMM_KERNEL_SPACE_SIZE;
         addr += common::PAGE_SIZE) {
        // 内核空间直接恒等映射
        mmap(pgd_kernel, addr, addr,
             VMM_PAGE_READABLE | VMM_PAGE_WRITABLE | VMM_PAGE_EXECUTABLE);
    }
    // 设置页目录
    set_pgd(pgd_kernel);
    // 开启分页
    CPU::ENABLE_PG();
    return true;
}


pte_t* VMM::find(const pt_t _pgd, uintptr_t _va, bool _alloc) {
    pt_t pgd = _pgd;
    // 从0级页表到4级页表查找
    for (size_t level = VMM_PT_LEVEL - 1; level > 0; level--) {
        // 找到 _va 的第 level 级页表项
        pte_t* pte = (pte_t*)&pgd[PX(level, _va)];
        // 页表项如果有效，获取 level+1 级页表，
        if ((*pte & VMM_PAGE_VALID) == 1) {
            pgd = (pt_t)PTE2PA(*pte);
        }
        // 如果无效
        else {
            // 是否需要分配
            if (_alloc == true) {
                // 申请新的物理页，一页里面可以放512个页表项
                pgd = (pt_t)PMM::get_instance().alloc_page_kernel();
                bzero(pgd, common::PAGE_SIZE);
                // 申请失败则返回
                if (pgd == nullptr) {
                    // 出现这种情况说明物理内存不够了，一般不会出现
                    assert(0);
                    return nullptr;
                }
                // 填充页表项
                *pte = PA2PTE((uintptr_t)pgd) | VMM_PAGE_VALID;
            }
            else {
                return nullptr;
            }
        }
    }
    // 0 最低级 pt
    return &pgd[PX(0, _va)];
}
pt_t VMM::get_pgd(void) {
    return (pt_t)CPU::GET_PGD();
}

void VMM::set_pgd(const pt_t _pgd) {
    // 设置页目录
    CPU::SET_PGD((uintptr_t)_pgd);
    // 刷新缓存
    CPU::VMM_FLUSH(0);
    return;
}

void VMM::mmap(const pt_t _pgd, uintptr_t _va, uintptr_t _pa, uint32_t _flag) {
    pte_t* pte = find(_pgd, _va, true);
    // 一般情况下不应该为空
    assert(pte != nullptr);
    // 已经映射过了 且 flag 没有变化
    if (((*pte & VMM_PAGE_VALID) == VMM_PAGE_VALID)
        && ((*pte & ((1 << VMM_PTE_PROP_BITS) - 1)) == _flag)) {
        // warn("remap.\n");
    }
    // 没有映射，或更改了 flag
    else {
        // 那么设置 *pte
        // pte 解引用后的值是页表项
        *pte = PA2PTE(_pa) | _flag | (*pte & ((1 << VMM_PTE_PROP_BITS) - 1))
             | VMM_PAGE_VALID;
        // 刷新缓存
        CPU::VMM_FLUSH(0);
    }
    return;
}

void VMM::unmmap(const pt_t _pgd, uintptr_t _va) {
    pte_t* pte = find(_pgd, _va, false);
    // 找到页表项
    // 未找到
    if (pte == nullptr) {
        warn("VMM::unmmap: find.\n");
        return;
    }
    // 找到了，但是并没有被映射
    if ((*pte & VMM_PAGE_VALID) == 0) {
        warn("VMM::unmmap: not mapped.\n");
    }
    // 置零
    *pte = 0x00;
    // 刷新缓存
    CPU::VMM_FLUSH(0);
    // TODO: 如果一页表都被 unmap，释放占用的物理内存
    return;
}

bool VMM::get_mmap(const pt_t _pgd, uintptr_t _va, const void* _pa) {
    pte_t* pte = find(_pgd, _va, false);
    bool   res = false;
    // pte 不为空且有效，说明映射了
    if ((pte != nullptr) && ((*pte & VMM_PAGE_VALID) == 1)) {
        // 如果 _pa 不为空
        if (_pa != nullptr) {
            // 设置 _pa
            // 将页表项转换为物理地址
            *(uintptr_t*)_pa = PTE2PA(*pte);
        }
        // 返回 true
        res = true;
    }
    // 否则说明没有映射
    else {
        // 如果 _pa 不为空
        if (_pa != nullptr) {
            // 设置 _pa
            *(uintptr_t*)_pa = (uintptr_t) nullptr;
        }
    }
    return res;
}
