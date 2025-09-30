#include "string.h"

void* memset(void* _dest, int _val, size_t _len) {
    unsigned char* ptr = _dest;
    while (_len-- > 0) {
        *ptr++ = _val;
    }
    return _dest;
}

void bzero(void* _dest, uint32_t _len) {
    memset(_dest, 0, _len);
}