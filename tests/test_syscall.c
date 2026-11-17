#include <assert.h>
#include <string.h>
#include <unistd.h>

#include "syscall.h"


static memory_t mem;
static int fds[2];

// Issues write(fds[1], addr, count) and returns the step result
static step_t do_write(uint32_t addr, uint32_t count) {
    register_file_t regs = {0};
    regs.abi.a7 = 64;
    regs.abi.a0 = fds[1];
    regs.abi.a1 = addr;
    regs.abi.a2 = count;
    return syscall_handle(&regs, &mem);
}

static void expect_output(const char *want, size_t len) {
    char buf[16] = {0};
    assert(read(fds[0], buf, sizeof buf) == (ssize_t)len);
    assert(memcmp(buf, want, len) == 0);
}

void test_write() {
    assert(memory_init(&mem));
    assert(pipe(fds) == 0);

    // From the very end of RAM
    uint32_t ram_end = MEMORY_RAM_BASE + MEMORY_RAM_SIZE;
    memcpy(mem.ram + MEMORY_RAM_SIZE - 6, "hello!", 6);
    assert(do_write(ram_end - 6, 6) == STEP_OK);
    expect_output("hello!", 6);

    // From ROM, where string literals in .rodata live
    memcpy(mem.rom + 0x100, "rodata", 6);
    assert(do_write(MEMORY_ROM_BASE + 0x100, 6) == STEP_OK);
    expect_output("rodata", 6);

    assert(do_write(ram_end - 6, 7) == STEP_ERROR);              // one byte past the end of RAM
    assert(do_write(MEMORY_MMIO_BASE, 4) == STEP_ERROR);         // device registers are not a buffer
    assert(do_write(MEMORY_ROM_BASE + MEMORY_ROM_SIZE - 2, 4) == STEP_ERROR); // straddles the end of ROM
    assert(do_write(MEMORY_RAM_BASE - 4, 4) == STEP_ERROR);      // unmapped gap below RAM
    assert(do_write(0xFFFFFFFF, 2) == STEP_ERROR);               // buf + count must not wrap into ROM

    close(fds[0]);
    close(fds[1]);
    memory_free(&mem);
}

void test_exit_and_unknown() {
    memory_t mem = {0};
    register_file_t regs = {0};
    regs.abi.a7 = 93;
    assert(syscall_handle(&regs, &mem) == STEP_EXIT);
    regs.abi.a7 = 12345;
    assert(syscall_handle(&regs, &mem) == STEP_ERROR);
}


int main() {
    test_write();
    test_exit_and_unknown();
    return 0;
}
