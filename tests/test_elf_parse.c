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
    
    memory_t mem;
    assert(memory_init(&mem, 256));
    elf_fields_t fields;
    if (!elf_read(file, &fields, &mem)) {
        fprintf(stderr, "Failed to parse ELF file\n");
        fclose(file);
        return 1;
    }

    // Something was loaded at the entry point
    uint32_t first;
    assert(memory_read_word(&mem, fields.ehdr.entry, &first));
    assert(first != 0);

    memory_free(&mem);
    fclose(file);
    return 0;
}