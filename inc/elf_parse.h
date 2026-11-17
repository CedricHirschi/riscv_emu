/**
 * @file elf_parse.h
 * @brief 32-bit RISC-V ELF header parsing and segment loading into guest memory.
 *
 */

#ifndef __ELF_H__
#define __ELF_H__

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#include "memory.h"

/**
 * @brief Parses an ELF file and loads its loadable segments into guest memory.
 *
 * @param file Open file stream positioned at the start of the ELF binary.
 * @param mem Target guest memory into which segments are loaded.
 * @param entry Output pointer populated with the ELF entry point address.
 *
 * @return true on success, false if reading, validation, or memory loading fails.
 *
 */
bool elf_read(FILE *file, memory_t *mem, uint32_t *entry);

#endif // __ELF_H__
