#include <assert.h>
#include <string.h>
#include <unistd.h>

#include "syscall.h"


void test_write() {
    memory_t mem;
    assert(memory_init(&mem, 16));
    memcpy(mem.data + 10, "hello!", 6);

    int fds[2];
    assert(pipe(fds) == 0);

    register_file_t regs = {0};
    regs.regs.a7 = 64;
    regs.regs.a0 = fds[1];
    regs.regs.a1 = 10;
    regs.regs.a2 = 6; // ends exactly at mem.size
    assert(syscall_handle(&regs, &mem) == STEP_OK);

    char buf[8] = {0};
    assert(read(fds[0], buf, sizeof buf) == 6);
    assert(memcmp(buf, "hello!", 6) == 0);

    regs.regs.a2 = 7; // one byte past the end
    assert(syscall_handle(&regs, &mem) == STEP_ERROR);
    regs.regs.a1 = 0xFFFFFFFF; // buf + count must not wrap
    regs.regs.a2 = 2;
    assert(syscall_handle(&regs, &mem) == STEP_ERROR);

    close(fds[0]);
    close(fds[1]);
    memory_free(&mem);
}

void test_exit_and_unknown() {
    memory_t mem = {0};
    register_file_t regs = {0};
    regs.regs.a7 = 93;
    assert(syscall_handle(&regs, &mem) == STEP_EXIT);
    regs.regs.a7 = 12345;
    assert(syscall_handle(&regs, &mem) == STEP_ERROR);
}


int main() {
    test_write();
    test_exit_and_unknown();
    return 0;
}
