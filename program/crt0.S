.section .text.startup
.globl _start
_start:
    lla sp, __stack_top
    call main
1:  j 1b
