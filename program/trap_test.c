#include <stdint.h>

#include "lib/print.h"

// Exercises the guest trap handler: EBREAK must resume, a load fault must exit with 128 + cause
int main(void)
{
    printf("before\n");
    __asm__ volatile("ebreak");
    printf("after ebreak\n");

    volatile uint32_t *unmapped = (volatile uint32_t *)0x40000000;
    (void)*unmapped; // load access fault (cause 5): handler exits with 133
    printf("not reached\n");
    return 0;
}
