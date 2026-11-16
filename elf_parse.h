/**
 * @file elf_parse.h
 * @brief 32-bit RISC-V ELF header parsing and segment loading into guest memory.
 *
 */

#ifndef __ELF_H__
#define __ELF_H__

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

#include "memory.h"

/**
 * @brief Parsed 32-bit ELF identification, file header, program headers, and section headers.
 *
 */
typedef struct {
    /**
     * @brief ELF identification header (16 bytes).
     *
     */
    struct {
        uint8_t magic[4];
        uint8_t clazz;
        uint8_t data;
        uint8_t version;
        uint8_t osabi;
        uint8_t abiversion;
        uint8_t pad[7];
    } ident;

    /**
     * @brief 32-bit ELF file header.
     *
     */
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

    /**
     * @brief Array of ELF program header entries.
     *
     */
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

    /**
     * @brief Array of ELF section header entries.
     *
     */
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

/**
 * @brief Parses an ELF file and loads its loadable segments into guest memory.
 *
 * @param file Open file stream positioned at the start of the ELF binary.
 * @param fields Output structure populated with parsed ELF headers.
 * @param mem Target guest memory into which segments are loaded.
 *
 * @return true on success, false if reading, validation, or memory loading fails.
 *
 */
bool elf_read(FILE *file, elf_fields_t *fields, memory_t *mem);

#endif // __ELF_H__
