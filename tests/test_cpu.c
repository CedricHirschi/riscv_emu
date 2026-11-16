#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "cpu.h"

// Encoders written from the spec, independent of instruction.h
#define R(f7, f3, rd, rs1, rs2) ((uint32_t)(f7) << 25 | (rs2) << 20 | (rs1) << 15 | (f3) << 12 | (rd) << 7 | 0x33)
#define I(op, f3, rd, rs1, imm) (((uint32_t)(imm) & 0xFFF) << 20 | (rs1) << 15 | (f3) << 12 | (rd) << 7 | (op))
#define S(f3, rs1, rs2, imm) ((((uint32_t)(imm) >> 5) & 0x7F) << 25 | (rs2) << 20 | (rs1) << 15 | (f3) << 12 | ((uint32_t)(imm) & 0x1F) << 7 | 0x23)
#define B(f3, rs1, rs2, imm) ((((uint32_t)(imm) >> 12) & 1) << 31 | (((uint32_t)(imm) >> 5) & 0x3F) << 25 | (rs2) << 20 | (rs1) << 15 | \
                              (f3) << 12 | (((uint32_t)(imm) >> 1) & 0xF) << 8 | (((uint32_t)(imm) >> 11) & 1) << 7 | 0x63)
#define U(op, rd, imm) (((uint32_t)(imm) & 0xFFFFF000) | (rd) << 7 | (op))
#define J(rd, imm) ((((uint32_t)(imm) >> 20) & 1) << 31 | (((uint32_t)(imm) >> 1) & 0x3FF) << 21 | (((uint32_t)(imm) >> 11) & 1) << 20 | \
                    (((uint32_t)(imm) >> 12) & 0xFF) << 12 | (rd) << 7 | 0x6F)

#define MEM_SIZE 256

static memory_t mem;
static cpu_t cpu;

// Fresh zeroed memory and decode cache, with word placed at pc
static void reset(uint32_t pc, uint32_t word) {
    free(cpu.icache);
    memset(mem.data, 0, MEM_SIZE);
    memory_write_word(&mem, pc, word);
    assert(cpu_init(&cpu, &mem, pc));
}

// Runs one R/I-type instruction with x1=a, x2=b and returns x3
static uint32_t alu(uint32_t word, uint32_t a, uint32_t b) {
    reset(0, word);
    cpu.regs.arr[1] = a;
    cpu.regs.arr[2] = b;
    assert(cpu_step(&cpu, &mem) == STEP_OK);
    assert(cpu.pc == 4);
    return cpu.regs.arr[3];
}

static void expect_error(uint32_t pc, uint32_t word) {
    reset(pc & ~3u, word);
    cpu.pc = pc;
    assert(cpu_step(&cpu, &mem) == STEP_ERROR);
    assert(cpu.pc == pc);
}

void test_r_type() {
    assert(alu(R(0x00, 0, 3, 1, 2), 5, 7) == 12);                      // ADD
    assert(alu(R(0x20, 0, 3, 1, 2), 5, 7) == 0xFFFFFFFEu);             // SUB
    assert(alu(R(0x00, 1, 3, 1, 2), 1, 33) == 2);                      // SLL masks to 5 bits
    assert(alu(R(0x00, 2, 3, 1, 2), 0xFFFFFFFFu, 1) == 1);             // SLT signed
    assert(alu(R(0x00, 3, 3, 1, 2), 0xFFFFFFFFu, 1) == 0);             // SLTU unsigned
    assert(alu(R(0x00, 4, 3, 1, 2), 0xF0F0, 0xFF00) == 0x0FF0);        // XOR
    assert(alu(R(0x00, 5, 3, 1, 2), 0x80000000u, 4) == 0x08000000u);   // SRL
    assert(alu(R(0x20, 5, 3, 1, 2), 0x80000000u, 36) == 0xF8000000u);  // SRA masks to 5 bits
    assert(alu(R(0x00, 6, 3, 1, 2), 0xF0, 0x0F) == 0xFF);              // OR
    assert(alu(R(0x00, 7, 3, 1, 2), 0xF0, 0x3C) == 0x30);              // AND

    expect_error(0, R(0x20, 1, 3, 1, 2)); // funct7 0x20 only valid for SUB/SRA
    expect_error(0, R(0x02, 0, 3, 1, 2));
}

void test_m_ext() {
    assert(alu(R(1, 0, 3, 1, 2), (uint32_t)-3, 5) == (uint32_t)-15);                   // MUL
    assert(alu(R(1, 1, 3, 1, 2), 0x80000000u, 0x80000000u) == 0x40000000u);           // MULH
    assert(alu(R(1, 1, 3, 1, 2), 0xFFFFFFFFu, 2) == 0xFFFFFFFFu);                      // MULH -1*2
    assert(alu(R(1, 2, 3, 1, 2), 0xFFFFFFFFu, 0xFFFFFFFFu) == 0xFFFFFFFFu);            // MULHSU -1*(2^32-1)
    assert(alu(R(1, 3, 3, 1, 2), 0xFFFFFFFFu, 0xFFFFFFFFu) == 0xFFFFFFFEu);            // MULHU

    assert(alu(R(1, 4, 3, 1, 2), 7, (uint32_t)-2) == (uint32_t)-3);                   // DIV truncates
    assert(alu(R(1, 6, 3, 1, 2), (uint32_t)-7, 2) == (uint32_t)-1);                   // REM sign of dividend
    assert(alu(R(1, 5, 3, 1, 2), 0xFFFFFFFFu, 2) == 0x7FFFFFFFu);                      // DIVU
    assert(alu(R(1, 7, 3, 1, 2), 0xFFFFFFFFu, 10) == 5);                               // REMU

    // Division by zero and overflow, as the spec defines them
    assert(alu(R(1, 4, 3, 1, 2), 42, 0) == 0xFFFFFFFFu);
    assert(alu(R(1, 5, 3, 1, 2), 42, 0) == 0xFFFFFFFFu);
    assert(alu(R(1, 6, 3, 1, 2), 42, 0) == 42);
    assert(alu(R(1, 7, 3, 1, 2), 42, 0) == 42);
    assert(alu(R(1, 4, 3, 1, 2), 0x80000000u, 0xFFFFFFFFu) == 0x80000000u);
    assert(alu(R(1, 6, 3, 1, 2), 0x80000000u, 0xFFFFFFFFu) == 0);
}

void test_i_type() {
    assert(alu(I(0x13, 0, 3, 1, -6), 5, 0) == 0xFFFFFFFFu);              // ADDI negative imm
    assert(alu(I(0x13, 2, 3, 1, 0), 0xFFFFFFFFu, 0) == 1);               // SLTI signed
    assert(alu(I(0x13, 3, 3, 1, -1), 5, 0) == 1);                        // SLTIU imm sign-extends then compares unsigned
    assert(alu(I(0x13, 4, 3, 1, -1), 0x1234, 0) == ~0x1234u);            // XORI
    assert(alu(I(0x13, 6, 3, 1, 0x0F0), 0x0F, 0) == 0xFF);               // ORI
    assert(alu(I(0x13, 7, 3, 1, -16), 0x12345678u, 0) == 0x12345670u);   // ANDI
    assert(alu(I(0x13, 1, 3, 1, 31), 1, 0) == 0x80000000u);              // SLLI
    assert(alu(I(0x13, 5, 3, 1, 31), 0x80000000u, 0) == 1);              // SRLI
    assert(alu(I(0x13, 5, 3, 1, 0x400 | 31), 0x80000000u, 0) == 0xFFFFFFFFu); // SRAI

    expect_error(0, I(0x13, 5, 3, 1, 0x200 | 1)); // bad shift funct7
    expect_error(0, I(0x13, 1, 3, 1, 0x400 | 1)); // SLLI has no arithmetic variant
    expect_error(0, I(0x13, 1, 3, 1, 0x020 | 1)); // shamt[5] set is illegal on RV32
}

void test_x0() {
    reset(0, I(0x13, 0, 0, 0, 5)); // ADDI x0, x0, 5
    assert(cpu_step(&cpu, &mem) == STEP_OK);
    assert(cpu.regs.arr[0] == 0);
}

void test_loads() {
    const uint8_t data[4] = { 0x80, 0xFF, 0x34, 0x12 };
    struct { uint32_t f3, want; } cases[] = {
        { 0, 0xFFFFFF80u }, // LB
        { 4, 0x80 },        // LBU
        { 1, 0xFFFFFF80u }, // LH
        { 5, 0xFF80 },      // LHU
        { 2, 0x1234FF80u }, // LW
    };
    for (size_t i = 0; i < sizeof cases / sizeof cases[0]; i++) {
        reset(0, I(0x03, cases[i].f3, 3, 1, -16));
        memcpy(mem.data + 128, data, 4);
        cpu.regs.arr[1] = 144;
        assert(cpu_step(&cpu, &mem) == STEP_OK);
        assert(cpu.regs.arr[3] == cases[i].want);
    }

    reset(0, I(0x03, 2, 3, 1, 0));
    cpu.regs.arr[1] = MEM_SIZE - 2;
    assert(cpu_step(&cpu, &mem) == STEP_ERROR);
    expect_error(0, I(0x03, 3, 3, 1, 0)); // no LD on RV32
}

void test_stores() {
    uint32_t want[3] = { 0xDD, 0xCCDD, 0xAABBCCDD };
    for (uint32_t f3 = 0; f3 < 3; f3++) {
        reset(0, S(f3, 1, 2, -4));
        cpu.regs.arr[1] = 132;
        cpu.regs.arr[2] = 0xAABBCCDDu;
        assert(cpu_step(&cpu, &mem) == STEP_OK);
        uint32_t got = 0;
        assert(memory_read_word(&mem, 128, &got));
        assert(got == want[f3]);
    }

    reset(0, S(2, 1, 2, 0));
    cpu.regs.arr[1] = MEM_SIZE;
    assert(cpu_step(&cpu, &mem) == STEP_ERROR);
}

void test_branches() {
    struct { uint32_t f3, a, b; int taken; } cases[] = {
        { 0, 5, 5, 1 }, { 0, 5, 6, 0 },                     // BEQ
        { 1, 5, 6, 1 }, { 1, 5, 5, 0 },                     // BNE
        { 4, 0xFFFFFFFFu, 1, 1 }, { 4, 1, 0xFFFFFFFFu, 0 }, // BLT signed
        { 5, 1, 0xFFFFFFFFu, 1 }, { 5, 3, 3, 1 },           // BGE
        { 5, 0xFFFFFFFFu, 1, 0 },
        { 6, 1, 0xFFFFFFFFu, 1 }, { 6, 0xFFFFFFFFu, 1, 0 }, // BLTU unsigned
        { 7, 0xFFFFFFFFu, 1, 1 }, { 7, 1, 0xFFFFFFFFu, 0 }, // BGEU
    };
    for (size_t i = 0; i < sizeof cases / sizeof cases[0]; i++) {
        for (int imm = -16; imm <= 16; imm += 32) { // backward and forward
            reset(64, B(cases[i].f3, 1, 2, imm));
            cpu.regs.arr[1] = cases[i].a;
            cpu.regs.arr[2] = cases[i].b;
            assert(cpu_step(&cpu, &mem) == STEP_OK);
            assert(cpu.pc == (cases[i].taken ? (uint32_t)(64 + imm) : 68));
        }
    }
    expect_error(0, B(2, 1, 2, 8));
}

void test_upper_and_jumps() {
    reset(16, U(0x37, 3, 0xABCDE000u)); // LUI
    assert(cpu_step(&cpu, &mem) == STEP_OK && cpu.regs.arr[3] == 0xABCDE000u);

    reset(16, U(0x17, 3, 0x1000)); // AUIPC
    assert(cpu_step(&cpu, &mem) == STEP_OK && cpu.regs.arr[3] == 0x1010);

    reset(16, J(1, -8)); // JAL backward
    assert(cpu_step(&cpu, &mem) == STEP_OK);
    assert(cpu.regs.arr[1] == 20 && cpu.pc == 8);

    reset(16, I(0x67, 0, 1, 1, 5)); // JALR x1, 5(x1): target uses old x1, low bit cleared
    cpu.regs.arr[1] = 100;
    assert(cpu_step(&cpu, &mem) == STEP_OK);
    assert(cpu.regs.arr[1] == 20 && cpu.pc == 104);
}

void test_system_and_errors() {
    reset(0, 0x00000073); // ECALL exit
    cpu.regs.abi.a7 = 93;
    assert(cpu_step(&cpu, &mem) == STEP_EXIT);

    expect_error(0, 0x00100073); // EBREAK
    expect_error(0, 0x00200073); // unknown SYSTEM
    expect_error(0, 0x0000007F); // unknown opcode
    expect_error(2, 0x00000013); // misaligned pc
    expect_error(MEM_SIZE, 0);   // pc past end
}

void test_self_modifying() {
    uint32_t addi3 = I(0x13, 0, 3, 3, 1);  // x3 += 1
    uint32_t addi4 = I(0x13, 0, 4, 4, 1);  // x4 += 1
    uint32_t new3 = I(0x13, 0, 3, 3, 7);   // x3 += 7
    uint32_t new4 = I(0x13, 0, 5, 4, 1);   // x5 = x4 + 1, differs from addi4 only in the low half
    reset(0, addi3);
    memory_write_word(&mem, 4, addi4);
    memory_write_word(&mem, 8, S(2, 0, 2, 2)); // SW x2, 2(x0): straddles words 0 and 4
    cpu.regs.arr[2] = (new3 >> 16) | (new4 << 16);

    for (int i = 0; i < 3; i++) assert(cpu_step(&cpu, &mem) == STEP_OK);
    assert(cpu.regs.arr[3] == 1 && cpu.regs.arr[4] == 1);

    cpu.pc = 0; // rerun both words; stale decodes would give x3 == 2, x4 == 2
    assert(cpu_step(&cpu, &mem) == STEP_OK);
    assert(cpu_step(&cpu, &mem) == STEP_OK);
    assert(cpu.regs.arr[3] == 8 && cpu.regs.arr[4] == 1 && cpu.regs.arr[5] == 2);
}


int main() {
    assert(memory_init(&mem, MEM_SIZE));
    test_r_type();
    test_m_ext();
    test_i_type();
    test_x0();
    test_loads();
    test_stores();
    test_branches();
    test_upper_and_jumps();
    test_system_and_errors();
    test_self_modifying();
    free(cpu.icache);
    memory_free(&mem);
    return 0;
}
