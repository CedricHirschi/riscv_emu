#include <stdint.h>

#include "lib/print.h"

#ifndef FIB_N
#define FIB_N 35
#endif

static uint32_t fib(uint32_t n)
{
    if (n < 2)
        return n;

    return fib(n - 1) + fib(n - 2);
}

int main(void)
{
    printf("fib(%d) = %d\n", FIB_N, fib(FIB_N));

    printf("Hello, %s!\n", "world");
    
    return 0;
}
