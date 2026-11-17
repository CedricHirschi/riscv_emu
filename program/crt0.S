#include "device_exit_regs.h"

.section .text.startup
.globl _start
_start:
    lla sp, __stack_top
    la t0, trap_handler
    csrw mtvec, t0      # from here on, traps (including ECALL) go to the guest handler
    call main
    li t0, EXIT_BASE    # EXIT_BASE: stores need a base register, no absolute addressing
    sw a0, 0(t0)        # exit(main's return value)
1:
    j 1b
