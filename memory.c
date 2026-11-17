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

    mem->devices = (memory_device_t *)calloc(0, sizeof(memory_device_t));
    mem->n_devices = 0;

    mem->rom = (uint8_t *)calloc(MEMORY_ROM_SIZE, 1);
    if (!mem->rom) {
        free(mem->ram);
        free(mem->devices);
        return false;
    }
    mem->rom_size = MEMORY_ROM_SIZE;

    return true;
}

bool memory_add_device(memory_t *mem, const memory_device_t *dev)
{
    memory_device_t *devices = realloc(mem->devices, (mem->n_devices + 1) * sizeof *devices);
    if (!devices) {
        return false;
    }
    devices[mem->n_devices++] = *dev;
    mem->devices = devices;
    return true;
}

void memory_free(memory_t *mem)
{
    free(mem->ram);
    mem->ram = NULL;
    mem->ram_size = 0;

    free(mem->devices);
    mem->devices = NULL;
    mem->n_devices = 0;

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
    if (addr - MEMORY_MMIO_BASE < MEMORY_MMIO_SIZE) {
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
    return p;
}

static memory_device_t *find_device(const memory_t *mem, uint32_t addr, uint32_t len)
{
    for (size_t i = 0; i < mem->n_devices; i++) {
        memory_device_t *d = &mem->devices[i];
        if ((uint64_t)(uint32_t)(addr - d->base) + len <= d->size) return d;
    }
    return NULL;
}

bool memory_read(const memory_t *mem, uint32_t addr, void *value, uint32_t len)
{
    uint8_t *p = memory_ptr(mem, addr, len, MEMORY_LOCATION_MEM);
    if (p) { memcpy(value, p, len); return true; }

    memory_device_t *d = len <= 4 ? find_device(mem, addr, len) : NULL;
    uint32_t v = 0;
    if (!d || !d->read || !d->read(d, addr - d->base, len, &v)) return false;
    memcpy(value, &v, len); // little-endian host: low bytes first
    return true;
}

bool memory_write(memory_t *mem, uint32_t addr, const void *value, uint32_t len)
{
    uint8_t *p = memory_ptr(mem, addr, len, MEMORY_LOCATION_RAM);
    if (p) { memcpy(p, value, len); return true; }

    memory_device_t *d = len <= 4 ? find_device(mem, addr, len) : NULL;
    uint32_t v = 0;
    memcpy(&v, value, len);
    return d && d->write && d->write(d, addr - d->base, len, v);
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
