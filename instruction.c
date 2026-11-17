#include "instruction.h"

#define BITS(x, hi, lo) (((x) >> (lo)) & ((1u << ((hi) - (lo) + 1)) - 1))

static const uint8_t R_BASE[8]  = { INS_ADD, INS_SLL, INS_SLT, INS_SLTU, INS_XOR, INS_SRL, INS_OR, INS_AND };
static const uint8_t R_MUL[8]   = { INS_MUL, INS_MULH, INS_MULHSU, INS_MULHU, INS_DIV, INS_DIVU, INS_REM, INS_REMU };
static const uint8_t I_ALU[8]   = { INS_ADDI, INS_SLLI, INS_SLTI, INS_SLTIU, INS_XORI, INS_SRLI, INS_ORI, INS_ANDI };
static const uint8_t LOADS[8]   = { INS_LB, INS_LH, INS_LW, INS_ILLEGAL, INS_LBU, INS_LHU, INS_ILLEGAL, INS_ILLEGAL };
static const uint8_t STORES[8]  = { INS_SB, INS_SH, INS_SW, INS_ILLEGAL, INS_ILLEGAL, INS_ILLEGAL, INS_ILLEGAL, INS_ILLEGAL };
static const uint8_t BRANCH[8]  = { INS_BEQ, INS_BNE, INS_ILLEGAL, INS_ILLEGAL, INS_BLT, INS_BGE, INS_BLTU, INS_BGEU };
static const uint8_t CSR_OPS[8] = { INS_ILLEGAL, INS_CSRRW, INS_CSRRS, INS_CSRRC, INS_ILLEGAL, INS_CSRRWI, INS_CSRRSI, INS_CSRRCI };

inst_format_t inst_get_format(uint32_t inst) {
    switch (inst_get_opcode(inst)) {
        case 0x33: return OP_R;

        case 0x13: return OP_I;

        case 0x03: return OP_I;
        
        case 0x23: return OP_S;

        case 0x63: return OP_B;

        case 0x6F: return OP_J;
        case 0x67: return OP_I;

        case 0x37: return OP_U;
        case 0x17: return OP_U;

        case 0x73: return OP_I;

        default: return OP_INVALID;
    }
}

uint32_t inst_get_opcode(uint32_t inst) {
    return BITS(inst, 6, 0);
}

uint32_t inst_get_rd(uint32_t inst) {
    return BITS(inst, 11, 7);
}

uint32_t inst_get_funct3(uint32_t inst) {
    return BITS(inst, 14, 12);
}

uint32_t inst_get_rs1(uint32_t inst) {
    return BITS(inst, 19, 15);
}

uint32_t inst_get_rs2(uint32_t inst) {
    return BITS(inst, 24, 20);
}

uint32_t inst_get_funct7(uint32_t inst) {
    return BITS(inst, 31, 25);
}

int32_t inst_get_imm(uint32_t w, inst_format_t fmt) {
    // Shift left then arithmetic right-shift to sign-extend 12-, 13-, and 21-bit immediates
    switch (fmt) {
        case OP_I:
            return (int32_t)(BITS(w, 31, 20) << 20) >> 20;
        case OP_S:
            return (int32_t)(((BITS(w, 31, 25) << 5) | BITS(w, 11, 7)) << 20) >> 20;
        case OP_B:
            return (int32_t)(((BITS(w, 31, 31) << 12) | (BITS(w, 7, 7) << 11) | (BITS(w, 30, 25) << 5) | (BITS(w, 11, 8) << 1)) << 19) >> 19;
        case OP_U:
            return (int32_t)(BITS(w, 31, 12) << 12);
        case OP_J:
            return (int32_t)(((BITS(w, 31, 31) << 20) | (BITS(w, 19, 12) << 12) | (BITS(w, 20, 20) << 11) | (BITS(w, 30, 21) << 1)) << 11) >> 11;
        default:
            return 0;
    }
}

inst_decoded_t inst_decode(uint32_t word)
{
    uint32_t funct3 = inst_get_funct3(word);
    uint32_t funct7 = inst_get_funct7(word);
    inst_decoded_t d = {
        .op = INS_ILLEGAL,
        .rd = inst_get_rd(word),
        .rs1 = inst_get_rs1(word),
        .rs2 = inst_get_rs2(word),
        .imm = inst_get_imm(word, inst_get_format(word)),
    };

    switch (inst_get_opcode(word)) {
        case 0x33:
            if (funct7 == 0x00) d.op = R_BASE[funct3];
            else if (funct7 == 0x01) d.op = R_MUL[funct3];
            else if (funct7 == 0x20 && funct3 == 0x0) d.op = INS_SUB;
            else if (funct7 == 0x20 && funct3 == 0x5) d.op = INS_SRA;
            break;
        case 0x13:
            d.op = I_ALU[funct3];
            // Shift immediates encode funct7 in bits [11:5] (0x400 = SRAI); keep only the 5-bit shamt
            if (funct3 == 0x5 && (d.imm & 0xFE0) == 0x400) d.op = INS_SRAI;
            else if ((funct3 == 0x1 || funct3 == 0x5) && (d.imm & 0xFE0) != 0) d.op = INS_ILLEGAL;
            if (funct3 == 0x1 || funct3 == 0x5) d.imm &= 0x1F;
            break;
        case 0x03: d.op = LOADS[funct3]; break;
        case 0x23: d.op = STORES[funct3]; break;
        case 0x63: d.op = BRANCH[funct3]; break;
        case 0x37: d.op = INS_LUI; break;
        case 0x17: d.op = INS_AUIPC; break;
        case 0x6F: d.op = INS_JAL; break;
        case 0x67: d.op = INS_JALR; break;
        case 0x73:
            d.imm &= 0xFFF; // CSR number / funct12 is unsigned
            d.op = CSR_OPS[funct3];
            if (funct3 == 0 && d.rd == 0 && d.rs1 == 0) {
                if (d.imm == 0x000) d.op = INS_ECALL;
                else if (d.imm == 0x001) d.op = INS_EBREAK;
                else if (d.imm == 0x302) d.op = INS_MRET;
            }
            break;
    }
    return d;
}
