#include <stdio.h>

#include "elf_parse.h"
#include "memory.h"
#include "cpu.h"


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
    if (!memory_init(&mem, 8192)) {
        fprintf(stderr, "Failed to allocate memory\n");
        fclose(file);
        return 1;
    }
    elf_fields_t fields;
    if (!elf_read(file, &fields, &mem)) {
        fprintf(stderr, "Failed to parse ELF file\n");
        fclose(file);
        return 1;
    }

    fclose(file);

    cpu_t cpu;
    if (!cpu_init(&cpu, &mem, fields.ehdr.entry)) {
        fprintf(stderr, "Failed to allocate CPU\n");
        return 1;
    }

    printf("\n");

    step_t r = STEP_OK;
    for (size_t i = 0; i < MAX_ITER && r == STEP_OK; i++) {
        r = cpu_step(&cpu, &mem);
    }

    printf("\n");
    if (r == STEP_OK) {
        fprintf(stderr, "Maximum number of iterations reached\n");
        return 1;
    }
    if (r == STEP_ERROR) {
        return 1;
    }

    printf("> Program returned %d\n", cpu.regs.arr[10]);

    return cpu.regs.arr[10] & 0xFF; // a0 exit code, truncated to 8 bits like a POSIX process
}
