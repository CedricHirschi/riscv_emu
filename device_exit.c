#include "device_exit.h"

#include <unistd.h>
#include <stddef.h>

_Static_assert(offsetof(exit_regs_t, code) == 0x0, "EXIT layout is guest ABI");

static bool exit_write(memory_device_t *dev, uint32_t off, uint32_t len, uint32_t val)
{
    (void)len;
    if (off != offsetof(exit_regs_t, code)) return false;
    ((exit_state_t *)dev->state)->code = val;
    ((exit_state_t *)dev->state)->exited = true;
    return true;
}

memory_device_t device_exit_init(uint32_t base, void *state) {
    memory_device_t dev = {0};
    dev.base = base;
    dev.size = sizeof(exit_regs_t);
    dev.state = state;
    dev.read = NULL;
    dev.write = exit_write;

    return dev;
}
