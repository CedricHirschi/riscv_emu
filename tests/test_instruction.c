#include <assert.h>
#include <stdio.h>

#include "instruction.h"


void test_inst_get_opcode() {
    assert(inst_get_opcode(0b0110011) == 0x33);
    assert(inst_get_opcode(0b0010011) == 0x13);
    assert(inst_get_opcode(0b0000011) == 0x03);
    assert(inst_get_opcode(0b0100011) == 0x23);
    assert(inst_get_opcode(0b1100011) == 0x63);
    assert(inst_get_opcode(0b1101111) == 0x6F);
    assert(inst_get_opcode(0b1100111) == 0x67);
    assert(inst_get_opcode(0b0110111) == 0x37);
    assert(inst_get_opcode(0b0010111) == 0x17);
    assert(inst_get_opcode(0b1110011) == 0x73);
}

void test_inst_get_format() {
    assert(inst_get_format(0b0110011) == OP_R);
    assert(inst_get_format(0b0010011) == OP_I);
    assert(inst_get_format(0b0000011) == OP_I);
    assert(inst_get_format(0b0100011) == OP_S);
    assert(inst_get_format(0b1100011) == OP_B);
    assert(inst_get_format(0b1101111) == OP_J);
    assert(inst_get_format(0b1100111) == OP_I);
    assert(inst_get_format(0b0110111) == OP_U);
    assert(inst_get_format(0b0010111) == OP_U);
    assert(inst_get_format(0b1110011) == OP_I);
    assert(inst_get_format(0b1111111) == OP_INVALID);
}

void test_inst_get_rd() {
    assert(inst_get_rd(0b00000000000100000000000010110011) == 1); // rd = 1
    assert(inst_get_rd(0b00000000001000000000000100110011) == 2); // rd = 2
}

void test_inst_get_funct3() {
    uint32_t opcodes[] = { 0b0110011, 0b0010011, 0b0000011, 0b0100011, 0b1100011, 0b1100111, 0b1110011 };
    for (size_t i = 0; i < sizeof(opcodes) / sizeof(opcodes[0]); i++) {
        for (uint32_t funct3 = 0; funct3 < 8; funct3++) {
            uint32_t inst = opcodes[i] | (funct3 << 12);
            assert(inst_get_funct3(inst) == funct3);
        }
    }
}

void test_inst_get_rs1() {
    uint32_t opcodes[] = { 0b0110011, 0b0010011, 0b0000011, 0b0100011, 0b1100011, 0b1100111, 0b1110011 };
    for (size_t i = 0; i < sizeof(opcodes) / sizeof(opcodes[0]); i++) {
        for (uint32_t rs1 = 0; rs1 < 32; rs1++) {
            uint32_t inst = opcodes[i] | (rs1 << 15);
            assert(inst_get_rs1(inst) == rs1);
        }
    }
}

void test_inst_get_rs2() {
    uint32_t opcodes[] = { 0b0110011, 0b0100011, 0b1100011 };
    for (size_t i = 0; i < sizeof(opcodes) / sizeof(opcodes[0]); i++) {
        for (uint32_t rs2 = 0; rs2 < 32; rs2++) {
            uint32_t inst = opcodes[i] | (rs2 << 20);
            assert(inst_get_rs2(inst) == rs2);
        }
    }
}

void test_inst_get_funct7() {
    for (uint32_t funct7 = 0; funct7 < 128; funct7++) {
        uint32_t inst = 0b0110011 | (funct7 << 25);
        assert(inst_get_funct7(inst) == funct7);
    }
}

void test_inst_get_imm() {
    uint32_t i_opcodes[] = { 0b0010011, 0b0000011, 0b1100111, 0b1110011 };
    for (size_t i = 0; i < sizeof(i_opcodes) / sizeof(i_opcodes[0]); i++) {
        for (int32_t imm = -2048; imm < 2048; imm++) {
            uint32_t inst = i_opcodes[i] | (((uint32_t)imm & 0xFFF) << 20);
            assert(inst_get_imm(inst, inst_get_format(inst)) == imm);
        }
    }

    for (int32_t imm = -2048; imm < 2048; imm++) {
        uint32_t imm_4_0 = imm & 0b11111;
        uint32_t imm_11_5 = (imm >> 5) & 0b1111111;
        uint32_t inst = 0b0100011 | (imm_4_0 << 7) | (imm_11_5 << 25);
        assert(inst_get_imm(inst, inst_get_format(inst)) == imm);
    }

    for (int32_t imm = -2048; imm < 2048; imm += 2) {
        uint32_t imm_12 = (imm >> 12) & 0b1;
        uint32_t imm_10_5 = (imm >> 5) & 0b111111;
        uint32_t imm_4_1 = (imm >> 1) & 0b1111;
        uint32_t imm_11 = (imm >> 11) & 0b1;
        uint32_t inst = 0b1100011 | (imm_11 << 7) | (imm_4_1 << 8) | (imm_10_5 << 25) | (imm_12 << 31);
        assert(inst_get_imm(inst, inst_get_format(inst)) == imm);
    }

    for (int32_t imm = -2048; imm < 2048; imm += 2) {
        uint32_t imm_20 = (imm >> 20) & 0b1;
        uint32_t imm_10_1 = (imm >> 1) & 0b1111111111;
        uint32_t imm_11 = (imm >> 11) & 0b1;
        uint32_t imm_19_12 = (imm >> 12) & 0b11111111;
        uint32_t inst = 0b1101111 | (imm_19_12 << 12) | (imm_11 << 20) | (imm_10_1 << 21) | (imm_20 << 31);
        assert(inst_get_imm(inst, inst_get_format(inst)) == imm);
    }

    uint32_t u_opcodes[] = { 0b0110111, 0b0010111 };
    for (size_t i = 0; i < sizeof(u_opcodes) / sizeof(u_opcodes[0]); i++) {
        for (int32_t imm = -0x80000000; imm < 0x7FF000; imm += 0x1000) {
            uint32_t imm_31_12 = (imm >> 12) & 0xFFFFF;
            uint32_t inst = u_opcodes[i] | (imm_31_12 << 12);
            assert(inst_get_imm(inst, inst_get_format(inst)) == imm);
        }
    }
}


int main() {
    test_inst_get_opcode();
	test_inst_get_format();
	test_inst_get_rd();
	test_inst_get_funct3();
	test_inst_get_rs1();
	test_inst_get_rs2();
	test_inst_get_funct7();
	test_inst_get_imm();

	return 0;
}
