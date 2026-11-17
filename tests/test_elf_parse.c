#include <stdio.h>
#include <assert.h>

#include "elf_parse.h"


int main() {
    // Load the sample program built from program/
    FILE *file = fopen(PROGRAM_DIR "/main.elf", "rb");
    if (!file) {
        perror("Failed to open " PROGRAM_DIR "/main.elf");
        return 1;
    }
    
    memory_t mem;
    assert(memory_init(&mem));
    uint32_t entry;
    if (!elf_read(file, &mem, &entry)) {
        fprintf(stderr, "Failed to parse ELF file\n");
        fclose(file);
        return 1;
    }

    // Something was loaded at the entry point, which must be executable ROM
    assert(memory_location(&mem, entry) == MEMORY_LOCATION_ROM);
    uint32_t first;
    assert(memory_read_word(&mem, entry, &first));
    assert(first != 0);

    memory_free(&mem);
    fclose(file);
    return 0;
}