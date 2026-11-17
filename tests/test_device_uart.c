#include <assert.h>
#include <unistd.h>

#include "device_uart.h"

#define BASE   UART_BASE
#define TX     UART_BASE + offsetof(uart_regs_t, tx)
#define STATUS UART_BASE + offsetof(uart_regs_t, status)

static memory_t mem;
static uart_state_t uart;
static int fds[2];

// Reads whatever the UART wrote so far; the pipe is non-empty only if a byte was sent
static size_t drain(char *buf, size_t cap) {
    ssize_t n = read(fds[0], buf, cap);
    assert(n >= 0);
    return (size_t)n;
}

static void test_tx() {
    char buf[8];

    // Every store width sends exactly one byte: the low one
    assert(memory_write_byte(&mem, TX, 'A'));
    assert(memory_write_half(&mem, TX, 0x4200 | 'B'));
    assert(memory_write_word(&mem, TX, 0x11223300 | 'C'));
    assert(drain(buf, sizeof buf) == 3);
    assert(buf[0] == 'A' && buf[1] == 'B' && buf[2] == 'C');
}

static void test_status() {
    uint8_t b = 0;
    uint32_t w = 0;
    assert(memory_read_word(&mem, STATUS, &w) && w == 1); // always ready
    assert(memory_read_byte(&mem, STATUS, &b) && b == 1);
}

static void test_bad_accesses() {
    uint32_t w = 0x55555555;
    assert(!memory_read_word(&mem, TX, &w) && w == 0x55555555); // TX is write-only
    assert(!memory_write_word(&mem, STATUS, 'X'));              // STATUS is read-only
    assert(!memory_write_byte(&mem, TX + 1, 'X'));              // inside TX, but not its first byte
    assert(!memory_write_word(&mem, BASE + 0x8, 'X'));          // unused offset
    assert(!memory_read_word(&mem, BASE + 0x8, &w));

    // None of the rejected stores reached the output
    assert(memory_write_byte(&mem, TX, '.'));
    char buf[8];
    assert(drain(buf, sizeof buf) == 1 && buf[0] == '.');
}

static void test_write_error() {
    uart.fd = -1; // a failing host write becomes a store fault
    assert(!memory_write_byte(&mem, TX, 'X'));
    uart.fd = fds[1];
}


int main() {
    assert(memory_init(&mem));
    assert(pipe(fds) == 0);
    uart.fd = fds[1];
    memory_device_t dev = device_uart_init(BASE, &uart);
    assert(memory_add_device(&mem, &dev));

    test_tx();
    test_status();
    test_bad_accesses();
    test_write_error();

    close(fds[0]);
    close(fds[1]);
    memory_free(&mem);
    return 0;
}
