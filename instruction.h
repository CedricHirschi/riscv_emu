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
 * @brief R-type instruction bitfield layout.
 *
 */
typedef struct {
    uint32_t opcode : 7;
    uint32_t rd     : 5;
    uint32_t funct3 : 3;
    uint32_t rs1    : 5;
    uint32_t rs2    : 5;
    uint32_t funct7 : 7;
} inst_r_t;

/**
 * @brief I-type instruction bitfield layout.
 *
 */
typedef struct {
    uint32_t opcode : 7;
    uint32_t rd     : 5;
    uint32_t funct3 : 3;
    uint32_t rs1    : 5;
    uint32_t imm    : 12;
} inst_i_t;

/**
 * @brief S-type instruction bitfield layout.
 *
 */
typedef struct {
    uint32_t opcode   : 7;
    uint32_t imm_4_0  : 5;
    uint32_t funct3   : 3;
    uint32_t rs1      : 5;
    uint32_t rs2      : 5;
    uint32_t imm_11_5 : 7;
} inst_s_t;

/**
 * @brief B-type instruction bitfield layout.
 *
 */
typedef struct {
    uint32_t opcode   : 7;
    uint32_t imm_11   : 1;
    uint32_t imm_4_1  : 4;
    uint32_t funct3   : 3;
    uint32_t rs1      : 5;
    uint32_t rs2      : 5;
    uint32_t imm_10_5 : 6;
    uint32_t imm_12   : 1;
} inst_b_t;

/**
 * @brief U-type instruction bitfield layout.
 *
 */
typedef struct {
    uint32_t opcode    : 7;
    uint32_t rd        : 5;
    uint32_t imm_31_12 : 20;
} inst_u_t;

/**
 * @brief J-type instruction bitfield layout.
 *
 */
typedef struct {
    uint32_t opcode    : 7;
    uint32_t rd        : 5;
    uint32_t imm_19_12 : 8;
    uint32_t imm_11    : 1;
    uint32_t imm_10_1  : 10;
    uint32_t imm_20    : 1;
} inst_j_t;

/**
 * @brief Union representing a raw 32-bit instruction word and its format-specific views.
 *
 */
typedef union {
    uint32_t val;
    inst_r_t r;
    inst_i_t i;
    inst_s_t s;
    inst_b_t b;
    inst_u_t u;
    inst_j_t j;
} inst_t;

/**
 * @brief Determines the encoding format of a RISC-V instruction.
 *
 * @param inst Instruction word to inspect.
 *
 * @return The instruction format, or OP_INVALID if unrecognized.
 *
 */
inst_format_t inst_get_format(inst_t inst);

/**
 * @brief Extracts the 7-bit opcode field.
 *
 * @param inst Instruction word.
 *
 * @return Opcode value (bits 6:0).
 *
 */
uint32_t inst_get_opcode(inst_t inst);

/**
 * @brief Extracts the 5-bit destination register index (rd).
 *
 * @param inst Instruction word.
 *
 * @return Destination register index (bits 11:7).
 *
 */
uint32_t inst_get_rd(inst_t inst);

/**
 * @brief Extracts the 3-bit funct3 field.
 *
 * @param inst Instruction word.
 *
 * @return funct3 value (bits 14:12).
 *
 */
uint32_t inst_get_funct3(inst_t inst);

/**
 * @brief Extracts the 5-bit first source register index (rs1).
 *
 * @param inst Instruction word.
 *
 * @return First source register index (bits 19:15).
 *
 */
uint32_t inst_get_rs1(inst_t inst);

/**
 * @brief Extracts the 5-bit second source register index (rs2).
 *
 * @param inst Instruction word.
 *
 * @return Second source register index (bits 24:20).
 *
 */
uint32_t inst_get_rs2(inst_t inst);

/**
 * @brief Extracts the 7-bit funct7 field.
 *
 * @param inst Instruction word.
 *
 * @return funct7 value (bits 31:25).
 *
 */
uint32_t inst_get_funct7(inst_t inst);

/**
 * @brief Extracts and sign-extends the immediate value for the given instruction format.
 *
 * @param inst Instruction word.
 * @param fmt Instruction format determining immediate bit layout.
 *
 * @return Sign-extended 32-bit immediate value, or 0 for formats without immediates.
 *
 */
int32_t inst_get_imm(inst_t inst, inst_format_t fmt);

#endif // __INSTRUCTION_H__
