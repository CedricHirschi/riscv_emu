/**
 * @file cpu.h
 * @brief RISC-V CPU state, initialization, and single-step execution interface.
 *
 */

#ifndef __CPU_H__
#define __CPU_H__

#include <stdbool.h>

#include "memory.h"
#include "register.h"

/**
 * @brief Represents the emulated RISC-V CPU state.
 *
 */
typedef struct {
    register_file_t regs;
    uint32_t pc;
    struct decoded *icache;
} cpu_t;

/**
 * @brief Result status of a single CPU execution step.
 *
 */
typedef enum { STEP_OK, STEP_EXIT, STEP_ERROR } step_t;

/**
 * @brief Initializes registers, program counter, and the instruction decode cache.
 *
 * @param cpu Pointer to the CPU state to initialize.
 * @param mem Pointer to the guest memory used to size the decode cache.
 * @param pc Initial program counter value.
 *
 * @return true on success, false if decode cache allocation fails.
 *
 */
bool cpu_init(cpu_t *cpu, const memory_t *mem, uint32_t pc);

/**
 * @brief Executes one instruction at the current program counter.
 *
 * @param cpu Pointer to the CPU state.
 * @param mem Pointer to guest memory.
 *
 * @return STEP_OK to continue execution, STEP_EXIT on exit syscall, or STEP_ERROR on failure.
 *
 */
step_t cpu_step(cpu_t *cpu, memory_t *mem);

#endif // __CPU_H__
