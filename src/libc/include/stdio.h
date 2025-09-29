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

#ifdef __cplusplus
}
#endif

#endif