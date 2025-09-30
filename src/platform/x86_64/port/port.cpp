#include "port.h"

/* 8-bit */
uint8_t port::inb(uint16_t __port) {
    uint8_t __val;
    __asm__ volatile ("inb %w1, %b0"
                    : "=a"(__val)
                    : "Nd"(__port));
    return __val;
}

void port::outb(uint16_t __port, uint8_t __val) {
    __asm__ volatile ("outb %b0, %w1"
                    :
                    : "a"(__val), "Nd"(__port));
}

/* 16-bit */
uint16_t port::inw(uint16_t __port) {
    uint16_t __val;
    __asm__ volatile ("inw %w1, %w0"
                    : "=a"(__val)
                    : "Nd"(__port));
    return __val;
}

void port::outw(uint16_t __port, uint16_t __val) {
    __asm__ volatile ("outw %w0, %w1"
                    :
                    : "a"(__val), "Nd"(__port));
}

/* 32-bit */
uint32_t port::ind(uint16_t __port) {
    uint32_t __val;
    __asm__ volatile ("inl %w1, %k0"
                    : "=a"(__val)
                    : "Nd"(__port));
    return __val;
}

void port::outd(uint16_t __port, uint32_t __val) {
    __asm__ volatile ("outl %k0, %w1"
                    :
                    : "a"(__val), "Nd"(__port));
}