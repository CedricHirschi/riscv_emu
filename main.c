#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "elf_parse.h"
#include "instruction.h"
#include "register.h"
#include "memory.h"
#include "cpu.h"


#define MAX_ITER 1000000000

int main(int argc, char *argv[]) {
    // Open and read ELF file specified as the first command-line argument
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
    size_t offset;
    if (!elf_read(file, &fields, &mem, &offset)) {
        fprintf(stderr, "Failed to parse ELF file\n");
        fclose(file);
        return 1;
    }

    fclose(file);

    register_file_t regs;
    memset(&regs, 0, sizeof(regs));

    uint32_t pc = fields.ehdr.entry; // Program counter in bytes

    printf("\n");

    size_t i = 0;
    while (i++ < MAX_ITER) {
        if (!cpu_step(&mem, &regs, &pc)) {
            break;
        }
    }

    printf("\n");
    if (i == MAX_ITER + 1) {
        fprintf(stderr, "Maximum number of iterations reached\n");
    }
    // printf("Number of iterations: %zu\n", i);

    // for (int i = 0; i < 32; i++) {
    //     printf("x%02d: %d\n", i, regs.arr[i]);
    // }

    printf("> Program returned %d\n", regs.arr[10]);

    return 0;
}
