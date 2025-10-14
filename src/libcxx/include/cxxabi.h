/**
 * @file cxxabi.h
 * @brief 用于C++基础需求，比如__cxa_atexit用于类的析构，这些函数会被C++隐式调用
 * @author cbw (chenboven@qq.com)
 * @date 2025-10-09
 */

#ifndef CKERNEL_CXXABI_H
#define CKERNEL_CXXABI_H

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @brief C++初始化函数，会被入口汇编调用，用于初始化Cpp环境
     * 遍历__init_array_start到__init_array_end之间的函数指针数组
     * 依次调用每个构造函数，完成全局/静态对象的构造
     */
    void cpp_init(void);

    int __cxa_atexit(void (*f)(void *), void *objptr, void *dso);
    void __cxa_finalize(void *f);

    int __aeabi_atexit(void (*f)(void *), void *objptr, void *dso);
    void __cxa_finalize(void *f);

#if UINT32_MAX == UINTPTR_MAX
#define STACK_CHK_GUARD 0xe2dee396
#else
#define STACK_CHK_GUARD 0x595e9fbd94fda766
#endif

#ifdef __cplusplus
};
#endif

namespace std
{
    class type_info
    {
    private:
        const char *tname;

    public:
        virtual ~type_info(void);

        type_info(const type_info &);

        explicit type_info(const char *);

        const char *name(void) const;

        bool operator==(const type_info &) const;

        bool operator!=(const type_info &) const;
    };
} // namespace std
namespace __cxxabiv1 {

#define ADD_CXX_TYPEINFO_HEADER(t)    \
    class t : public std::type_info { \
    private:                          \
                                      \
    protected:                        \
                                      \
    public:                           \
        explicit t(const char*);      \
        virtual ~t(void);             \
    }

ADD_CXX_TYPEINFO_HEADER(__fundamental_type_info);
ADD_CXX_TYPEINFO_HEADER(__array_type_info);
ADD_CXX_TYPEINFO_HEADER(__function_type_info);
ADD_CXX_TYPEINFO_HEADER(__enum_type_info);
ADD_CXX_TYPEINFO_HEADER(__pbase_type_info);
ADD_CXX_TYPEINFO_HEADER(__pointer_type_info);
ADD_CXX_TYPEINFO_HEADER(__pointer_to_member_type_info);
ADD_CXX_TYPEINFO_HEADER(__class_type_info);
ADD_CXX_TYPEINFO_HEADER(__si_class_type_info);
ADD_CXX_TYPEINFO_HEADER(__vmi_class_type_info);

#undef ADD_CXX_TYPEINFO_HEADER
}    // namespace __cxxabiv1
#endif
