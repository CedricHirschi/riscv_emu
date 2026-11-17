#ifndef __DEVICE_UART_REGS_H__
#define __DEVICE_UART_REGS_H__

#include <stdint.h>

#define UART_BASE 0x10000000u

typedef struct {
    uint32_t tx;     ///< +0x0 write-only: low byte is sent
    uint32_t status; ///< +0x4 read-only: bit 0 = ready to send
} uart_regs_t;

#define UART_STATUS_READY 0x1u

#endif // __DEVICE_UART_REGS_H__
