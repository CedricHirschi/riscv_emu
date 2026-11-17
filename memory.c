#include "memory.h"

#include <stdlib.h>
#include <string.h>

_Static_assert(__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__, "host must be little-endian");


bool memory_init(memory_t *mem)
{
    mem->ram = (uint8_t *)calloc(MEMORY_RAM_SIZE, 1);
    if (!mem->ram) {
        return false;
    }
    mem->ram_size = MEMORY_RAM_SIZE;

    mem->mmio = (uint8_t *)calloc(MEMORY_MMIO_SIZE, 1);
    if (!mem->mmio) {
        free(mem->ram);
        return false;
    }
    mem->mmio_size = MEMORY_MMIO_SIZE;

    mem->rom = (uint8_t *)calloc(MEMORY_ROM_SIZE, 1);
    if (!mem->rom) {
        free(mem->ram);
        free(mem->mmio);
        return false;
    }
    mem->rom_size = MEMORY_ROM_SIZE;

    return true;
}

void memory_free(memory_t *mem)
{
    free(mem->ram);
    mem->ram = NULL;
    mem->ram_size = 0;

    free(mem->mmio);
    mem->mmio = NULL;
    mem->mmio_size = 0;

    free(mem->rom);
    mem->rom = NULL;
    mem->rom_size = 0;
}

static uint8_t *memory_in(uint8_t *buf, uint32_t base, uint32_t size, uint32_t addr, uint32_t len)
{
    uint32_t off = addr - base;
    return (uint64_t)off + len <= size ? buf + off : NULL;
}

memory_location_t memory_location(const memory_t *mem, uint32_t addr)
{
    if (memory_in(mem->rom, MEMORY_ROM_BASE, mem->rom_size, addr, 1)) {
        return MEMORY_LOCATION_ROM;
    }
    if (memory_in(mem->ram, MEMORY_RAM_BASE, mem->ram_size, addr, 1)) {
        return MEMORY_LOCATION_RAM;
    }
    if (memory_in(mem->mmio, MEMORY_MMIO_BASE, mem->mmio_size, addr, 1)) {
        return MEMORY_LOCATION_MMIO;
    }
    return MEMORY_LOCATION_NONE;
}

uint8_t *memory_ptr(const memory_t *mem, uint32_t addr, uint32_t len, memory_location_t loc)
{
    uint8_t *p = NULL;
    if (loc & MEMORY_LOCATION_ROM) {
        p = memory_in(mem->rom, MEMORY_ROM_BASE, mem->rom_size, addr, len);
    }
    if (!p && (loc & MEMORY_LOCATION_RAM)) {
        p = memory_in(mem->ram, MEMORY_RAM_BASE, mem->ram_size, addr, len);
    }
    if (!p && (loc & MEMORY_LOCATION_MMIO)) {
        p = memory_in(mem->mmio, MEMORY_MMIO_BASE, mem->mmio_size, addr, len);
    }
    return p;
}

bool memory_read(const memory_t *mem, uint32_t addr, void *value, uint32_t len)
{
    uint8_t *p = memory_ptr(mem, addr, len, MEMORY_LOCATION_ALL);
    if (!p) {
        return false;
    }
    memcpy(value, p, len);
    return true;
}

bool memory_write(memory_t *mem, uint32_t addr, const void *value, uint32_t len)
{
    uint8_t *p = memory_ptr(mem, addr, len, MEMORY_LOCATION_WRITABLE);
    if (!p) {
        return false;
    }
    memcpy(p, value, len);
    return true;
}

bool memory_read_byte(const memory_t *mem, uint32_t addr, uint8_t *value)
{
    return memory_read(mem, addr, value, 1);
}

bool memory_write_byte(memory_t *mem, uint32_t addr, uint8_t value)
{
    return memory_write(mem, addr, &value, 1);
}

bool memory_read_half(const memory_t *mem, uint32_t addr, uint16_t *value)
{
    return memory_read(mem, addr, value, 2);
}

bool memory_write_half(memory_t *mem, uint32_t addr, uint16_t value)
{
    return memory_write(mem, addr, &value, 2);
}

bool memory_read_word(const memory_t *mem, uint32_t addr, uint32_t *value)
{
    return memory_read(mem, addr, value, 4);
}

bool memory_write_word(memory_t *mem, uint32_t addr, uint32_t value)
{
    return memory_write(mem, addr, &value, 4);
}
