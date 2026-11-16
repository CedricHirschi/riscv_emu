#include "memory.h"

#include <stdlib.h>
#include <string.h>

_Static_assert(__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__, "host must be little-endian");

bool memory_init(memory_t *mem, size_t size)
{
    mem->data = (uint8_t *)malloc(size);
    if (!mem->data) {
        return false;
    }

    mem->size = size;

    return true;
}

void memory_free(memory_t *mem)
{
    if (mem->data) {
        free(mem->data);
        mem->data = NULL;
        mem->size = 0;
    }
}

bool memory_read_byte(const memory_t *mem, uint32_t addr, uint8_t *value)
{
    if (addr >= mem->size) {
        return false;
    }
    *value = mem->data[addr];
    return true;
}

bool memory_write_byte(memory_t *mem, uint32_t addr, uint8_t value)
{
    if (addr >= mem->size) {
        return false;
    }
    mem->data[addr] = value;
    return true;
}

bool memory_read_half(const memory_t *mem, uint32_t addr, uint16_t *value)
{
    if ((uint64_t)addr + 1 >= mem->size) {
        return false;
    }
    memcpy(value, &mem->data[addr], 2);
    return true;
}

bool memory_write_half(memory_t *mem, uint32_t addr, uint16_t value)
{
    if ((uint64_t)addr + 1 >= mem->size) {
        return false;
    }
    memcpy(&mem->data[addr], &value, 2);
    return true;
}

bool memory_read_word(const memory_t *mem, uint32_t addr, uint32_t *value)
{
    if ((uint64_t)addr + 3 >= mem->size) {
        return false;
    }
    memcpy(value, &mem->data[addr], 4);
    return true;
}

bool memory_write_word(memory_t *mem, uint32_t addr, uint32_t value)
{
    if ((uint64_t)addr + 3 >= mem->size) {
        return false;
    }
    memcpy(&mem->data[addr], &value, 4);
    return true;
}
