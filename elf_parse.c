#include "elf_parse.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>


typedef struct {
    uint8_t magic[4];
    uint8_t clazz;
    uint8_t data;
    uint8_t version;
    uint8_t osabi;
    uint8_t abiversion;
    uint8_t pad[7];
} elf_ident_t;

typedef struct {
    uint16_t type;
    uint16_t machine;
    uint32_t version;
    uint32_t entry;
    uint32_t phoff;
    uint32_t shoff;
    uint32_t flags;
    uint16_t ehsize;
    uint16_t phentsize;
    uint16_t phnum;
    uint16_t shentsize;
    uint16_t shnum;
    uint16_t shstrndx;
} elf_ehdr_t;

typedef struct {
    uint32_t type;
    uint32_t offset;
    uint32_t vaddr;
    uint32_t paddr;
    uint32_t filesz;
    uint32_t memsz;
    uint32_t flags;
    uint32_t align;
} elf_phdr_t;

bool elf_read(FILE *file, memory_t *mem, uint32_t *entry)
{
    if (!file || !mem || !entry) {
        fprintf(stderr, "elf_read: invalid NULL argument\n");
        return false;
    }

    fseek(file, 0, SEEK_END);
    size_t file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    elf_ident_t ident;
    elf_ehdr_t ehdr;

    if (file_size < sizeof(ident) + sizeof(ehdr)) {
        fprintf(stderr, "elf_read: file size is too small\n");
        return false;
    }

    if (fread(&ident, 1, sizeof(ident), file) != sizeof(ident)) {
        fprintf(stderr, "elf_read: failed to read ELF ident\n");
        return false;
    }

    bool valid = ident.magic[0] == 0x7f &&
            ident.magic[1] == 'E' &&
            ident.magic[2] == 'L' &&
            ident.magic[3] == 'F';
    if (!valid) {
        fprintf(stderr, "elf_parse: invalid magic number 0x%02x%02x%02x%02x, expected 0x7f454c46\n",
                ident.magic[0], ident.magic[1],
                ident.magic[2], ident.magic[3]);
        return false;
    }

    valid = ident.clazz == 1 && // ELFCLASS32
            ident.data == 1 &&  // ELFDATA2LSB
            ident.version == 1; // EV_CURRENT
    if (!valid) {
        fprintf(stderr, "elf_parse: invalid class, data, or version (class=0x%02x, data=0x%02x, version=0x%02x), expected (class=0x01, data=0x01, version=0x01)\n",
                ident.clazz, ident.data, ident.version);
        return false;
    }

    valid = ident.osabi == 0 && // ELFOSABI_SYSV
            ident.abiversion == 0; // ABI version 0
    if (!valid) {
        fprintf(stderr, "elf_parse: invalid OSABI or ABI version (osabi=0x%02x, abiversion=0x%02x), expected (osabi=0x00, abiversion=0x00)\n",
                ident.osabi, ident.abiversion);
        return false;
    }

    if (fread(&ehdr, 1, sizeof(ehdr), file) != sizeof(ehdr)) {
        fprintf(stderr, "elf_read: failed to read ELF header\n");
        return false;
    }

    valid = ehdr.type == 2; // ET_EXEC
    if (!valid) {
        fprintf(stderr, "elf_parse: invalid ELF type (type=0x%04x), expected ET_EXEC (0x0002)\n",
                ehdr.type);
        return false;
    }

    valid = ehdr.machine == 0xF3; // EM_RISCV
    if (!valid) {
        fprintf(stderr, "elf_parse: invalid machine (machine=0x%04x), expected EM_RISCV (0x00F3)\n",
                ehdr.machine);
        return false;
    }

    valid = ehdr.version == 1; // EV_CURRENT
    if (!valid) {
        fprintf(stderr, "elf_parse: invalid ELF version (version=0x%08x), expected EV_CURRENT (0x00000001)\n",
                ehdr.version);
        return false;
    }

    valid = ehdr.phentsize == sizeof(elf_phdr_t);
    if (!valid) {
        fprintf(stderr, "elf_parse: invalid program header entry size (phentsize=0x%04x), expected 0x%04zx\n",
                ehdr.phentsize, sizeof(elf_phdr_t));
        return false;
    }

    size_t phdr_size = ehdr.phentsize * ehdr.phnum;

    fseek(file, ehdr.phoff, SEEK_SET);
    elf_phdr_t *phdr = malloc(phdr_size);
    if (!phdr) {
        fprintf(stderr, "elf_read: failed to allocate memory for program headers\n");
        return false;
    }
    if (fread(phdr, 1, phdr_size, file) != phdr_size) {
        fprintf(stderr, "elf_read: failed to read program headers\n");
        free(phdr);
        return false;
    }

    for (size_t i = 0; i < ehdr.phnum; i++) {
        if (phdr[i].type != 1) { // PT_LOAD
            printf("  Skipping non-loadable program header (type=%u)\n", phdr[i].type);
            continue;
        }

        uint32_t va = phdr[i].vaddr;
        uint32_t filesz = phdr[i].filesz;
        uint32_t memsz = phdr[i].memsz;

        if (filesz > memsz) {
            fprintf(stderr, "elf_read: invalid program header %zu (filesz=%u, memsz=%u)\n", i, filesz, memsz);
            free(phdr);
            return false;
        }
        if ((uint64_t)va + memsz > mem->size) {
            fprintf(stderr, "elf_read: program section %zu exceeds memory bounds (vaddr=%u, memsz=%u, mem_size=%zu)\n", i, va, memsz, mem->size);
            free(phdr);
            return false;
        }

        // Zero full memsz first so any trailing .bss (memsz > filesz) is zero-initialized
        memset(mem->data + va, 0, memsz);
        fseek(file, phdr[i].offset, SEEK_SET);
        if (fread(mem->data + va, 1, filesz, file) != filesz) {
            fprintf(stderr, "elf_read: failed to read program section %zu\n", i);
            free(phdr);
            return false;
        }
    }

    free(phdr);
    *entry = ehdr.entry;
    return true;
}
