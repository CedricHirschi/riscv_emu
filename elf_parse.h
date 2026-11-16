#ifndef __ELF_H__
#define __ELF_H__

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

#include "memory.h"

typedef struct {
    struct {
        uint8_t magic[4];
        uint8_t clazz;
        uint8_t data;
        uint8_t version;
        uint8_t osabi;
        uint8_t abiversion;
        uint8_t pad[7];
    } ident;

    struct {
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
    } ehdr;

    struct {
        uint32_t type;
        uint32_t offset;
        uint32_t vaddr;
        uint32_t paddr;
        uint32_t filesz;
        uint32_t memsz;
        uint32_t flags;
        uint32_t align;
    } *phdr;

    struct {
        uint32_t name;
        uint32_t type;
        uint32_t flags;
        uint32_t addr;
        uint32_t offset;
        uint32_t size;
        uint32_t link;
        uint32_t info;
        uint32_t addralign;
        uint32_t entsize;
    } *shdr;
} elf_fields_t;

bool elf_read(FILE *file, elf_fields_t *fields, memory_t *mem);

#endif // __ELF_H__
