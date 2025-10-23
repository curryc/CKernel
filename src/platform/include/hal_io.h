/**
 * @brief 统一的输入输出头
 * @param  addr
 * @return uint8_t 
 */

 #ifndef CKERNEL_HAL_IO_H
#define CKERNEL_HAL_IO_H

// 使用标准整数类型
#include <stdint.h>

/**
 * @brief  读一个字节
 * @param  _port           要读的端口
 * @return uint8_t         读取到的数据
 * @warning 不处理执行失败的问题
 */
uint8_t  port_inb(const uint32_t addr);
/**
 * @brief  写一个字节
 * @param  _port           要写的端口
 * @param  _data           要写的数据
 * @warning 不处理执行失败的问题
 */
void     port_outb(const uint32_t addr, uint8_t val);
/**
 * @brief  读一个字
 * @param  _port           要读的端口
 * @return uint16_t        读取到的数据
 * @warning 不处理执行失败的问题
 */
uint16_t port_inw(const uint32_t addr);
/**
 * @brief  写一个字
 * @param  _port           要写的端口
 * @param  _data           要写的数据
 * @warning 不处理执行失败的问题
 */
void     port_outw(const uint32_t addr, uint16_t val);
/**
 * @brief  读一个双字
 * @param  _port           要读的端口
 * @return uint32_t        读取到的数据
 * @warning 不处理执行失败的问题
 */
uint32_t port_ind(const uint32_t addr);
/**
 * @brief  写一个双字
 * @param  _port           要写的端口
 * @param  _data           要写的数据
 * @warning 不处理执行失败的问题
 */
void     port_outd(const uint32_t addr, uint32_t val);

#endif