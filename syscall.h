#ifndef __SYSCALL_H__
#define __SYSCALL_H__

#include "cpu.h"

step_t syscall_handle(register_file_t *regs, memory_t *mem);

#endif // __SYSCALL_H__
