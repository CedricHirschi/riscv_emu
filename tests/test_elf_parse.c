#include <stdio.h>
#include <assert.h>

#include "elf_parse.h"


int main() {
    // Open and read "add12.elf"
    FILE *file = fopen("tests/add12.elf", "rb");
    if (!file) {
        perror("Failed to open add12.elf");
        return 1;
    }
    
    uint8_t mem[256];
    elf_fields_t fields;
    size_t offset;
    size_t read;
    if (!elf_read(file, &fields, mem, sizeof(mem), &offset, &read)) {
        fprintf(stderr, "Failed to parse ELF file\n");
        fclose(file);
        return 1;
    }

    assert(offset == 0);
    assert(read > 0);
    assert(read % 4 == 0);

    fclose(file);
    return 0;
}