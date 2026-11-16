#ifndef __CPU_H__
#define __CPU_H__

#include <stdbool.h>

#include "memory.h"
#include "register.h"

typedef struct {
    register_file_t regs;
    uint32_t pc;
    struct decoded *icache; // one pre-decoded entry per memory word, filled on first execution
} cpu_t;

typedef enum { STEP_OK, STEP_EXIT, STEP_ERROR } step_t;

// Sets up registers, PC and the decode cache for mem. Returns false if allocation fails.
bool cpu_init(cpu_t *cpu, const memory_t *mem, uint32_t pc);

// Executes one instruction. Anything other than STEP_OK means stop.
step_t cpu_step(cpu_t *cpu, memory_t *mem);

#endif // __CPU_H__
