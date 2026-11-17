/**
 * @file print.h
 * @brief Minimal freestanding output routines using the UART device.
 *
 */

#pragma once

#include <stdint.h>

/**
 * @brief Writes a buffer of characters to standard output.
 *
 * @param buf Pointer to the character buffer.
 * @param len Number of bytes to write.
 *
 */
void puts(const char *buf, uint32_t len);

/**
 * @brief Writes a single character to standard output.
 *
 * @param c Character to write.
 *
 */
void putchar(char c);

/**
 * @brief Prints formatted output to standard output.
 *
 * @param fmt Format string.
 *
 */
void printf(const char *fmt, ...);
