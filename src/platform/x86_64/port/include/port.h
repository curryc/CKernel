/**
 * @file port.h
 * @brief 在x86_64中实现端口读写
 * @author curryc (chencurry5@gmail.com)
 * @date 2025-09-29
 */

#ifndef CKERNEL_PORT_H
#define CKERNEL_PORT_H

typedef unsigned char uint8_t;
typedef unsigned short int uint16_t;
typedef unsigned int uint32_t;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief  读一个字节
 * @param  _port           要读的端口
 * @return uint8_t         读取到的数据
 * @warning 不处理执行失败的问题
 */
uint8_t port_inb(const uint32_t _port);

/**
 * @brief  读一个字
 * @param  _port           要读的端口
 * @return uint16_t        读取到的数据
 * @warning 不处理执行失败的问题
 */
uint16_t port_inw(const uint32_t _port);

/**
 * @brief  读一个双字
 * @param  _port           要读的端口
 * @return uint32_t        读取到的数据
 * @warning 不处理执行失败的问题
 */
uint32_t port_ind(const uint32_t _port);

/**
 * @brief  写一个字节
 * @param  _port           要写的端口
 * @param  _data           要写的数据
 * @warning 不处理执行失败的问题
 */
void port_outb(const uint32_t _port, const uint8_t _data);

/**
 * @brief  写一个字
 * @param  _port           要写的端口
 * @param  _data           要写的数据
 * @warning 不处理执行失败的问题
 */
void port_outw(const uint32_t _port, const uint16_t _data);

/**
 * @brief  写一个双字
 * @param  _port           要写的端口
 * @param  _data           要写的数据
 * @warning 不处理执行失败的问题
 */
void port_outd(const uint32_t _port, const uint32_t _data);

#ifdef __cplusplus
} // extern "C"

// 为C++提供namespace封装
namespace port
{
    inline uint8_t inb(const uint32_t _port) { return port_inb(_port); }
    inline uint16_t inw(const uint32_t _port) { return port_inw(_port); }
    inline uint32_t ind(const uint32_t _port) { return port_ind(_port); }
    inline void outb(const uint32_t _port, const uint8_t _data) { port_outb(_port, _data); }
    inline void outw(const uint32_t _port, const uint16_t _data) { port_outw(_port, _data); }
    inline void outd(const uint32_t _port, const uint32_t _data) { port_outd(_port, _data); }
}
#endif

#endif