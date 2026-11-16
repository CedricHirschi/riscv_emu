/**
 * @file instruction.h
 * @brief RISC-V 32-bit instruction encodings, formats, and field extraction helpers.
 *
 */

#ifndef __INSTRUCTION_H__
#define __INSTRUCTION_H__

#include <stdint.h>

/**
 * @brief RISC-V base instruction encoding formats.
 *
 */
typedef enum {
    OP_R,
    OP_I,
    OP_S,
    OP_B,
    OP_U,
    OP_J,
    OP_INVALID
} inst_format_t;

/**
 * @brief Determines the encoding format of a RISC-V instruction.
 *
 * @param inst 32-bit instruction word to inspect.
 *
 * @return The instruction format, or OP_INVALID if unrecognized.
 *
 */
inst_format_t inst_get_format(uint32_t inst);

/**
 * @brief Extracts the 7-bit opcode field.
 *
 * @param inst 32-bit instruction word.
 *
 * @return Opcode value (bits 6:0).
 *
 */
uint32_t inst_get_opcode(uint32_t inst);

/**
 * @brief Extracts the 5-bit destination register index (rd).
 *
 * @param inst 32-bit instruction word.
 *
 * @return Destination register index (bits 11:7).
 *
 */
uint32_t inst_get_rd(uint32_t inst);

/**
 * @brief Extracts the 3-bit funct3 field.
 *
 * @param inst 32-bit instruction word.
 *
 * @return funct3 value (bits 14:12).
 *
 */
uint32_t inst_get_funct3(uint32_t inst);

/**
 * @brief Extracts the 5-bit first source register index (rs1).
 *
 * @param inst 32-bit instruction word.
 *
 * @return First source register index (bits 19:15).
 *
 */
uint32_t inst_get_rs1(uint32_t inst);

/**
 * @brief Extracts the 5-bit second source register index (rs2).
 *
 * @param inst 32-bit instruction word.
 *
 * @return Second source register index (bits 24:20).
 *
 */
uint32_t inst_get_rs2(uint32_t inst);

/**
 * @brief Extracts the 7-bit funct7 field.
 *
 * @param inst 32-bit instruction word.
 *
 * @return funct7 value (bits 31:25).
 *
 */
uint32_t inst_get_funct7(uint32_t inst);

/**
 * @brief Extracts and sign-extends the immediate value for the given instruction format.
 *
 * @param inst 32-bit instruction word.
 * @param fmt Instruction format determining immediate bit layout.
 *
 * @return Sign-extended 32-bit immediate value, or 0 for formats without immediates.
 *
 */
int32_t inst_get_imm(uint32_t inst, inst_format_t fmt);

#endif // __INSTRUCTION_H__
