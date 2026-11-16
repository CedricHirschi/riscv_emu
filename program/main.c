#include <stdint.h>

#ifndef FIB_N
#define FIB_N 10
#endif

__attribute__((noinline))
static uint32_t fib(uint32_t n)
{
    if (n < 2)
        return n;

    return fib(n - 1) + fib(n - 2);
}

static void write_stdout(const char *buf, uint32_t len)
{
    register uint32_t a0 __asm__("a0") = 1;
    register const char *a1 __asm__("a1") = buf;
    register uint32_t a2 __asm__("a2") = len;
    register uint32_t a7 __asm__("a7") = 64;

    __asm__ volatile("ecall"
        : "+r"(a0)
        : "r"(a1), "r"(a2), "r"(a7)
        : "memory");
}

__attribute__((noreturn))
static void sys_exit(uint32_t status)
{
    register uint32_t a0 __asm__("a0") = status;
    register uint32_t a7 __asm__("a7") = 93;

    __asm__ volatile("ecall" : :
        "r"(a0), "r"(a7) : "memory");

    for (;;) {}
}

int main(void)
{
    uint32_t value = fib(27);
    char out[11];
    uint32_t len = 0;
    uint32_t started = 0;

    // Decimal conversion without division (RV32I has no DIV).
    static const uint32_t powers[] = {
        1000000000u, 100000000u, 10000000u,
        1000000u, 100000u, 10000u,
        1000u, 100u, 10u, 1u
    };

    for (uint32_t i = 0; i < 10; ++i) {
        uint32_t digit = 0;

        while (value >= powers[i]) {
            value -= powers[i];
            ++digit;
        }

        if (digit || started || i == 9) {
            out[len++] = (char)('0' + digit);
            started = 1;
        }
    }

    out[len++] = '\n';
    write_stdout(out, len);
    sys_exit(0);
}

// Initialize SP before executing recursive C code.
__attribute__((naked, noreturn, used, section(".text.startup")))
void _start(void)
{
    __asm__ volatile(
        "lla sp, __stack_top\n"
        "call main\n"
        "1: j 1b\n"
    );
}
