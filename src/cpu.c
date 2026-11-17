#include "cpu.h"

#include <stdio.h>
#include <stdlib.h>

#include "syscall.h"

#define JUMP(t) do { uint32_t t_ = (t); if (t_ & 3) return cpu_trap(cpu, CAUSE_FETCH_MISALIGNED, t_); next = t_; } while (0)

bool cpu_init(cpu_t *cpu, const memory_t *mem, uint32_t pc)
{
    *cpu = (cpu_t){ .pc = pc, .icache = calloc(mem->rom_size / 4, sizeof(inst_decoded_t))};
    return cpu->icache != NULL;
}

static step_t cpu_trap(cpu_t *cpu, uint32_t cause, uint32_t tval)
{
    if (cpu->csr.mtvec == 0) { // no handler installed (_start sits at 0, so 0 is never a handler)
        fprintf(stderr, "Unhandled trap cause=%u tval=0x%08x at PC=0x%08x\n", cause, tval, cpu->pc);
        return STEP_ERROR;
    }

    uint32_t ms = cpu->csr.mstatus;
    cpu->csr.mstatus = (ms & ~0x88u) | ((ms & 0x8) << 4); // MPIE = MIE, MIE = 0
    cpu->csr.mepc = cpu->pc;
    cpu->csr.mcause = cause;
    cpu->csr.mtval = tval;
    cpu->pc = cpu->csr.mtvec;

    return STEP_OK;
}

static bool cpu_csr_read(const cpu_t *cpu, uint32_t num, uint32_t *v)
{
    switch (num) {
        case 0x300: *v = cpu->csr.mstatus | 0x1800; return true; // MPP hardwired to M
        case 0x301: *v = 0x40001100; return true;                // misa: RV32 I M
        case 0x304: *v = cpu->csr.mie; return true;
        case 0x305: *v = cpu->csr.mtvec; return true;
        case 0x340: *v = cpu->csr.mscratch; return true;
        case 0x341: *v = cpu->csr.mepc; return true;
        case 0x342: *v = cpu->csr.mcause; return true;
        case 0x343: *v = cpu->csr.mtval; return true;
        case 0x344: *v = cpu->csr.mip; return true;
        case 0xF14: *v = 0; return true;                         // mhartid
        default: return false;
    }
}

static bool cpu_csr_write(cpu_t *cpu, uint32_t num, uint32_t v)
{
    switch (num) {
        case 0x300: cpu->csr.mstatus = v & 0x88; return true;    // only MIE/MPIE are writable
        case 0x304: cpu->csr.mie = v; return true;
        case 0x305: cpu->csr.mtvec = v & ~3u; return true;       // direct mode only for now
        case 0x340: cpu->csr.mscratch = v; return true;
        case 0x341: cpu->csr.mepc = v & ~3u; return true;
        case 0x342: cpu->csr.mcause = v; return true;
        case 0x343: cpu->csr.mtval = v; return true;
        default: return false;                                   // unknown or read-only (misa, mhartid, mip)
    }
}

step_t cpu_step(cpu_t *cpu, memory_t *mem)
{
    uint32_t *x = cpu->regs.arr;
    uint32_t pc = cpu->pc;

    if (pc & 3) {
        return cpu_trap(cpu, CAUSE_FETCH_MISALIGNED, pc);
    }
    uint32_t off = pc - MEMORY_ROM_BASE;
    if (off >= mem->rom_size) {
        return cpu_trap(cpu, CAUSE_FETCH_FAULT, pc);
    }

    inst_decoded_t *d = &cpu->icache[off >> 2];
    if (d->op == INS_UNDECODED) {
        uint32_t word = 0;
        memory_read_word(mem, pc, &word);
        *d = inst_decode(word);
    }

    uint32_t next = pc + 4;

    switch (d->op) {
        case INS_ADD:  x[d->rd] = x[d->rs1] + x[d->rs2]; break;
        case INS_SUB:  x[d->rd] = x[d->rs1] - x[d->rs2]; break;
        case INS_SLL:  x[d->rd] = x[d->rs1] << (x[d->rs2] & 0x1F); break;
        case INS_SLT:  x[d->rd] = (int32_t)x[d->rs1] < (int32_t)x[d->rs2]; break;
        case INS_SLTU: x[d->rd] = x[d->rs1] < x[d->rs2]; break;
        case INS_XOR:  x[d->rd] = x[d->rs1] ^ x[d->rs2]; break;
        case INS_SRL:  x[d->rd] = x[d->rs1] >> (x[d->rs2] & 0x1F); break;
        case INS_SRA:  x[d->rd] = (int32_t)x[d->rs1] >> (x[d->rs2] & 0x1F); break;
        case INS_OR:   x[d->rd] = x[d->rs1] | x[d->rs2]; break;
        case INS_AND:  x[d->rd] = x[d->rs1] & x[d->rs2]; break;

        case INS_MUL:    x[d->rd] = x[d->rs1] * x[d->rs2]; break;
        case INS_MULH:   x[d->rd] = ((int64_t)(int32_t)x[d->rs1] * (int64_t)(int32_t)x[d->rs2]) >> 32; break;
        case INS_MULHSU: x[d->rd] = ((int64_t)(int32_t)x[d->rs1] * (uint64_t)x[d->rs2]) >> 32; break;
        case INS_MULHU:  x[d->rd] = ((uint64_t)x[d->rs1] * (uint64_t)x[d->rs2]) >> 32; break;

        // RISC-V defines div-by-zero and INT32_MIN / -1 results instead of trapping
        case INS_DIV: {
            int32_t dividend = (int32_t)x[d->rs1], divisor = (int32_t)x[d->rs2];
            if (divisor == 0) x[d->rd] = UINT32_MAX;
            else if (dividend == INT32_MIN && divisor == -1) x[d->rd] = (uint32_t)dividend;
            else x[d->rd] = (uint32_t)(dividend / divisor);
        } break;
        case INS_DIVU: {
            uint32_t divisor = x[d->rs2];
            x[d->rd] = divisor == 0 ? UINT32_MAX : x[d->rs1] / divisor;
        } break;
        case INS_REM: {
            int32_t dividend = (int32_t)x[d->rs1], divisor = (int32_t)x[d->rs2];
            if (divisor == 0) x[d->rd] = (uint32_t)dividend;
            else if (dividend == INT32_MIN && divisor == -1) x[d->rd] = 0;
            else x[d->rd] = (uint32_t)(dividend % divisor);
        } break;
        case INS_REMU: {
            uint32_t divisor = x[d->rs2];
            x[d->rd] = divisor == 0 ? x[d->rs1] : x[d->rs1] % divisor;
        } break;

        case INS_ADDI:  x[d->rd] = x[d->rs1] + d->imm; break;
        case INS_SLTI:  x[d->rd] = (int32_t)x[d->rs1] < d->imm; break;
        case INS_SLTIU: x[d->rd] = x[d->rs1] < (uint32_t)d->imm; break;
        case INS_XORI:  x[d->rd] = x[d->rs1] ^ d->imm; break;
        case INS_ORI:   x[d->rd] = x[d->rs1] | d->imm; break;
        case INS_ANDI:  x[d->rd] = x[d->rs1] & d->imm; break;
        case INS_SLLI:  x[d->rd] = x[d->rs1] << d->imm; break;
        case INS_SRLI:  x[d->rd] = x[d->rs1] >> d->imm; break;
        case INS_SRAI:  x[d->rd] = (int32_t)x[d->rs1] >> d->imm; break;

        case INS_LB: case INS_LBU: {
            uint32_t addr = x[d->rs1] + d->imm;
            uint8_t v;
            if (!memory_read_byte(mem, addr, &v)) return cpu_trap(cpu, CAUSE_LOAD_FAULT, addr);
            x[d->rd] = d->op == INS_LB ? (uint32_t)(int8_t)v : v;
        } break;
        case INS_LH: case INS_LHU: {
            uint32_t addr = x[d->rs1] + d->imm;
            uint16_t v;
            if (!memory_read_half(mem, addr, &v)) return cpu_trap(cpu, CAUSE_LOAD_FAULT, addr);
            x[d->rd] = d->op == INS_LH ? (uint32_t)(int16_t)v : v;
        } break;
        case INS_LW: {
            uint32_t addr = x[d->rs1] + d->imm;
            if (!memory_read_word(mem, addr, &x[d->rd])) return cpu_trap(cpu, CAUSE_LOAD_FAULT, addr);
        } break;

        case INS_SB: {
            uint32_t addr = x[d->rs1] + d->imm;
            if (!memory_write_byte(mem, addr, x[d->rs2] & 0xFF)) return cpu_trap(cpu, CAUSE_STORE_FAULT, addr);
        } break;
        case INS_SH: {
            uint32_t addr = x[d->rs1] + d->imm;
            if (!memory_write_half(mem, addr, x[d->rs2] & 0xFFFF)) return cpu_trap(cpu, CAUSE_STORE_FAULT, addr);
        } break;
        case INS_SW: {
            uint32_t addr = x[d->rs1] + d->imm;
            if (!memory_write_word(mem, addr, x[d->rs2])) return cpu_trap(cpu, CAUSE_STORE_FAULT, addr);
        } break;

        case INS_BEQ:  if (x[d->rs1] == x[d->rs2]) JUMP(pc + d->imm); break;
        case INS_BNE:  if (x[d->rs1] != x[d->rs2]) JUMP(pc + d->imm); break;
        case INS_BLT:  if ((int32_t)x[d->rs1] < (int32_t)x[d->rs2]) JUMP(pc + d->imm); break;
        case INS_BGE:  if ((int32_t)x[d->rs1] >= (int32_t)x[d->rs2]) JUMP(pc + d->imm); break;
        case INS_BLTU: if (x[d->rs1] < x[d->rs2]) JUMP(pc + d->imm); break;
        case INS_BGEU: if (x[d->rs1] >= x[d->rs2]) JUMP(pc + d->imm); break;

        case INS_LUI:   x[d->rd] = d->imm; break;
        case INS_AUIPC: x[d->rd] = pc + d->imm; break;
        case INS_JAL:   JUMP(pc + d->imm); x[d->rd] = pc + 4; break;
        case INS_JALR:  JUMP((x[d->rs1] + d->imm) & ~1u); x[d->rd] = pc + 4; break; // clear bit 0 per spec; rs1 read before rd write (rd may equal rs1)

        case INS_ECALL: {
            if (cpu->csr.mtvec) return cpu_trap(cpu, CAUSE_ECALL_M, 0); // guest owns ECALL once it has a handler
            step_t r = syscall_handle(&cpu->regs, mem);
            if (r != STEP_OK) {
                return r;
            }
        } break;
        case INS_EBREAK:
            return cpu_trap(cpu, CAUSE_BREAKPOINT, pc);

        case INS_CSRRW: case INS_CSRRS: case INS_CSRRC: case INS_CSRRWI: case INS_CSRRSI: case INS_CSRRCI: {
            bool imm = d->op >= INS_CSRRWI;                  // relies on enum order
            bool rw = d->op == INS_CSRRW || d->op == INS_CSRRWI;
            bool set = d->op == INS_CSRRS || d->op == INS_CSRRSI;
            uint32_t src = imm ? d->rs1 : x[d->rs1], old = 0;
            if (!(rw && d->rd == 0) && !cpu_csr_read(cpu, d->imm, &old)) goto illegal;  // CSRRW x0 must not read
            if (rw || d->rs1 != 0) {                                                // CSRRS/C with x0 must not write
                if (!cpu_csr_write(cpu, d->imm, rw ? src : set ? old | src : old & ~src)) goto illegal;
            }
            x[d->rd] = old;
        } break;

        case INS_MRET: {
            uint32_t ms = cpu->csr.mstatus;
            cpu->csr.mstatus = (ms & ~0x8u) | ((ms >> 4) & 0x8) | 0x80; // MIE = MPIE, MPIE = 1
            next = cpu->csr.mepc;
        } break;
        
        default: {
        illegal: ;
            uint32_t word = 0;
            memory_read_word(mem, pc, &word);
            return cpu_trap(cpu, CAUSE_ILLEGAL, word);
        }
    }

    cpu->pc = next;
    x[0] = 0;
    return STEP_OK;
}
