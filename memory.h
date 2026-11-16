#ifndef __MEMORY_H__
#define __MEMORY_H__

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>


typedef struct {
    uint8_t *data;
    size_t size;
    size_t used;
} memory_t;

bool memory_init(memory_t *mem, size_t size);
void memory_free(memory_t *mem);

bool memory_read_byte(const memory_t *mem, size_t addr, uint8_t *value);
bool memory_write_byte(memory_t *mem, size_t addr, uint8_t value);

bool memory_read_half(const memory_t *mem, size_t addr, uint16_t *value);
bool memory_write_half(memory_t *mem, size_t addr, uint16_t value);

bool memory_read_word(const memory_t *mem, size_t addr, uint32_t *value);
bool memory_write_word(memory_t *mem, size_t addr, uint32_t value);

#endif // __MEMORY_H__
