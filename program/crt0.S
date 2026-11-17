.section .text.startup
.globl _start
_start:
    lla sp, __stack_top
    call main
    li t0, 0x10001000   # EXIT_BASE: stores need a base register, no absolute addressing
    sw a0, 0(t0)        # exit(main's return value)
1:
    j 1b
