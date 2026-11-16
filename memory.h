/**
 * @file memory.h
 * @brief Guest physical memory allocation and little-endian access helpers.
 *
 */

#ifndef __MEMORY_H__
#define __MEMORY_H__

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

/**
 * @brief Represents the contiguous guest memory buffer.
 *
 */
typedef struct {
    uint8_t *data;
    size_t size;
} memory_t;

/**
 * @brief Allocates guest memory of the given size in bytes.
 *
 * @param mem Pointer to the memory structure to initialize.
 * @param size Number of bytes to allocate.
 *
 * @return true on success, false if allocation fails.
 *
 */
bool memory_init(memory_t *mem, size_t size);

/**
 * @brief Frees the guest memory buffer and resets its fields.
 *
 * @param mem Pointer to the memory structure to free.
 *
 */
void memory_free(memory_t *mem);

/**
 * @brief Reads a single byte from guest memory.
 *
 * @param mem Pointer to guest memory.
 * @param addr Byte address to read from.
 * @param value Output pointer for the read byte.
 *
 * @return true if the address is in bounds, false otherwise.
 *
 */
bool memory_read_byte(const memory_t *mem, size_t addr, uint8_t *value);

/**
 * @brief Writes a single byte to guest memory.
 *
 * @param mem Pointer to guest memory.
 * @param addr Byte address to write to.
 * @param value Byte value to write.
 *
 * @return true if the address is in bounds, false otherwise.
 *
 */
bool memory_write_byte(memory_t *mem, size_t addr, uint8_t value);

/**
 * @brief Reads a 16-bit halfword from guest memory.
 *
 * @param mem Pointer to guest memory.
 * @param addr Starting byte address to read from.
 * @param value Output pointer for the read halfword.
 *
 * @return true if the access is in bounds, false otherwise.
 *
 */
bool memory_read_half(const memory_t *mem, size_t addr, uint16_t *value);

/**
 * @brief Writes a 16-bit halfword to guest memory.
 *
 * @param mem Pointer to guest memory.
 * @param addr Starting byte address to write to.
 * @param value Halfword value to write.
 *
 * @return true if the access is in bounds, false otherwise.
 *
 */
bool memory_write_half(memory_t *mem, size_t addr, uint16_t value);

/**
 * @brief Reads a 32-bit word from guest memory.
 *
 * @param mem Pointer to guest memory.
 * @param addr Starting byte address to read from.
 * @param value Output pointer for the read word.
 *
 * @return true if the access is in bounds, false otherwise.
 *
 */
bool memory_read_word(const memory_t *mem, size_t addr, uint32_t *value);

/**
 * @brief Writes a 32-bit word to guest memory.
 *
 * @param mem Pointer to guest memory.
 * @param addr Starting byte address to write to.
 * @param value Word value to write.
 *
 * @return true if the access is in bounds, false otherwise.
 *
 */
bool memory_write_word(memory_t *mem, size_t addr, uint32_t value);

#endif // __MEMORY_H__
