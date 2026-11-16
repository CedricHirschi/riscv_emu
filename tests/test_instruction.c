#include <assert.h>
#include <stdio.h>

#include "instruction.h"


void test_inst_get_opcode() {
    inst_t inst;

    inst.val = 0b0110011;
    assert(inst_get_opcode(inst) == 0x33);

    inst.val = 0b0010011;
    assert(inst_get_opcode(inst) == 0x13);

    inst.val = 0b0000011;
    assert(inst_get_opcode(inst) == 0x03);

    inst.val = 0b0100011;
    assert(inst_get_opcode(inst) == 0x23);

    inst.val = 0b1100011;
    assert(inst_get_opcode(inst) == 0x63);

    inst.val = 0b1101111;
    assert(inst_get_opcode(inst) == 0x6F);

    inst.val = 0b1100111;
    assert(inst_get_opcode(inst) == 0x67);

    inst.val = 0b0110111;
    assert(inst_get_opcode(inst) == 0x37);

    inst.val = 0b0010111;
    assert(inst_get_opcode(inst) == 0x17);

    inst.val = 0b1110011;
    assert(inst_get_opcode(inst) == 0x73);
}

void test_inst_get_format() {
    inst_t inst;

    inst.val = 0b0110011;
    assert(inst_get_format(inst) == OP_R);

    inst.val = 0b0010011;
    assert(inst_get_format(inst) == OP_I);

    inst.val = 0b0000011;
    assert(inst_get_format(inst) == OP_I);

    inst.val = 0b0100011;
    assert(inst_get_format(inst) == OP_S);

    inst.val = 0b1100011;
    assert(inst_get_format(inst) == OP_B);

    inst.val = 0b1101111;
    assert(inst_get_format(inst) == OP_J);

    inst.val = 0b1100111;
    assert(inst_get_format(inst) == OP_I);

    inst.val = 0b0110111;
    assert(inst_get_format(inst) == OP_U);

    inst.val = 0b0010111;
    assert(inst_get_format(inst) == OP_U);

    inst.val = 0b1110011;
    assert(inst_get_format(inst) == OP_I);

    inst.val = 0b1111111;
    assert(inst_get_format(inst) == OP_INVALID);
}

void test_inst_get_rd() {
    inst_t inst;

    inst.val = 0b00000000000100000000000010110011; // rd = 1
    assert(inst_get_rd(inst) == 1);

    inst.val = 0b00000000001000000000000100110011; // rd = 2
    assert(inst_get_rd(inst) == 2);
}

void test_inst_get_funct3() {
    inst_t inst;

    inst.r.opcode = 0b0110011;
    for (uint32_t funct3 = 0; funct3 < 8; funct3++) {
        inst.r.funct3 = funct3;
        assert(inst_get_funct3(inst) == funct3);
    }

    inst.i.opcode = 0b0010011;
    for (uint32_t funct3 = 0; funct3 < 8; funct3++) {
        inst.i.funct3 = funct3;
        assert(inst_get_funct3(inst) == funct3);
    }

    inst.i.opcode = 0b0000011;
    for (uint32_t funct3 = 0; funct3 < 8; funct3++) {
        inst.i.funct3 = funct3;
        assert(inst_get_funct3(inst) == funct3);
    }

    inst.s.opcode = 0b0100011;
    for (uint32_t funct3 = 0; funct3 < 8; funct3++) {
        inst.s.funct3 = funct3;
        assert(inst_get_funct3(inst) == funct3);
    }

    inst.b.opcode = 0b1100011;
    for (uint32_t funct3 = 0; funct3 < 8; funct3++) {
        inst.b.funct3 = funct3;
        assert(inst_get_funct3(inst) == funct3);
    }

    inst.i.opcode = 0b1100111;
    for (uint32_t funct3 = 0; funct3 < 8; funct3++) {
        inst.i.funct3 = funct3;
        assert(inst_get_funct3(inst) == funct3);
    }

    inst.i.opcode = 0b1110011;
    for (uint32_t funct3 = 0; funct3 < 8; funct3++) {
        inst.i.funct3 = funct3;
        assert(inst_get_funct3(inst) == funct3);
    }
}

void test_inst_get_rs1() {
    inst_t inst;

    inst.r.opcode = 0b0110011;
    for (uint32_t rs1 = 0; rs1 < 32; rs1++) {
        inst.r.rs1 = rs1;
        assert(inst_get_rs1(inst) == rs1);
    }

    inst.i.opcode = 0b0010011;
    for (uint32_t rs1 = 0; rs1 < 32; rs1++) {
        inst.i.rs1 = rs1;
        assert(inst_get_rs1(inst) == rs1);
    }

    inst.i.opcode = 0b0000011;
    for (uint32_t rs1 = 0; rs1 < 32; rs1++) {
        inst.i.rs1 = rs1;
        assert(inst_get_rs1(inst) == rs1);
    }

    inst.s.opcode = 0b0100011;
    for (uint32_t rs1 = 0; rs1 < 32; rs1++) {
        inst.s.rs1 = rs1;
        assert(inst_get_rs1(inst) == rs1);
    }

    inst.b.opcode = 0b1100011;
    for (uint32_t rs1 = 0; rs1 < 32; rs1++) {
        inst.b.rs1 = rs1;
        assert(inst_get_rs1(inst) == rs1);
    }

    inst.i.opcode = 0b1100111;
    for (uint32_t rs1 = 0; rs1 < 32; rs1++) {
        inst.i.rs1 = rs1;
        assert(inst_get_rs1(inst) == rs1);
    }

    inst.i.opcode = 0b1110011;
    for (uint32_t rs1 = 0; rs1 < 32; rs1++) {
        inst.i.rs1 = rs1;
        assert(inst_get_rs1(inst) == rs1);
    }
}

void test_inst_get_rs2() {
    inst_t inst;

    inst.r.opcode = 0b0110011;
    for (uint32_t rs2 = 0; rs2 < 32; rs2++) {
        inst.r.rs2 = rs2;
        assert(inst_get_rs2(inst) == rs2);
    }

    inst.s.opcode = 0b0100011;
    for (uint32_t rs2 = 0; rs2 < 32; rs2++) {
        inst.s.rs2 = rs2;
        assert(inst_get_rs2(inst) == rs2);
    }

    inst.b.opcode = 0b1100011;
    for (uint32_t rs2 = 0; rs2 < 32; rs2++) {
        inst.b.rs2 = rs2;
        assert(inst_get_rs2(inst) == rs2);
    }
}

void test_inst_get_funct7() {
    inst_t inst;

    inst.r.opcode = 0b0110011;
    for (uint32_t funct7 = 0; funct7 < 128; funct7++) {
        inst.r.funct7 = funct7;
        assert(inst_get_funct7(inst) == funct7);
    }
}

void test_inst_get_imm() {
    inst_t inst;

    inst.i.opcode = 0b0010011;
    for (int32_t imm = -2048; imm < 2048; imm++) {
        inst.i.imm = imm & 0xFFF;
        assert(inst_get_imm(inst) == imm);
    }

    inst.i.opcode = 0b0000011;
    for (int32_t imm = -2048; imm < 2048; imm++) {
        inst.i.imm = imm & 0xFFF;
        assert(inst_get_imm(inst) == imm);
    }

    inst.s.opcode = 0b0100011;
    for (int32_t imm = -2048; imm < 2048; imm++) {
        uint32_t imm_4_0 = imm & 0b11111;
        uint32_t imm_11_5 = (imm >> 5) & 0b1111111;
        inst.s.imm_4_0 = imm_4_0;
        inst.s.imm_11_5 = imm_11_5;
        assert(inst_get_imm(inst) == imm);
    }

    inst.b.opcode = 0b1100011;
    for (int32_t imm = -2048; imm < 2048; imm += 2) {
        uint32_t imm_12 = (imm >> 12) & 0b1;
        uint32_t imm_10_5 = (imm >> 5) & 0b111111;
        uint32_t imm_4_1 = (imm >> 1) & 0b1111;
        uint32_t imm_11 = (imm >> 11) & 0b1;
        inst.b.imm_12 = imm_12;
        inst.b.imm_10_5 = imm_10_5;
        inst.b.imm_4_1 = imm_4_1;
        inst.b.imm_11 = imm_11;
        assert(inst_get_imm(inst) == imm);
    }

    inst.j.opcode = 0b1101111;
    for (int32_t imm = -2048; imm < 2048; imm += 2) {
        uint32_t imm_20 = (imm >> 20) & 0b1;
        uint32_t imm_10_1 = (imm >> 1) & 0b1111111111;
        uint32_t imm_11 = (imm >> 11) & 0b1;
        uint32_t imm_19_12 = (imm >> 12) & 0b11111111;
        inst.j.imm_20 = imm_20;
        inst.j.imm_10_1 = imm_10_1;
        inst.j.imm_11 = imm_11;
        inst.j.imm_19_12 = imm_19_12;
        assert(inst_get_imm(inst) == imm);
    }

    inst.i.opcode = 0b1100111;
    for (int32_t imm = -2048; imm < 2048; imm++) {
        inst.i.imm = imm & 0xFFF;
        assert(inst_get_imm(inst) == imm);
    }

    inst.u.opcode = 0b0110111;
    for (int32_t imm = -0x80000000; imm < 0x7FF000; imm += 0x1000) {
        int32_t imm_31_12 = (imm >> 12) & 0xFFFFF;
        inst.u.imm_31_12 = imm_31_12;
        assert(inst_get_imm(inst) == imm);
    }

    inst.u.opcode = 0b0010111;
    for (int32_t imm = -0x80000000; imm < 0x7FF000; imm += 0x1000) {
        inst.u.imm_31_12 = (imm >> 12) & 0xFFFFF;
        assert(inst_get_imm(inst) == imm);
    }

    inst.i.opcode = 0b1110011;
    for (int32_t imm = -2048; imm < 2048; imm++) {
        inst.i.imm = imm;
        assert(inst_get_imm(inst) == imm);
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
