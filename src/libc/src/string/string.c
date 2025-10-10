#include "string.h"

void* memset(void* _dest, int _val, size_t _len) {
    unsigned char* ptr = _dest;
    while (_len-- > 0) {
        *ptr++ = _val;
    }
    return _dest;
}

void bzero(void* _dest, size_t _len) {
    memset(_dest, 0, _len);
}

/**
 * @brief 字符串长度
 * @param  _str
 * @return size_t 
 */
size_t strlen(const char* _str) {
    size_t len = 0;
    while (_str[len]) {
        len++;
    }
    return len;
}