/**
 * @file register.h
 * @brief RISC-V 32-entry general-purpose register file definition.
 *
 */

#ifndef __REGISTER_H__
#define __REGISTER_H__

#include <stdint.h>

/**
 * @brief General-purpose register file accessible as an array or ABI-named fields (zero..t6).
 *
 */
typedef union {
    uint32_t arr[32];
    /**
     * @brief ABI-named 32-bit general-purpose registers (x0..x31).
     *
     */
    struct {
        uint32_t zero;
        uint32_t ra;
        uint32_t sp;
        uint32_t gp;
        uint32_t tp;
        uint32_t t0;
        uint32_t t1;
        uint32_t t2;
        uint32_t s0;
        uint32_t s1;
        uint32_t a0;
        uint32_t a1;
        uint32_t a2;
        uint32_t a3;
        uint32_t a4;
        uint32_t a5;
        uint32_t a6;
        uint32_t a7;
        uint32_t s2;
        uint32_t s3;
        uint32_t s4;
        uint32_t s5;
        uint32_t s6;
        uint32_t s7;
        uint32_t s8;
        uint32_t s9;
        uint32_t s10;
        uint32_t s11;
        uint32_t t3;
        uint32_t t4;
        uint32_t t5;
        uint32_t t6;
    } abi;
} register_file_t;

#endif // __REGISTER_H__
