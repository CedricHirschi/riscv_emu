#ifndef __CPU_H__
#define __CPU_H__

#include <stdbool.h>

#include "memory.h"
#include "register.h"

bool cpu_step(memory_t *mem, register_file_t *regs, uint32_t *pc);

#endif // __CPU_H__
