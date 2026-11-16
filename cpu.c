#include "cpu.h"

#include <stdio.h>
#include <stdlib.h>

#include "instruction.h"
#include "syscall.h"

enum {
    INS_UNDECODED = 0, // calloc'd icache starts zero-initialized
    INS_ILLEGAL,
    INS_ADD, INS_SUB, INS_SLL, INS_SLT, INS_SLTU, INS_XOR, INS_SRL, INS_SRA, INS_OR, INS_AND,
    INS_MUL, INS_MULH, INS_MULHSU, INS_MULHU, INS_DIV, INS_DIVU, INS_REM, INS_REMU,
    INS_ADDI, INS_SLTI, INS_SLTIU, INS_XORI, INS_ORI, INS_ANDI, INS_SLLI, INS_SRLI, INS_SRAI,
    INS_LB, INS_LH, INS_LW, INS_LBU, INS_LHU,
    INS_SB, INS_SH, INS_SW,
    INS_BEQ, INS_BNE, INS_BLT, INS_BGE, INS_BLTU, INS_BGEU,
    INS_LUI, INS_AUIPC, INS_JAL, INS_JALR,
    INS_ECALL, INS_EBREAK,
};

struct decoded {
    uint8_t op, rd, rs1, rs2;
    int32_t imm;
};

static const uint8_t R_BASE[8]  = { INS_ADD, INS_SLL, INS_SLT, INS_SLTU, INS_XOR, INS_SRL, INS_OR, INS_AND };
static const uint8_t R_MUL[8]   = { INS_MUL, INS_MULH, INS_MULHSU, INS_MULHU, INS_DIV, INS_DIVU, INS_REM, INS_REMU };
static const uint8_t I_ALU[8]   = { INS_ADDI, INS_SLLI, INS_SLTI, INS_SLTIU, INS_XORI, INS_SRLI, INS_ORI, INS_ANDI };
static const uint8_t LOADS[8]   = { INS_LB, INS_LH, INS_LW, INS_ILLEGAL, INS_LBU, INS_LHU, INS_ILLEGAL, INS_ILLEGAL };
static const uint8_t STORES[8]  = { INS_SB, INS_SH, INS_SW, INS_ILLEGAL, INS_ILLEGAL, INS_ILLEGAL, INS_ILLEGAL, INS_ILLEGAL };
static const uint8_t BRANCH[8]  = { INS_BEQ, INS_BNE, INS_ILLEGAL, INS_ILLEGAL, INS_BLT, INS_BGE, INS_BLTU, INS_BGEU };

static struct decoded decode(uint32_t word)
{
    uint32_t funct3 = inst_get_funct3(word);
    uint32_t funct7 = inst_get_funct7(word);
    struct decoded d = {
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
            if (d.imm == 0) d.op = INS_ECALL;
            else if (d.imm == 1) d.op = INS_EBREAK;
            break;
    }
    return d;
}

bool cpu_init(cpu_t *cpu, const memory_t *mem, uint32_t pc)
{
    // Round up so a trailing partial word still has an icache slot for invalidation
    *cpu = (cpu_t){ .pc = pc, .icache = calloc((mem->size + 3) / 4, sizeof(struct decoded)) };
    return cpu->icache != NULL;
}

// Invalidate first and last touched words (stores are <= 4 bytes, so at most two words)
static void invalidate(cpu_t *cpu, uint32_t addr, uint32_t len)
{
    cpu->icache[addr >> 2].op = INS_UNDECODED;
    cpu->icache[(addr + len - 1) >> 2].op = INS_UNDECODED;
}

step_t cpu_step(cpu_t *cpu, memory_t *mem)
{
    uint32_t *x = cpu->regs.arr;
    uint32_t pc = cpu->pc;

    if (pc & 3) {
        fprintf(stderr, "Misaligned PC=0x%08x\n", pc);
        return STEP_ERROR;
    }
    if (pc >> 2 >= mem->size / 4) {
        fprintf(stderr, "Failed to read memory at PC=0x%08x\n", pc);
        return STEP_ERROR;
    }

    struct decoded *d = &cpu->icache[pc >> 2];
    if (d->op == INS_UNDECODED) {
        uint32_t word = 0;
        memory_read_word(mem, pc, &word);
        *d = decode(word);
    }

    uint32_t a = x[d->rs1], b = x[d->rs2];
    uint32_t addr = a + d->imm;
    uint32_t next = pc + 4;

    switch (d->op) {
        case INS_ADD:  x[d->rd] = a + b; break;
        case INS_SUB:  x[d->rd] = a - b; break;
        case INS_SLL:  x[d->rd] = a << (b & 0x1F); break;
        case INS_SLT:  x[d->rd] = (int32_t)a < (int32_t)b; break;
        case INS_SLTU: x[d->rd] = a < b; break;
        case INS_XOR:  x[d->rd] = a ^ b; break;
        case INS_SRL:  x[d->rd] = a >> (b & 0x1F); break;
        case INS_SRA:  x[d->rd] = (int32_t)a >> (b & 0x1F); break;
        case INS_OR:   x[d->rd] = a | b; break;
        case INS_AND:  x[d->rd] = a & b; break;

        case INS_MUL:    x[d->rd] = a * b; break;
        case INS_MULH:   x[d->rd] = ((int64_t)(int32_t)a * (int64_t)(int32_t)b) >> 32; break;
        case INS_MULHSU: x[d->rd] = ((int64_t)(int32_t)a * (uint64_t)b) >> 32; break;
        case INS_MULHU:  x[d->rd] = ((uint64_t)a * (uint64_t)b) >> 32; break;

        // RISC-V defines div-by-zero and INT32_MIN / -1 results instead of trapping
        case INS_DIV:  x[d->rd] = b == 0 ? 0xFFFFFFFF : ((int32_t)a == INT32_MIN && (int32_t)b == -1) ? a : (uint32_t)((int32_t)a / (int32_t)b); break;
        case INS_DIVU: x[d->rd] = b == 0 ? 0xFFFFFFFF : a / b; break;
        case INS_REM:  x[d->rd] = b == 0 ? a : ((int32_t)a == INT32_MIN && (int32_t)b == -1) ? 0 : (uint32_t)((int32_t)a % (int32_t)b); break;
        case INS_REMU: x[d->rd] = b == 0 ? a : a % b; break;

        case INS_ADDI:  x[d->rd] = a + d->imm; break;
        case INS_SLTI:  x[d->rd] = (int32_t)a < d->imm; break;
        case INS_SLTIU: x[d->rd] = a < (uint32_t)d->imm; break;
        case INS_XORI:  x[d->rd] = a ^ d->imm; break;
        case INS_ORI:   x[d->rd] = a | d->imm; break;
        case INS_ANDI:  x[d->rd] = a & d->imm; break;
        case INS_SLLI:  x[d->rd] = a << d->imm; break;
        case INS_SRLI:  x[d->rd] = a >> d->imm; break;
        case INS_SRAI:  x[d->rd] = (int32_t)a >> d->imm; break;

        case INS_LB: case INS_LBU: {
            uint8_t v;
            if (!memory_read_byte(mem, addr, &v)) goto read_failed;
            x[d->rd] = d->op == INS_LB ? (uint32_t)(int8_t)v : v;
        } break;
        case INS_LH: case INS_LHU: {
            uint16_t v;
            if (!memory_read_half(mem, addr, &v)) goto read_failed;
            x[d->rd] = d->op == INS_LH ? (uint32_t)(int16_t)v : v;
        } break;
        case INS_LW:
            if (!memory_read_word(mem, addr, &x[d->rd])) goto read_failed;
            break;

        case INS_SB:
            if (!memory_write_byte(mem, addr, b & 0xFF)) goto write_failed;
            invalidate(cpu, addr, 1);
            break;
        case INS_SH:
            if (!memory_write_half(mem, addr, b & 0xFFFF)) goto write_failed;
            invalidate(cpu, addr, 2);
            break;
        case INS_SW:
            if (!memory_write_word(mem, addr, b)) goto write_failed;
            invalidate(cpu, addr, 4);
            break;

        case INS_BEQ:  if (a == b) next = pc + d->imm; break;
        case INS_BNE:  if (a != b) next = pc + d->imm; break;
        case INS_BLT:  if ((int32_t)a < (int32_t)b) next = pc + d->imm; break;
        case INS_BGE:  if ((int32_t)a >= (int32_t)b) next = pc + d->imm; break;
        case INS_BLTU: if (a < b) next = pc + d->imm; break;
        case INS_BGEU: if (a >= b) next = pc + d->imm; break;

        case INS_LUI:   x[d->rd] = d->imm; break;
        case INS_AUIPC: x[d->rd] = pc + d->imm; break;
        case INS_JAL:   x[d->rd] = pc + 4; next = pc + d->imm; break;
        case INS_JALR:  next = (a + d->imm) & ~1u; x[d->rd] = pc + 4; break; // clear bit 0 per spec; a read before rd write (rd may equal rs1)

        case INS_ECALL: {
            step_t r = syscall_handle(&cpu->regs, mem);
            if (r != STEP_OK) {
                return r;
            }
        } break;
        case INS_EBREAK:
            fprintf(stderr, "EBREAK encountered\n");
            return STEP_ERROR;

        default: {
            uint32_t word = 0;
            memory_read_word(mem, pc, &word);
            fprintf(stderr, "Illegal instruction 0x%08x at PC=0x%08x\n", word, pc);
            return STEP_ERROR;
        }
    }

    cpu->pc = next;
    x[0] = 0;
    return STEP_OK;

read_failed:
    fprintf(stderr, "Memory read failed\n");
    return STEP_ERROR;
write_failed:
    fprintf(stderr, "Memory write failed\n");
    return STEP_ERROR;
}
