#include <stdio.h>

#include "elf_parse.h"
#include "memory.h"
#include "cpu.h"

#include "device_uart.h"
#include "device_exit.h"


#define MAX_ITER 1000000000

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <elf_file>\n", argv[0]);
        return 1;
    }
    FILE *file = fopen(argv[1], "rb");
    if (!file) {
        fprintf(stderr, "Failed to open %s\n", argv[1]);
        return 1;
    }
    
    memory_t mem;
    if (!memory_init(&mem)) {
        fprintf(stderr, "Failed to allocate memory\n");
        fclose(file);
        return 1;
    }

    uart_state_t uart_state = { .fd = 1 }; // stdout
    memory_device_t uart_dev = device_uart_init(UART_BASE, &uart_state);
    if (!memory_add_device(&mem, &uart_dev)) {
        fprintf(stderr, "Failed to add UART device\n");
        memory_free(&mem);
        fclose(file);
        return 1;
    }

    exit_state_t exit_state = { .exited = false, .code = 0 };
    memory_device_t exit_dev = device_exit_init(EXIT_BASE, &exit_state);
    if (!memory_add_device(&mem, &exit_dev)) {
        fprintf(stderr, "Failed to add EXIT device\n");
        memory_free(&mem);
        fclose(file);
        return 1;
    }

    uint32_t entry;
    if (!elf_read(file, &mem, &entry)) {
        fprintf(stderr, "Failed to parse ELF file\n");
        memory_free(&mem);
        fclose(file);
        return 1;
    }

    fclose(file);

    cpu_t cpu;
    if (!cpu_init(&cpu, &mem, entry)) {
        fprintf(stderr, "Failed to allocate CPU\n");
        memory_free(&mem);
        return 1;
    }

    printf("\n");
    fflush(stdout);

    step_t r = STEP_OK;
    for (size_t i = 0; i < MAX_ITER && r == STEP_OK && !exit_state.exited; i++) {
        r = cpu_step(&cpu, &mem);
    }

    printf("\n");
    if (r == STEP_OK && !exit_state.exited) {
        fprintf(stderr, "Maximum number of iterations reached\n");
        memory_free(&mem);
        return 1;
    }
    if (r == STEP_ERROR) {
        memory_free(&mem);
        return 1;
    }

    if (exit_state.exited) {
        printf("> Program exited with code %d\n", exit_state.code);
    } else {
        printf("> Program did not exit properly\n");
    }

    int exit_code = exit_state.code & 0xFF; // truncate exit code to 8 bits like a POSIX process
    memory_free(&mem);
    return exit_code;
}
