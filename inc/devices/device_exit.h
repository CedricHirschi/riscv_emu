#ifndef __DEVICE_EXIT_H__
#define __DEVICE_EXIT_H__

#include "memory.h"
#include "device_exit_regs.h"

/**
 * @brief State structure for the EXIT device.
 * 
 */
typedef struct {
    bool exited;   ///< Indicates whether the exit has been triggered
    uint32_t code; ///< Exit code used on exit
} exit_state_t;

memory_device_t device_exit_init(uint32_t base, void *state);

#endif // __DEVICE_EXIT_H__
