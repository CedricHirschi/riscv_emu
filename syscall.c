#include "syscall.h"

#include <stdio.h>
#include <unistd.h>

step_t syscall_handle(register_file_t *regs, memory_t *mem)
{
    uint32_t syscall = regs->arr[17]; // a7
    switch (syscall) {
        case 64: { // write a2 bytes from memory address a1 to file descriptor a0
            uint32_t fd = regs->arr[10];
            uint32_t buf = regs->arr[11];
            uint32_t count = regs->arr[12];
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
        case 93: // exit, code stays in a0
            return STEP_EXIT;
        default:
            fprintf(stderr, "Unknown syscall %u\n", syscall);
            return STEP_ERROR;
    }
}
