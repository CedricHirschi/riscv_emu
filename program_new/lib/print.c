// Copyright 2024 ETH Zurich and University of Bologna.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0
//
// Philippe Sauter <phsauter@iis.ee.ethz.ch>

#include "print.h"

#include <stdint.h>
#include <stdarg.h>

const char hex_symbols[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

/// @brief format number as hexadecimal digits
/// @return number of characters written to buffer
uint8_t format_hex32(char *buffer, uint32_t num) {
    uint8_t idx = 0;
    if (num == 0) {
        buffer[0] = hex_symbols[0];
        return 1;
    }

    while (num > 0) {
        buffer[idx++] = hex_symbols[num & 0xF];
        num >>= 4;
    }
    return idx;
}

void puts(const char *buf, uint32_t len)
{
    register uint32_t a0 __asm__("a0") = 1;
    register const char *a1 __asm__("a1") = buf;
    register uint32_t a2 __asm__("a2") = len;
    register uint32_t a7 __asm__("a7") = 64;

    __asm__ volatile("ecall"
        : "+r"(a0)
        : "r"(a1), "r"(a2), "r"(a7)
        : "memory");
}

void putchar(char c) {
    puts(&c, 1);
}

void printf(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);

    while (*fmt) {
        if (*fmt == '%') {
            fmt++;
            if (*fmt == 'x') { // hex
                unsigned int hex = va_arg(args, unsigned int);
                char buffer[11];  // holds string while assembling
                unsigned int i = 0;
                
                if (hex == 0) {
                    putchar('0');
                } else {
                    while (hex > 0) {
                        buffer[i++] = (hex & 0xF) + (((hex & 0xF) <= 9) ? '0' : ('A' - 10));
                        hex >>= 4;
                    }
                    // print from stack
                    for (int j = i - 1; j >= 0; j--) {
                        putchar(buffer[j]);
                    }
                }
            } else if (*fmt == 'c') { // char
                char chr = (char) va_arg(args, int);
                putchar(chr);
            } else if (*fmt == 's') { // string
                char *str = va_arg(args, char *);
                while(*str) {
                    putchar(*str++);
                }
            } else if (*fmt == 'd') { // decimal
                int dec = va_arg(args, int);
                if (dec < 0) {
                    putchar('-');
                    dec = -dec;
                }
                char buffer[11];
                int i = 0;
                if (dec == 0) {
                    putchar('0');
                } else {
                    while (dec > 0) {
                        buffer[i++] = (dec % 10) + '0';
                        dec /= 10;
                    }
                    for (int j = i - 1; j >= 0; j--) {
                        putchar(buffer[j]);
                    }
                }
            } else {
                putchar('%');
            }
        } else {
            putchar(*fmt);
        }
        fmt++;
    }

    va_end(args);
}
