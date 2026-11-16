#ifndef __CPU_H__
#define __CPU_H__

#include <stdbool.h>

#include "memory.h"
#include "register.h"

typedef struct {
    register_file_t regs;
    uint32_t pc;
    struct decoded *icache;
} cpu_t;

typedef enum { STEP_OK, STEP_EXIT, STEP_ERROR } step_t;

bool cpu_init(cpu_t *cpu, const memory_t *mem, uint32_t pc);

step_t cpu_step(cpu_t *cpu, memory_t *mem);

#endif // __CPU_H__
