#include "syscall.h"

#include <stdio.h>
#include <unistd.h>

step_t syscall_handle(register_file_t *regs, memory_t *mem)
{
    uint32_t syscall = regs->abi.a7;
    switch (syscall) {
        case 64: { // write(fd=a0, buf=a1, count=a2)
            uint32_t fd = regs->abi.a0;
            uint32_t buf = regs->abi.a1;
            uint32_t count = regs->abi.a2;
            if ((uint64_t)buf + count > mem->size) {
                fprintf(stderr, "write syscall out of bounds\n");
                return STEP_ERROR;
            }
            if (write(fd, &mem->data[buf], count) < 0) {
                perror("write failed");
                return STEP_ERROR;
            }
            return STEP_OK;
        }
        case 93: // exit(code=a0)
            return STEP_EXIT;
        default:
            fprintf(stderr, "Unknown syscall %u\n", syscall);
            return STEP_ERROR;
    }
}
