#include "elf_parse.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>


bool elf_read(FILE *file, elf_fields_t *fields, memory_t *mem, size_t *offset)
{
    if (!file) {
        perror("Failed to open file");
        return false;
    }

    if (!fields) {
        perror("Fields pointer is NULL");
        return false;
    }

    fseek(file, 0, SEEK_END);
    size_t file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    size_t ident_size = sizeof(fields->ident);
    size_t ehdr_size = sizeof(fields->ehdr);

    if (file_size < ident_size + ehdr_size) {
        fprintf(stderr, "elf_read: file size is too small\n");
        return false;
    }

    if (fread(&fields->ident, 1, ident_size, file) != ident_size) {
        fprintf(stderr, "elf_read: failed to read ELF ident\n");
        return false;
    }

    bool valid = fields->ident.magic[0] == 0x7f &&
            fields->ident.magic[1] == 'E' &&
            fields->ident.magic[2] == 'L' &&
            fields->ident.magic[3] == 'F';
    if (!valid) {
        fprintf(stderr, "elf_parse: invalid magic number 0x%02x%02x%02x%02x, expected 0x7f454c46\n",
                fields->ident.magic[0], fields->ident.magic[1],
                fields->ident.magic[2], fields->ident.magic[3]);
        return false;
    }

    valid = fields->ident.clazz == 1 && // ELFCLASS32
            fields->ident.data == 1 &&  // ELFDATA2LSB
            fields->ident.version == 1; // EV_CURRENT
    if (!valid) {
        fprintf(stderr, "elf_parse: invalid class, data, or version (class=0x%02x, data=0x%02x, version=0x%02x), expected (class=0x01, data=0x01, version=0x01)\n",
                fields->ident.clazz, fields->ident.data, fields->ident.version);
        return false;
    }

    valid = fields->ident.osabi == 0 && // ELFOSABI_SYSV
            fields->ident.abiversion == 0; // ABI version 0
    if (!valid) {
        fprintf(stderr, "elf_parse: invalid OSABI or ABI version (osabi=0x%02x, abiversion=0x%02x), expected (osabi=0x00, abiversion=0x00)\n",
                fields->ident.osabi, fields->ident.abiversion);
        return false;
    }

    if (fread(&fields->ehdr, 1, ehdr_size, file) != ehdr_size) {
        fprintf(stderr, "elf_read: failed to read ELF header\n");
        return false;
    }

    valid = fields->ehdr.type == 2; // ET_EXEC
    if (!valid) {
        fprintf(stderr, "elf_parse: invalid ELF type (type=0x%04x), expected ET_EXEC (0x0002)\n",
                fields->ehdr.type);
        return false;
    }

    valid = fields->ehdr.machine == 0xF3; // EM_RISCV
    if (!valid) {
        fprintf(stderr, "elf_parse: invalid machine (machine=0x%04x), expected EM_RISCV (0x0028)\n",
                fields->ehdr.machine);
        return false;
    }

    valid = fields->ehdr.version == 1; // EV_CURRENT
    if (!valid) {
        fprintf(stderr, "elf_parse: invalid ELF version (version=0x%08x), expected EV_CURRENT (0x00000001)\n",
                fields->ehdr.version);
        return false;
    }

    valid = fields->ehdr.phentsize == sizeof(fields->phdr[0]);
    if (!valid) {
        fprintf(stderr, "elf_parse: invalid program header entry size (phentsize=0x%04x), expected 0x%04zx\n",
                fields->ehdr.phentsize, sizeof(fields->phdr[0]));
        return false;
    }

    valid = fields->ehdr.shentsize == sizeof(fields->shdr[0]);
    if (!valid) {
        fprintf(stderr, "elf_parse: invalid section header entry size (shentsize=0x%04x), expected 0x%04zx\n",
                fields->ehdr.shentsize, sizeof(fields->shdr[0]));
        return false;
    }

    size_t phdr_size = fields->ehdr.phentsize * fields->ehdr.phnum;

    fseek(file, fields->ehdr.phoff, SEEK_SET);
    fields->phdr = malloc(phdr_size);
    if (!fields->phdr) {
        fprintf(stderr, "elf_read: failed to allocate memory for program headers\n");
        return false;
    }
    if (fread(fields->phdr, 1, phdr_size, file) != phdr_size) {
        fprintf(stderr, "elf_read: failed to read program headers\n");
        free(fields->phdr);
        return false;
    }

    fields->programs = malloc(fields->ehdr.phnum * sizeof(*fields->programs));
    if (!fields->programs) {
        fprintf(stderr, "elf_read: failed to allocate memory for programs array\n");
        free(fields->phdr);
        free(fields->shdr);
        return false;
    }

    size_t minimum = mem->size;
    size_t maximum = 0;

    for (size_t i = 0; i < fields->ehdr.phnum; i++) {
        if (fields->phdr[i].type != 1) {
            printf("  Skipping non-loadable program header (type=%u)\n", fields->phdr[i].type);
            continue;
        }

        uint32_t va = fields->phdr[i].vaddr;
        uint32_t filesz = fields->phdr[i].filesz;
        uint32_t memsz = fields->phdr[i].memsz;

        if (va < minimum) {
            minimum = va;
        }
        if (va + memsz > maximum) {
            maximum = va + memsz;
        }

        if (filesz > memsz) {
            fprintf(stderr, "elf_read: invalid program header %zu (filesz=%u, memsz=%u)\n", i, filesz, memsz);
            return false;
        }
        if ((uint64_t)va + memsz > mem->size) {
            fprintf(stderr, "elf_read: program section %zu exceeds memory bounds (vaddr=%u, memsz=%u, mem_size=%zu)\n", i, va, memsz, mem->size);
            return false;
        }

        memset(mem->data + va, 0, memsz);
        fseek(file, fields->phdr[i].offset, SEEK_SET);
        if (fread(mem->data + va, 1, filesz, file) != filesz) {
            fprintf(stderr, "elf_read: failed to read program section %zu\n", i);
            return false;
        }
    }

    if (offset) {
        *offset = minimum;
    }
    mem->used = maximum - minimum;

    return true;
}
