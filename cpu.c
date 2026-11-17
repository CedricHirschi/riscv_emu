#include "cpu.h"

#include <stdio.h>
#include <stdlib.h>

#include "syscall.h"

bool cpu_init(cpu_t *cpu, const memory_t *mem, uint32_t pc)
{
    *cpu = (cpu_t){ .pc = pc, .icache = calloc(mem->rom_size / 4, sizeof(inst_decoded_t))};
    return cpu->icache != NULL;
}

step_t cpu_step(cpu_t *cpu, memory_t *mem)
{
    uint32_t *x = cpu->regs.arr;
    uint32_t pc = cpu->pc;

    if (pc & 3) {
        fprintf(stderr, "Misaligned PC=0x%08x\n", pc);
        return STEP_ERROR;
    }
    uint32_t off = pc - MEMORY_ROM_BASE;
    if (off >= mem->rom_size) {
        fprintf(stderr, "PC=0x%08x outside ROM\n", pc);
        return STEP_ERROR;
    }

    inst_decoded_t *d = &cpu->icache[off >> 2];
    if (d->op == INS_UNDECODED) {
        uint32_t word = 0;
        memory_read_word(mem, pc, &word);
        *d = inst_decode(word);
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
            break;
        case INS_SH:
            if (!memory_write_half(mem, addr, b & 0xFFFF)) goto write_failed;
            break;
        case INS_SW:
            if (!memory_write_word(mem, addr, b)) goto write_failed;
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
