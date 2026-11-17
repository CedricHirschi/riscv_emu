/**
 * @file syscall.h
 * @brief Guest ECALL system call dispatcher.
 *
 */

#ifndef __SYSCALL_H__
#define __SYSCALL_H__

#include "cpu.h"

/**
 * @brief Handles an ECALL instruction using the ABI registers (number in a7, args in a0..a2).
 *
 * @param regs Pointer to the CPU register file.
 * @param mem Pointer to guest memory.
 *
 * @return STEP_OK to continue, STEP_EXIT on exit syscall, or STEP_ERROR on invalid/failed syscall.
 *
 */
step_t syscall_handle(register_file_t *regs, memory_t *mem);

#endif // __SYSCALL_H__
