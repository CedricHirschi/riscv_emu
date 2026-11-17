#include <assert.h>
#include <string.h>

#include "memory.h"

static memory_t mem;

static void test_init(void) {
    assert(mem.rom && mem.ram);
    assert(mem.rom_size == MEMORY_ROM_SIZE);
    assert(mem.ram_size == MEMORY_RAM_SIZE);
    assert(mem.n_devices == 0);

    // Guest memory starts zeroed
    uint32_t w = 0xFFFFFFFF;
    assert(memory_read_word(&mem, MEMORY_ROM_BASE, &w) && w == 0);
    assert(memory_read_word(&mem, MEMORY_RAM_BASE + MEMORY_RAM_SIZE - 4, &w) && w == 0);
    assert(!memory_read_word(&mem, MEMORY_MMIO_BASE, &w)); // MMIO without a device faults
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
    assert(memory_ptr(&mem, MEMORY_ROM_BASE + 8, 4, MEMORY_LOCATION_MEM) == mem.rom + 8);
    assert(memory_ptr(&mem, MEMORY_RAM_BASE + 8, 4, MEMORY_LOCATION_MEM) == mem.ram + 8);
    assert(memory_ptr(&mem, MEMORY_MMIO_BASE + 8, 4, MEMORY_LOCATION_MEM | MEMORY_LOCATION_MMIO) == NULL); // devices have no host memory

    // The location mask filters regions
    assert(memory_ptr(&mem, MEMORY_ROM_BASE, 4, MEMORY_LOCATION_WRITABLE) == NULL);
    assert(memory_ptr(&mem, MEMORY_RAM_BASE, 4, MEMORY_LOCATION_ROM) == NULL);
    assert(memory_ptr(&mem, MEMORY_MMIO_BASE, 4, MEMORY_LOCATION_RAM) == NULL);

    // Ranges must lie fully inside one region, even when two regions touch
    uint32_t ram_end = MEMORY_RAM_BASE + MEMORY_RAM_SIZE;
    assert(memory_ptr(&mem, ram_end - 4, 4, MEMORY_LOCATION_MEM) == mem.ram + MEMORY_RAM_SIZE - 4);
    assert(memory_ptr(&mem, ram_end - 3, 4, MEMORY_LOCATION_MEM) == NULL);
    assert(memory_ptr(&mem, MEMORY_ROM_BASE + MEMORY_ROM_SIZE - 2, 4, MEMORY_LOCATION_MEM) == NULL); // past the end of ROM
    assert(memory_ptr(&mem, MEMORY_RAM_BASE, MEMORY_RAM_SIZE, MEMORY_LOCATION_MEM) == mem.ram);
    assert(memory_ptr(&mem, MEMORY_RAM_BASE, MEMORY_RAM_SIZE + 1, MEMORY_LOCATION_MEM) == NULL);
}

// Last in-bounds RAM address works and the next one fails, for every size
static void test_bounds(void) {
    uint32_t end = MEMORY_RAM_BASE + MEMORY_RAM_SIZE;
    for (uint32_t len = 1; len <= 4; len *= 2) {
        uint32_t in = 0xA1B2C3D4, out = 0;
        assert(memory_write(&mem, end - len, &in, len));
        assert(memory_read(&mem, end - len, &out, len));
        assert(memcmp(&in, &out, len) == 0);

        assert(!memory_write(&mem, end - len + 1, &in, len));
        assert(!memory_read(&mem, end - len + 1, &out, len));
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

// Fake device: records the last access, reads return 0x11223344 + off
typedef struct { uint32_t off, len, val, writes; } fake_t;

static bool fake_read(memory_device_t *dev, uint32_t off, uint32_t len, uint32_t *val) {
    fake_t *f = dev->state;
    f->off = off;
    f->len = len;
    *val = 0x11223344 + off;
    return true;
}

static bool fake_write(memory_device_t *dev, uint32_t off, uint32_t len, uint32_t val) {
    fake_t *f = dev->state;
    f->off = off;
    f->len = len;
    f->val = val;
    f->writes++;
    return true;
}

static bool reject_read(memory_device_t *dev, uint32_t off, uint32_t len, uint32_t *val) {
    (void)dev; (void)off; (void)len;
    *val = 0xBAD;
    return false;
}

static bool reject_write(memory_device_t *dev, uint32_t off, uint32_t len, uint32_t val) {
    (void)dev; (void)off; (void)len; (void)val;
    return false;
}

#define DEV (MEMORY_MMIO_BASE + 0x100)
#define RO  (MEMORY_MMIO_BASE + 0x200)
#define BAD (MEMORY_MMIO_BASE + 0x300)

static void test_devices(void) {
    static fake_t fake;
    memory_device_t d = { DEV, 16, fake_read, fake_write, &fake };
    assert(memory_add_device(&mem, &d));
    d.base = 0; // memory keeps its own copy
    assert(memory_add_device(&mem, &(memory_device_t){ RO, 4, fake_read, NULL, &fake }));
    assert(memory_add_device(&mem, &(memory_device_t){ BAD, 4, reject_read, reject_write, NULL }));
    assert(mem.n_devices == 3);

    // Writes arrive as zero-extended register values with offset and width
    assert(memory_write_byte(&mem, DEV + 3, 0xAB));
    assert(fake.off == 3 && fake.len == 1 && fake.val == 0xAB);
    assert(memory_write_half(&mem, DEV + 2, 0xBEEF));
    assert(fake.off == 2 && fake.len == 2 && fake.val == 0xBEEF);
    assert(memory_write_word(&mem, DEV + 12, 0xDEADBEEF));
    assert(fake.off == 12 && fake.len == 4 && fake.val == 0xDEADBEEF);
    uint32_t in = 0xA1B2C3D4;
    assert(memory_write(&mem, DEV, &in, 1));
    assert(fake.val == 0xD4); // only the low len bytes are passed on

    // Reads return only the low len bytes of the device value
    uint8_t b;
    uint16_t h;
    uint32_t w;
    assert(memory_read_word(&mem, DEV + 4, &w) && w == 0x11223348);
    assert(memory_read_half(&mem, DEV + 2, &h) && h == 0x3346 && fake.len == 2);
    assert(memory_read_byte(&mem, DEV + 1, &b) && b == 0x45 && fake.len == 1);

    // Accesses must lie fully inside a device
    uint32_t writes = fake.writes;
    assert(!memory_write_word(&mem, DEV + 14, 0)); // straddles the end
    assert(!memory_write_byte(&mem, DEV - 1, 0));  // just before base
    assert(!memory_write_byte(&mem, DEV + 16, 0)); // just after the end, no device there
    assert(fake.writes == writes);

    // NULL write makes a device read-only
    assert(memory_read_word(&mem, RO, &w) && w == 0x11223344);
    assert(!memory_write_word(&mem, RO, 0));
    assert(fake.writes == writes);

    // A callback returning false faults, and a failed read leaves the output untouched
    w = 0x55555555;
    assert(!memory_read_word(&mem, BAD, &w) && w == 0x55555555);
    assert(!memory_write_word(&mem, BAD, 0));

    // RAM and ROM still behave as before with devices present
    assert(memory_write_word(&mem, MEMORY_RAM_BASE, 0x12345678));
    assert(memory_read_word(&mem, MEMORY_RAM_BASE, &w) && w == 0x12345678);
    assert(!memory_write_word(&mem, MEMORY_ROM_BASE, 0));
}

static void test_free(void) {
    memory_free(&mem);
    assert(!mem.rom && !mem.ram && !mem.devices);
    assert(mem.rom_size == 0 && mem.ram_size == 0 && mem.n_devices == 0);
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
    test_devices();
    test_free();
    return 0;
}
