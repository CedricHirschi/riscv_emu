#include <assert.h>
#include <string.h>

#include "memory.h"

static memory_t mem;

static void test_init(void) {
    assert(mem.rom && mem.ram && mem.mmio);
    assert(mem.rom_size == MEMORY_ROM_SIZE);
    assert(mem.ram_size == MEMORY_RAM_SIZE);
    assert(mem.mmio_size == MEMORY_MMIO_SIZE);

    // Guest memory starts zeroed
    uint32_t w = 0xFFFFFFFF;
    assert(memory_read_word(&mem, MEMORY_ROM_BASE, &w) && w == 0);
    assert(memory_read_word(&mem, MEMORY_RAM_BASE + MEMORY_RAM_SIZE - 4, &w) && w == 0);
    assert(memory_read_word(&mem, MEMORY_MMIO_BASE, &w) && w == 0);
}

// First and last byte of each region, plus the gaps around them
static void test_location(void) {
    assert(memory_location(&mem, MEMORY_ROM_BASE) == MEMORY_LOCATION_ROM);
    assert(memory_location(&mem, MEMORY_ROM_BASE + MEMORY_ROM_SIZE - 1) == MEMORY_LOCATION_ROM);
    assert(memory_location(&mem, MEMORY_MMIO_BASE) == MEMORY_LOCATION_MMIO);
    assert(memory_location(&mem, MEMORY_MMIO_BASE + MEMORY_MMIO_SIZE - 1) == MEMORY_LOCATION_MMIO);
    assert(memory_location(&mem, MEMORY_MMIO_BASE + MEMORY_MMIO_SIZE) == MEMORY_LOCATION_NONE);
    assert(memory_location(&mem, MEMORY_RAM_BASE - 1) == MEMORY_LOCATION_NONE);
    assert(memory_location(&mem, MEMORY_RAM_BASE) == MEMORY_LOCATION_RAM);
    assert(memory_location(&mem, MEMORY_RAM_BASE + MEMORY_RAM_SIZE - 1) == MEMORY_LOCATION_RAM);
    assert(memory_location(&mem, MEMORY_RAM_BASE + MEMORY_RAM_SIZE) == MEMORY_LOCATION_NONE);
    assert(memory_location(&mem, 0xFFFFFFFF) == MEMORY_LOCATION_NONE);
}

static void test_ptr(void) {
    // Maps guest address to the matching host offset
    assert(memory_ptr(&mem, MEMORY_ROM_BASE + 8, 4, MEMORY_LOCATION_ALL) == mem.rom + 8);
    assert(memory_ptr(&mem, MEMORY_RAM_BASE + 8, 4, MEMORY_LOCATION_ALL) == mem.ram + 8);
    assert(memory_ptr(&mem, MEMORY_MMIO_BASE + 8, 4, MEMORY_LOCATION_ALL) == mem.mmio + 8);

    // The location mask filters regions
    assert(memory_ptr(&mem, MEMORY_ROM_BASE, 4, MEMORY_LOCATION_WRITABLE) == NULL);
    assert(memory_ptr(&mem, MEMORY_RAM_BASE, 4, MEMORY_LOCATION_ROM) == NULL);
    assert(memory_ptr(&mem, MEMORY_MMIO_BASE, 4, MEMORY_LOCATION_RAM) == NULL);

    // Ranges must lie fully inside one region, even when two regions touch
    uint32_t ram_end = MEMORY_RAM_BASE + MEMORY_RAM_SIZE;
    assert(memory_ptr(&mem, ram_end - 4, 4, MEMORY_LOCATION_ALL) == mem.ram + MEMORY_RAM_SIZE - 4);
    assert(memory_ptr(&mem, ram_end - 3, 4, MEMORY_LOCATION_ALL) == NULL);
    assert(memory_ptr(&mem, MEMORY_ROM_BASE + MEMORY_ROM_SIZE - 2, 4, MEMORY_LOCATION_ALL) == NULL); // past the end of ROM
    assert(memory_ptr(&mem, MEMORY_RAM_BASE, MEMORY_RAM_SIZE, MEMORY_LOCATION_ALL) == mem.ram);
    assert(memory_ptr(&mem, MEMORY_RAM_BASE, MEMORY_RAM_SIZE + 1, MEMORY_LOCATION_ALL) == NULL);
}

// Last in-bounds address works and the next one fails, for every size, in each writable region
static void test_bounds(void) {
    const uint32_t ends[] = { MEMORY_RAM_BASE + MEMORY_RAM_SIZE, MEMORY_MMIO_BASE + MEMORY_MMIO_SIZE };
    for (size_t r = 0; r < 2; r++) {
        for (uint32_t len = 1; len <= 4; len *= 2) {
            uint32_t in = 0xA1B2C3D4, out = 0;
            assert(memory_write(&mem, ends[r] - len, &in, len));
            assert(memory_read(&mem, ends[r] - len, &out, len));
            assert(memcmp(&in, &out, len) == 0);

            assert(!memory_write(&mem, ends[r] - len + 1, &in, len));
            assert(!memory_read(&mem, ends[r] - len + 1, &out, len));
        }
    }
}

// addr + len must not wrap past 0xFFFFFFFF into ROM at address 0
static void test_address_wraparound(void) {
    for (uint32_t len = 1; len <= 4; len *= 2) {
        for (uint32_t addr = 0xFFFFFFFC; addr != 0; addr++) {
            uint32_t v = 0;
            assert(!memory_write(&mem, addr, &v, len));
            assert(!memory_read(&mem, addr, &v, len));
        }
    }
}

// ROM is readable but every write fails and leaves it unchanged
static void test_rom_read_only(void) {
    memcpy(mem.rom, "\x13\x00\x00\x00", 4); // loaders write ROM through the host pointer
    uint32_t w = 0;
    assert(memory_read_word(&mem, MEMORY_ROM_BASE, &w) && w == 0x13);

    assert(!memory_write_byte(&mem, MEMORY_ROM_BASE, 0xFF));
    assert(!memory_write_half(&mem, MEMORY_ROM_BASE, 0xFFFF));
    assert(!memory_write_word(&mem, MEMORY_ROM_BASE, 0xFFFFFFFF));
    assert(memory_read_word(&mem, MEMORY_ROM_BASE, &w) && w == 0x13);
}

// Unmapped addresses fail and a failed access changes neither memory nor the output
static void test_failed_access_has_no_effect(void) {
    uint32_t w = 0x55555555;
    assert(!memory_read_word(&mem, 0x40000000, &w) && w == 0x55555555);
    assert(!memory_write_word(&mem, 0x40000000, 0));

    uint32_t ram_end = MEMORY_RAM_BASE + MEMORY_RAM_SIZE;
    memset(mem.ram + MEMORY_RAM_SIZE - 4, 0x5A, 4);
    assert(!memory_write_word(&mem, ram_end - 3, 0xFFFFFFFF)); // straddles the end
    assert(!memory_read_word(&mem, ram_end - 3, &w) && w == 0x55555555);
    for (uint32_t i = 1; i <= 4; i++) {
        assert(mem.ram[MEMORY_RAM_SIZE - i] == 0x5A);
    }
}

// Values are stored little-endian at unaligned addresses, touching exactly len bytes
static void test_little_endian(void) {
    memset(mem.ram, 0, 16);
    assert(memory_write_word(&mem, MEMORY_RAM_BASE + 1, 0x11223344));
    assert(!memcmp(mem.ram, "\x00\x44\x33\x22\x11\x00", 6));

    uint8_t b;
    uint16_t h;
    assert(memory_read_byte(&mem, MEMORY_RAM_BASE + 1, &b) && b == 0x44);
    assert(memory_read_half(&mem, MEMORY_RAM_BASE + 2, &h) && h == 0x2233);

    assert(memory_write_half(&mem, MEMORY_RAM_BASE + 9, 0xBEEF));
    assert(!memcmp(mem.ram + 8, "\x00\xEF\xBE\x00", 4));
    assert(memory_write_byte(&mem, MEMORY_RAM_BASE + 13, 0x7F));
    assert(!memcmp(mem.ram + 12, "\x00\x7F\x00", 3));
}

static void test_free(void) {
    memory_free(&mem);
    assert(!mem.rom && !mem.ram && !mem.mmio);
    assert(mem.rom_size == 0 && mem.ram_size == 0 && mem.mmio_size == 0);
    memory_free(&mem); // second free is a no-op

    uint32_t w;
    assert(!memory_read_word(&mem, MEMORY_RAM_BASE, &w)); // nothing is mapped any more
}


int main(void) {
    assert(memory_init(&mem));
    test_init();
    test_location();
    test_ptr();
    test_bounds();
    test_address_wraparound();
    test_rom_read_only();
    test_failed_access_has_no_effect();
    test_little_endian();
    test_free();
    return 0;
}
