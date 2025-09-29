/**
 * @file port.h
 * @brief 在x86_64中实现端口读写
 * @author curryc (chencurry5@gmail.com)
 * @date 2025-09-29
 */
#pragma once
#include <cstdint>
#include <cstddef>

namespace port {

/**
 * @brief inb 8个字节的端口读
 * @param  __port           
 * @return uint8_t 
 */
static inline uint8_t inb(uint16_t __port);

/**
 * @brief outb 8个字节的端口写
 * @param  __port
 * @param  __val
 */
static inline void outb(uint16_t __port, uint8_t __val);


/**
 * @brief inw 16个字字节的端口读
 * @param  __port
 * @return uint16_t 
 */
static inline uint16_t inw(uint16_t __port) ;

/**
 * @brief outw 16个字字节的端口写
 * @param  __port
 * @param  __val
 */
static inline void outw(uint16_t __port, uint16_t __val);

/**
 * @brief inl 32个字字节的端口读
 * @param  __port
 * @return uint32_t 
 */
static inline uint32_t ind(uint16_t __port);

/**
 * @brief outl 32个字字字节的端口写
 * @param  __port
 * @param  __val
 */
static inline void outd(uint16_t __port, uint32_t __val);

} // namespace port