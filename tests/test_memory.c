#include <assert.h>

#include "memory.h"


void test_bounds() {
    memory_t mem;
    assert(memory_init(&mem, 16));
    uint8_t b = 0;
    uint16_t h = 0;
    uint32_t w = 0;

    assert(memory_write_byte(&mem, 15, 0xAB));
    assert(memory_read_byte(&mem, 15, &b) && b == 0xAB);
    assert(!memory_write_byte(&mem, 16, 0));
    assert(!memory_read_byte(&mem, 16, &b));

    assert(memory_write_half(&mem, 14, 0x1234));
    assert(memory_read_half(&mem, 14, &h) && h == 0x1234);
    assert(!memory_write_half(&mem, 15, 0));
    assert(!memory_read_half(&mem, 15, &h));

    assert(memory_write_word(&mem, 12, 0xDEADBEEF));
    assert(memory_read_word(&mem, 12, &w) && w == 0xDEADBEEF);
    assert(!memory_write_word(&mem, 13, 0));
    assert(!memory_read_word(&mem, 13, &w));

    // A failed read leaves the output untouched
    w = 0x55555555;
    assert(!memory_read_word(&mem, 100, &w) && w == 0x55555555);

    memory_free(&mem);
}

void test_little_endian() {
    memory_t mem;
    assert(memory_init(&mem, 8));
    assert(memory_write_word(&mem, 1, 0x11223344)); // unaligned is allowed
    uint8_t b;
    assert(memory_read_byte(&mem, 1, &b) && b == 0x44);
    assert(memory_read_byte(&mem, 4, &b) && b == 0x11);
    uint16_t h;
    assert(memory_read_half(&mem, 2, &h) && h == 0x2233);
    memory_free(&mem);
}

void test_free() {
    memory_t mem;
    assert(memory_init(&mem, 8));
    memory_free(&mem);
    assert(mem.data == NULL && mem.size == 0);
    memory_free(&mem); // second free is a no-op
}


int main() {
    test_bounds();
    test_little_endian();
    test_free();
    return 0;
}
