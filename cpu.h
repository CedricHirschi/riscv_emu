#ifndef __CPU_H__
#define __CPU_H__

#include <stdbool.h>

#include "memory.h"
#include "register.h"

typedef struct {
    register_file_t regs;
    uint32_t pc;
} cpu_t;

// Returns false when execution should stop (exit, halt or error).
bool cpu_step(cpu_t *cpu, memory_t *mem);

#endif // __CPU_H__
