#include "memory.h"

#include <stdlib.h>

bool memory_init(memory_t *mem, size_t size)
{
    mem->data = (uint8_t *)malloc(size);
    if (!mem->data) {
        return false;
    }

    mem->size = size;
    mem->used = 0;

    return true;
}

void memory_free(memory_t *mem)
{
    if (mem->data) {
        free(mem->data);
        mem->data = NULL;
        mem->size = 0;
        mem->used = 0;
    }
}

bool memory_read_byte(const memory_t *mem, size_t addr, uint8_t *value)
{
    if (addr >= mem->size) {
        return false;
    }
    *value = mem->data[addr];
    return true;
}

bool memory_write_byte(memory_t *mem, size_t addr, uint8_t value)
{
    if (addr >= mem->size) {
        return false;
    }
    mem->data[addr] = value;
    return true;
}

bool memory_read_half(const memory_t *mem, size_t addr, uint16_t *value)
{
    if (addr + 1 >= mem->size) {
        return false;
    }
    *value = mem->data[addr] | (mem->data[addr + 1] << 8);
    return true;
}

bool memory_write_half(memory_t *mem, size_t addr, uint16_t value)
{
    if (addr + 1 >= mem->size) {
        return false;
    }
    mem->data[addr] = value & 0xFF;
    mem->data[addr + 1] = (value >> 8) & 0xFF;
    return true;
}

bool memory_read_word(const memory_t *mem, size_t addr, uint32_t *value)
{
    if (addr + 3 >= mem->size) {
        return false;
    }
    *value = mem->data[addr] | (mem->data[addr + 1] << 8) | (mem->data[addr + 2] << 16) | (mem->data[addr + 3] << 24);
    return true;
}

bool memory_write_word(memory_t *mem, size_t addr, uint32_t value)
{
    if (addr + 3 >= mem->size) {
        return false;
    }
    mem->data[addr] = value & 0xFF;
    mem->data[addr + 1] = (value >> 8) & 0xFF;
    mem->data[addr + 2] = (value >> 16) & 0xFF;
    mem->data[addr + 3] = (value >> 24) & 0xFF;
    return true;
}
