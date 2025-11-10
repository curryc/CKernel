/**
 * @file color.h
 * @brief
 * @author curryc (chencurry5@gmail.com)
 * @date 2025-09-29
 */

#ifndef CKERNEL_COLOR_H
#define CKERNEL_COLOR_H

#include "cstdint"

/**
 * @brief 定义不同平台的颜色
 */
namespace COLOR
{
    typedef enum : uint8_t
    {
#if defined(__i386__) || defined(__x86_64__)
        /// RGB: 0 0 0
        BLACK = 0,
        /// RGB: 0 0 170
        BLUE = 1,
        /// RGB: 0 170 0
        GREEN = 2,
        /// RGB: 0 170 170
        CYAN = 3,
        /// RGB: 170 0 0
        RED = 4,
        /// RGB: 170 0 170
        PURPLE = 5,
        /// RGB: 170 85 0
        BROWN = 6,
        /// RGB: 170 170 170
        GREY = 7,
        /// RGB: 85 85 85
        DARK_GREY = 8,
        /// RGB: 85 85 255
        LIGHT_BLUE = 9,
        /// RGB: 85 255 85
        LIGHT_GREEN = 10,
        /// RGB: 85 255 255
        LIGHT_CYAN = 11,
        /// RGB: 255 85 85
        LIGHT_RED = 12,
        /// RGB: 255 85 255
        LIGHT_PURPLE = 13,
        /// RGB: 255 255 85
        YELLOW = 14,
        /// RGB: 255 255 255
        WHITE = 15,
    } color_t;
}; // namespace COLOR

#endif

#endif