.section .text.startup
.globl _start
_start:
    lla sp, __stack_top
    call main
    li a7, 93 # exit(a0 = main's return value)
    ecall
