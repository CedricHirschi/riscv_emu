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
 * @brief Base addresses for ROM (Read-Only Memory)
 * 
 */
#define MEMORY_ROM_BASE  0x00000000u
/**
 * @brief Base address for MMIO (Memory-Mapped I/O) region.
 *
 */
#define MEMORY_MMIO_BASE 0x10000000u
/**
 * @brief Base address for RAM (Random Access Memory) region.
 *
 */
#define MEMORY_RAM_BASE  0x80000000u

/**
 * @brief Size of the RAM region in bytes.
 * 
 */
#define MEMORY_RAM_SIZE  0x20000000u
/**
 * @brief Size of the MMIO region in bytes.
 *
 */
#define MEMORY_MMIO_SIZE  0x10000000u
/**
 * @brief Size of the ROM region in bytes.
 *
 */
#define MEMORY_ROM_SIZE  0x00100000u


typedef enum {
    MEMORY_LOCATION_NONE = 0x0,
    MEMORY_LOCATION_ROM = 0x1,
    MEMORY_LOCATION_RAM = 0x2,
    MEMORY_LOCATION_MMIO = 0x4,
    MEMORY_LOCATION_MEM = MEMORY_LOCATION_ROM | MEMORY_LOCATION_RAM,
    MEMORY_LOCATION_WRITABLE = MEMORY_LOCATION_RAM | MEMORY_LOCATION_MMIO,
    MEMORY_LOCATION_ALL = MEMORY_LOCATION_ROM | MEMORY_LOCATION_RAM | MEMORY_LOCATION_MMIO
} memory_location_t;


/**
 * @brief Guest memory: separate host buffers for the ROM, MMIO and RAM regions.
 *
 */
typedef struct {
    uint8_t *rom; uint32_t rom_size;
    uint8_t *mmio; uint32_t mmio_size;
    uint8_t *ram; uint32_t ram_size;
} memory_t;

/**
 * @brief Allocates guest memory for ROM and RAM regions.
 *
 * @return true on success, false if allocation fails.
 *
 */
bool memory_init(memory_t *mem);

/**
 * @brief Frees the guest memory buffer and resets its fields.
 *
 * @param mem Pointer to the memory structure to free.
 *
 */
void memory_free(memory_t *mem);

/**
 * @brief Determines the memory location type for a given address.
 *
 * @param mem Pointer to guest memory.
 * @param addr Byte address to check.
 *
 * @return Memory location type (ROM, RAM, MMIO, or NONE).
 *
 */
memory_location_t memory_location(const memory_t *mem, uint32_t addr);

/**
 * @brief Returns a pointer to memory location if the address and length are within bounds.
 *
 * @param mem Pointer to guest memory.
 * @param addr Starting byte address.
 * @param len Number of bytes to access.
 * @param loc Memory location to access (ROM, RAM, MMIO, or ALL).
 *
 * @return Pointer to the memory location if in bounds, NULL otherwise.
 *
 */
uint8_t *memory_ptr(const memory_t *mem, uint32_t addr, uint32_t len, memory_location_t loc);

/**
 * @brief Reads a block of memory from guest memory.
 *
 * @param mem Pointer to guest memory.
 * @param addr Starting byte address to read from.
 * @param value Output pointer for the read data.
 * @param len Number of bytes to read.
 *
 * @return true if the access is in bounds, false otherwise.
 *
 */
bool memory_read(const memory_t *mem, uint32_t addr, void *value, uint32_t len);

/**
 * @brief Writes a block of memory to guest memory.
 *
 * @param mem Pointer to guest memory.
 * @param addr Starting byte address to write to.
 * @param value Pointer to the data to write.
 * @param len Number of bytes to write.
 *
 * @return true if the access is in bounds, false otherwise.
 *
 */
bool memory_write(memory_t *mem, uint32_t addr, const void *value, uint32_t len);

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
bool memory_read_byte(const memory_t *mem, uint32_t addr, uint8_t *value);

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
bool memory_write_byte(memory_t *mem, uint32_t addr, uint8_t value);

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
bool memory_read_half(const memory_t *mem, uint32_t addr, uint16_t *value);

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
bool memory_write_half(memory_t *mem, uint32_t addr, uint16_t value);

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
bool memory_read_word(const memory_t *mem, uint32_t addr, uint32_t *value);

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
bool memory_write_word(memory_t *mem, uint32_t addr, uint32_t value);

#endif // __MEMORY_H__
