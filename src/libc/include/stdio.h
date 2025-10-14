/**
 * @file stdio.h
 * @brief 包含一些io函数，并不全部实现
 * @author curryc (chencurry5@gmail.com)
 * @date 2025-09-29
 */

#ifndef CKERNEL_STDIO_H
#define CKERNEL_STDIO_H

#include "stdarg.h"
#include "stddef.h"
#include "stdint.h"

#ifdef __cplusplus
extern "C"
{
#endif

    int _vsnprintf(char *_buffer, const size_t _maxlen, const char *_format,
                   va_list _va);


    /**
     * @brief 输出格式化字符串
     * @param  _fmt
     * @param  ...
     * @return int32_t
     */
    int32_t printf(const char *_fmt, ...);
    /**
     * @brief 输入格式化字符串，颜色为青色
     * @param  _fmt
     * @param  ...
     * @return int32_t
     */
    int32_t info(const char *_fmt, ...);
    /**
     * @brief 输入格式化字符串，颜色为黄色
     * @param  _fmt
     * @param  ...
     * @return int32_t
     */
    int32_t warn(const char *_fmt, ...);
    /**
     * @brief 输入格式化字符串，颜色为红色
     * @param  _fmt
     * @param  ...
     * @return int32_t
     */
    int32_t err(const char *_fmt, ...);
#ifdef __cplusplus
}
#endif

#endif