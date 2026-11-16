#include "instruction.h"

#define BITS(x, hi, lo) (((x) >> (lo)) & ((1u << ((hi) - (lo) + 1)) - 1))


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
