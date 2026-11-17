#include "device_uart.h"

#include <unistd.h>
#include <stddef.h>

_Static_assert(offsetof(uart_regs_t, tx) == 0x0 && offsetof(uart_regs_t, status) == 0x4, "UART layout is guest ABI");

static bool uart_read(memory_device_t *dev, uint32_t off, uint32_t len, uint32_t *val)
{
    (void)dev; (void)len;
    if (off != offsetof(uart_regs_t, status)) return false;
    *val = UART_STATUS_READY; // always ready
    return true;
}

static bool uart_write(memory_device_t *dev, uint32_t off, uint32_t len, uint32_t val)
{
    (void)len;
    if (off != offsetof(uart_regs_t, tx)) return false;
    uint8_t c = val;
    return write(((uart_state_t *)dev->state)->fd, &c, 1) == 1; // unbuffered, same as the write syscall
}

memory_device_t device_uart_init(uint32_t base, void *state) {
    memory_device_t dev = {0};
    dev.base = base;
    dev.size = sizeof(uart_regs_t);
    dev.state = state;
    dev.read = uart_read;
    dev.write = uart_write;

    return dev;
}
