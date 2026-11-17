#include "print.h"

#include "device_exit.h"

#define EXIT ((volatile exit_regs_t *)EXIT_BASE)

__attribute__((interrupt("machine"), aligned(4)))
void trap_handler(void)
{
    uint32_t cause, epc, tval;
    __asm__ volatile("csrr %0, mcause" : "=r"(cause));
    __asm__ volatile("csrr %0, mepc"   : "=r"(epc));
    __asm__ volatile("csrr %0, mtval"  : "=r"(tval));

    if (cause == 3) { // EBREAK: report and continue after it
        printf("breakpoint at 0x%x\n", epc);
        __asm__ volatile("csrw mepc, %0" :: "r"(epc + 4));
        return; // compiler emits mret
    }
    printf("trap: cause=%d mepc=0x%x mtval=0x%x\n", cause, epc, tval);
    EXIT->code = 128 + cause; // fatal: exit through the exit device
    for (;;) {}
}
