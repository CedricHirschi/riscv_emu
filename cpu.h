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
#include "instruction.h"

/**
 * @brief Represents the emulated RISC-V CPU state.
 *
 */
typedef struct {
    register_file_t regs;
    uint32_t pc;
    inst_decoded_t *icache;

    struct {
        uint32_t mstatus;  // bit 3 MIE (interrupts on), bit 7 MPIE (saved MIE); MPP reads as M (0x1800)
        uint32_t mtvec;    // handler address; low 2 bits = mode (0 direct, 1 vectored)
        uint32_t mepc;     // pc of the trapping instruction; writes clear bits [1:0]
        uint32_t mcause;   // bit 31 = interrupt, rest = cause code
        uint32_t mtval;    // faulting address or instruction word
        uint32_t mscratch; // free for the handler, used to save a register on entry
        uint32_t mie, mip; // interrupt enable/pending, only needed once interrupts exist
    } csr;
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
