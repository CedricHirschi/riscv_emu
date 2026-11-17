#ifndef __DEVICE_EXIT_REGS_H__
#define __DEVICE_EXIT_REGS_H__

#include <stdint.h>

#define EXIT_BASE 0x10001000u

typedef struct {
    uint32_t code;     ///< +0x0 write-only: perform exit with this code
} exit_regs_t;

#define EXIT_STATUS_READY 0x1u

#endif // __DEVICE_EXIT_REGS_H__
