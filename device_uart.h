#ifndef __DEVICE_UART_H__
#define __DEVICE_UART_H__

#include "memory.h"
#include "device_uart_regs.h"

/**
 * @brief State structure for the UART device.
 * 
 */
typedef struct {
    int fd;  ///< File descriptor for the UART device.
} uart_state_t;

memory_device_t device_uart_init(uint32_t base, void *state);

#endif // __DEVICE_UART_H__
