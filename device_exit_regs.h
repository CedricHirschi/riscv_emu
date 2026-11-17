#ifndef __DEVICE_EXIT_REGS_H__
#define __DEVICE_EXIT_REGS_H__

#define EXIT_BASE 0x10001000

#ifndef __ASSEMBLER__ // Wrapped to make sure this can be included in the assembly code as well
#include <stdint.h>

typedef struct {
    uint32_t code;     ///< +0x0 write-only: perform exit with this code
} exit_regs_t;
#endif

#define EXIT_STATUS_READY 0x1u

#endif // __DEVICE_EXIT_REGS_H__
