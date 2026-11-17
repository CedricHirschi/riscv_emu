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
            if (!memory_read_byte(mem, addr, &v)) return cpu_trap(cpu, CAUSE_LOAD_FAULT, addr);
            x[d->rd] = d->op == INS_LB ? (uint32_t)(int8_t)v : v;
        } break;
        case INS_LH: case INS_LHU: {
            uint16_t v;
            if (!memory_read_half(mem, addr, &v)) return cpu_trap(cpu, CAUSE_LOAD_FAULT, addr);
            x[d->rd] = d->op == INS_LH ? (uint32_t)(int16_t)v : v;
        } break;
        case INS_LW:
            if (!memory_read_word(mem, addr, &x[d->rd])) return cpu_trap(cpu, CAUSE_LOAD_FAULT, addr);
            break;

        case INS_SB:
            if (!memory_write_byte(mem, addr, b & 0xFF)) return cpu_trap(cpu, CAUSE_STORE_FAULT, addr);
            break;
        case INS_SH:
            if (!memory_write_half(mem, addr, b & 0xFFFF)) return cpu_trap(cpu, CAUSE_STORE_FAULT, addr);
            break;
        case INS_SW:
            if (!memory_write_word(mem, addr, b)) return cpu_trap(cpu, CAUSE_STORE_FAULT, addr);
            break;

        case INS_BEQ:  if (a == b) JUMP(pc + d->imm); break;
        case INS_BNE:  if (a != b) JUMP(pc + d->imm); break;
        case INS_BLT:  if ((int32_t)a < (int32_t)b) JUMP(pc + d->imm); break;
        case INS_BGE:  if ((int32_t)a >= (int32_t)b) JUMP(pc + d->imm); break;
        case INS_BLTU: if (a < b) JUMP(pc + d->imm); break;
        case INS_BGEU: if (a >= b) JUMP(pc + d->imm); break;

        case INS_LUI:   x[d->rd] = d->imm; break;
        case INS_AUIPC: x[d->rd] = pc + d->imm; break;
        case INS_JAL:   JUMP(pc + d->imm); x[d->rd] = pc + 4; break;
        case INS_JALR:  JUMP((a + d->imm) & ~1u); x[d->rd] = pc + 4; break; // clear bit 0 per spec; a read before rd write (rd may equal rs1)

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
            uint32_t src = imm ? d->rs1 : a, old = 0;
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
