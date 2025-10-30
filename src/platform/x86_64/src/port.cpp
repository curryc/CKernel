#include "port.h"

/* 8-bit */
uint8_t PORT::port_inb(const uint32_t _port)
{
    uint8_t __val;
    __asm__ volatile("inb %w1, %b0"
                     : "=a"(__val)
                     : "Nd"(_port));
    return __val;
}

void PORT::port_outb(const uint32_t _port, const uint8_t _data)
{
    __asm__ volatile("outb %b0, %w1"
                     :
                     : "a"(_data), "Nd"(_port));
}

/* 16-bit */
uint16_t PORT::port_inw(const uint32_t _port)
{
    uint16_t __val;
    __asm__ volatile("inw %w1, %w0"
                     : "=a"(__val)
                     : "Nd"(_port));
    return __val;
}

void PORT::port_outw(const uint32_t _port, const uint16_t _data)
{
    __asm__ volatile("outw %w0, %w1"
                     :
                     : "a"(_data), "Nd"(_port));
}

/* 32-bit */
uint32_t PORT::port_ind(const uint32_t _port)
{
    uint32_t __val;
    __asm__ volatile("inl %w1, %k0"
                     : "=a"(__val)
                     : "Nd"(_port));
    return __val;
}

void PORT::port_outd(const uint32_t _port, const uint32_t _data)
{
    __asm__ volatile("outl %k0, %w1"
                     :
                     : "a"(_data), "Nd"(_port));
}