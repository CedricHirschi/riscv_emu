#ifndef __INSTRUCTION_H__
#define __INSTRUCTION_H__

#include <stdint.h>

typedef enum {
    OP_R,
    OP_I,
    OP_S,
    OP_B,
    OP_U,
    OP_J
} inst_format_t;

typedef struct {
    uint32_t opcode : 7;
    uint32_t rd     : 5;
    uint32_t funct3 : 3;
    uint32_t rs1    : 5;
    uint32_t rs2    : 5;
    uint32_t funct7 : 7;
} inst_r_t;

typedef struct {
    uint32_t opcode : 7;
    uint32_t rd     : 5;
    uint32_t funct3 : 3;
    uint32_t rs1    : 5;
    uint32_t imm    : 12;
} inst_i_t;

typedef struct {
    uint32_t opcode   : 7;
    uint32_t imm_4_0  : 5;
    uint32_t funct3   : 3;
    uint32_t rs1      : 5;
    uint32_t rs2      : 5;
    uint32_t imm_11_5 : 7;
} inst_s_t;

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

typedef struct {
    uint32_t opcode    : 7;
    uint32_t rd        : 5;
    uint32_t imm_31_12 : 20;
} inst_u_t;

typedef struct {
    uint32_t opcode    : 7;
    uint32_t rd        : 5;
    uint32_t imm_19_12 : 8;
    uint32_t imm_11    : 1;
    uint32_t imm_10_1  : 10;
    uint32_t imm_20    : 1;
} inst_j_t;

typedef union {
    uint32_t val;
    inst_r_t r;
    inst_i_t i;
    inst_s_t s;
    inst_b_t b;
    inst_u_t u;
    inst_j_t j;
} inst_t;

inst_format_t inst_get_format(inst_t inst);

uint32_t inst_get_opcode(inst_t inst);
uint32_t inst_get_rd(inst_t inst);
uint32_t inst_get_funct3(inst_t inst);
uint32_t inst_get_rs1(inst_t inst);
uint32_t inst_get_rs2(inst_t inst);
uint32_t inst_get_funct7(inst_t inst);
int32_t inst_get_imm(inst_t inst);

#endif // __INSTRUCTION_H__
