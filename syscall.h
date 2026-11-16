#ifndef __SYSCALL_H__
#define __SYSCALL_H__

#include <stdbool.h>

#include "memory.h"
#include "register.h"

// Handles an ECALL (number in a7, args in a0..a2).
// Returns false when execution should stop (exit or error).
bool syscall_handle(register_file_t *regs, memory_t *mem);

#endif // __SYSCALL_H__
