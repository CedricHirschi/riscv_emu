#include "instruction.h"


inst_format_t inst_get_format(inst_t inst) {
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

uint32_t inst_get_opcode(inst_t inst) {
    // Always same location in all formats
    return inst.val & 0x7f;
}

uint32_t inst_get_rd(inst_t inst) {
    // Always same location in all formats
    return (inst.val >> 7) & 0x1f;
}

uint32_t inst_get_funct3(inst_t inst) {
    // Always same location in all formats
    return (inst.val >> 12) & 0x7;
}

uint32_t inst_get_rs1(inst_t inst) {
    // Always same location in all formats
    return (inst.val >> 15) & 0x1f;
}

uint32_t inst_get_rs2(inst_t inst) {
    // Always same location in all formats
    return (inst.val >> 20) & 0x1f;
}

uint32_t inst_get_funct7(inst_t inst) {
    // Always same location in all formats
    return (inst.val >> 25) & 0x7f;
}

int32_t inst_get_imm(inst_t inst) {
    switch (inst_get_format(inst)) {
        case OP_I:
            return (int32_t)(inst.i.imm << 20) >> 20;
        case OP_S:
            return (int32_t)(((inst.s.imm_11_5 << 5) | inst.s.imm_4_0) << 20) >> 20;
        case OP_B:
            return (int32_t)(((inst.b.imm_12 << 12) | (inst.b.imm_11 << 11) | (inst.b.imm_10_5 << 5) | (inst.b.imm_4_1 << 1)) << 19) >> 19;
        case OP_U:
            return (int32_t)(inst.u.imm_31_12 << 12);
        case OP_J:
            return (int32_t)(((inst.j.imm_20 << 20) | (inst.j.imm_19_12 << 12) | (inst.j.imm_11 << 11) | (inst.j.imm_10_1 << 1)) << 11) >> 11;
        default:
            return 0;
    }
}
