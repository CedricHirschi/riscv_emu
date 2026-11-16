#ifndef __CPU_H__
#define __CPU_H__

#include <stdbool.h>

#include "memory.h"
#include "register.h"

typedef struct {
    register_file_t regs;
    uint32_t pc;
} cpu_t;

typedef enum { STEP_OK, STEP_EXIT, STEP_ERROR } step_t;

// Executes one instruction. Anything other than STEP_OK means stop.
step_t cpu_step(cpu_t *cpu, memory_t *mem);

#endif // __CPU_H__
