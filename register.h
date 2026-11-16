#ifndef __REGISTER_H__
#define __REGISTER_H__

#include <stdint.h>

typedef union {
    uint32_t arr[32];
    struct {
        uint32_t x0;
        uint32_t x1;
        uint32_t x2;
        uint32_t x3;
        uint32_t x4;
        uint32_t x5;
        uint32_t x6;
        uint32_t x7;
        uint32_t x8;
        uint32_t x9;
        uint32_t x10;
        uint32_t x11;
        uint32_t x12;
        uint32_t x13;
        uint32_t x14;
        uint32_t x15;
        uint32_t x16;
        uint32_t x17;
        uint32_t x18;
        uint32_t x19;
        uint32_t x20;
        uint32_t x21;
        uint32_t x22;
        uint32_t x23;
        uint32_t x24;
        uint32_t x25;
        uint32_t x26;
        uint32_t x27;
        uint32_t x28;
        uint32_t x29;
        uint32_t x30;
        uint32_t x31;
    } regs;
} register_file_t;

#endif // __REGISTER_H__
