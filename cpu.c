#include "cpu.h"

#include <stdio.h>

#include "instruction.h"
#include "syscall.h"

bool cpu_step(cpu_t *cpu, memory_t *mem)
{
    register_file_t *regs = &cpu->regs;
    uint32_t *pc = &cpu->pc;

    uint32_t inst_word = 0;
    bool ok = memory_read_word(mem, *pc, &inst_word);
    if (!ok) {
        fprintf(stderr, "Failed to read memory at PC=0x%08x\n", *pc);
        return false;
    }
    // printf("Instruction @PC=%zu: 0x%08x\n", *pc, inst_word);

    if (inst_word == 0) {
        printf("  Encountered zero inst, stopping execution\n");
        return false;
    }
    
    inst_t inst;
    inst.val = inst_word;
    inst_format_t fmt = inst_get_format(inst);
    // switch (fmt) {
    //     case OP_R: printf("  R-type instruction\n"); break;
    //     case OP_I: printf("  I-type instruction\n"); break;
    //     case OP_S: printf("  S-type instruction\n"); break;
    //     case OP_B: printf("  B-type instruction\n"); break;
    //     case OP_U: printf("  U-type instruction\n"); break;
    //     case OP_J: printf("  J-type instruction\n"); break;
    //     default: printf("  Unknown instruction format\n"); return false;
    // }

    switch (fmt) {
        case OP_R: {
            uint32_t rd = inst_get_rd(inst);
            uint32_t rs1 = inst_get_rs1(inst);
            uint32_t rs2 = inst_get_rs2(inst);
            uint32_t funct3 = inst_get_funct3(inst);
            uint32_t funct7 = inst_get_funct7(inst);
            // printf("    rd: %u, rs1: %u, rs2: %u, funct3: %u, funct7: %u\n", rd, rs1, rs2, funct3, funct7);

            switch (funct3) {
                case 0x0: {
                    if (funct7 == 0x00) {
                        // printf("      ADD instruction\n");
                        regs->arr[rd] = regs->arr[rs1] + regs->arr[rs2];
                    } else if (funct7 == 0x20) {
                        // printf("      SUB instruction\n");
                        regs->arr[rd] = regs->arr[rs1] - regs->arr[rs2];
                    } else if (funct7 == 0x01) {
                        // printf("      MUL instruction\n");
                        regs->arr[rd] = (uint64_t)(regs->arr[rs1] * regs->arr[rs2]) & 0xFFFFFFFF;
                    } else {
                        fprintf(stderr, "      Unknown R-type instruction\n");
                        return false;
                    }
                } break;
                case 0x1: {
                    if (funct7 == 0x00) {
                        // printf("      SLL instruction\n");
                        regs->arr[rd] = regs->arr[rs1] << (regs->arr[rs2] & 0x1F);
                    } else if (funct7 == 0x01) {
                        // printf("      MULH instruction\n");
                        regs->arr[rd] = ((int64_t)(int32_t)regs->arr[rs1] * (int64_t)(int32_t)regs->arr[rs2]) >> 32;
                    } else {
                        fprintf(stderr, "      Unknown R-type instruction\n");
                        return false;
                    }
                } break;
                case 0x2: {
                    if (funct7 == 0x00) {
                        // printf("      SLT instruction\n");
                        regs->arr[rd] = (int32_t)regs->arr[rs1] < (int32_t)regs->arr[rs2];
                    } else if (funct7 == 0x01) {
                        // printf("      MULHSU instruction\n");
                        regs->arr[rd] = ((int64_t)(int32_t)regs->arr[rs1] * (uint64_t)regs->arr[rs2]) >> 32;
                    } else {
                        fprintf(stderr, "      Unknown R-type instruction\n");
                        return false;
                    }
                } break;
                case 0x3: {
                    if (funct7 == 0x00) {
                        // printf("      SLTU instruction\n");
                        regs->arr[rd] = regs->arr[rs1] < regs->arr[rs2];
                    } else if (funct7 == 0x01) {
                        // printf("      MULHU instruction\n");
                        regs->arr[rd] = ((uint64_t)regs->arr[rs1] * (uint64_t)regs->arr[rs2]) >> 32;
                    } else {
                        fprintf(stderr, "      Unknown R-type instruction\n");
                        return false;
                    }
                } break;
                case 0x4: {
                    if (funct7 == 0x00) {
                        // printf("      XOR instruction\n");
                        regs->arr[rd] = regs->arr[rs1] ^ regs->arr[rs2];
                    } else if (funct7 == 0x01) {
                        // printf("      DIV instruction\n");
                        // RISC-V defines div-by-zero and overflow results; C would trap (SIGFPE)
                        int32_t a = regs->arr[rs1], b = regs->arr[rs2];
                        regs->arr[rd] = b == 0 ? 0xFFFFFFFF : (a == INT32_MIN && b == -1) ? (uint32_t)INT32_MIN : (uint32_t)(a / b);
                    } else {
                        fprintf(stderr, "      Unknown R-type instruction\n");
                        return false;
                    }
                } break;
                case 0x5: {
                    if (funct7 == 0x00) {
                        // printf("      SRL instruction\n");
                        regs->arr[rd] = regs->arr[rs1] >> (regs->arr[rs2] & 0x1F);
                    } else if (funct7 == 0x20) {
                        // printf("      SRA instruction\n");
                        regs->arr[rd] = (int32_t)regs->arr[rs1] >> (regs->arr[rs2] & 0x1F);
                    } else if (funct7 == 0x01) {
                        // printf("      DIVU instruction\n");
                        regs->arr[rd] = regs->arr[rs2] == 0 ? 0xFFFFFFFF : regs->arr[rs1] / regs->arr[rs2];
                    } else {
                        fprintf(stderr, "      Unknown R-type instruction\n");
                        return false;
                    }
                } break;
                case 0x6: {
                    if (funct7 == 0x00) {
                        // printf("      OR instruction\n");
                        regs->arr[rd] = regs->arr[rs1] | regs->arr[rs2];
                    } else if (funct7 == 0x01) {
                        // printf("      REM instruction\n");
                        int32_t a = regs->arr[rs1], b = regs->arr[rs2];
                        regs->arr[rd] = b == 0 ? (uint32_t)a : (a == INT32_MIN && b == -1) ? 0 : (uint32_t)(a % b);
                    } else {
                        fprintf(stderr, "      Unknown R-type instruction\n");
                        return false;
                    }
                } break;
                case 0x7: {
                    if (funct7 == 0x00) {
                        // printf("      AND instruction\n");
                        regs->arr[rd] = regs->arr[rs1] & regs->arr[rs2];
                    } else if (funct7 == 0x01) {
                        // printf("      REMU instruction\n");
                        regs->arr[rd] = regs->arr[rs2] == 0 ? regs->arr[rs1] : regs->arr[rs1] % regs->arr[rs2];
                    } else {
                        fprintf(stderr, "      Unknown R-type instruction\n");
                        return false;
                    }
                } break;
                default: {
                    fprintf(stderr, "      Unknown R-type instruction\n");
                    return false;
                } break;
            }
        } break;
        case OP_I: {
            uint32_t opcode = inst_get_opcode(inst);
            uint32_t rd = inst_get_rd(inst);
            uint32_t rs1 = inst_get_rs1(inst);
            uint32_t funct3 = inst_get_funct3(inst);
            int32_t imm = inst_get_imm(inst);
            // printf("    opcode: %u, rd: %u, rs1: %u, funct3: %u, imm: %d\n", opcode, rd, rs1, funct3, imm);

            if (opcode == 0x13) {
                switch (funct3) {
                    case 0x0: {
                        // printf("      ADDI instruction\n");
                        regs->arr[rd] = regs->arr[rs1] + imm;
                    } break;
                    case 0x4: {
                        // printf("      XORI instruction\n");
                        regs->arr[rd] = regs->arr[rs1] ^ imm;
                    } break;
                    case 0x6: {
                        // printf("      ORI instruction\n");
                        regs->arr[rd] = regs->arr[rs1] | imm;
                    } break;
                    case 0x7: {
                        // printf("      ANDI instruction\n");
                        regs->arr[rd] = regs->arr[rs1] & imm;
                    } break;
                    case 0x1: {
                        // printf("      SLLI instruction\n");
                        regs->arr[rd] = regs->arr[rs1] << (imm & 0x1F);
                    } break;
                    case 0x5: {
                        if ((imm & 0xFE0) == 0) {
                            // printf("      SRLI instruction\n");
                            regs->arr[rd] = regs->arr[rs1] >> (imm & 0x1F);
                        } else if ((imm & 0xFE0) == 0x400) {
                            // printf("      SRAI instruction\n");
                            regs->arr[rd] = ((int32_t)regs->arr[rs1]) >> (imm & 0x1F);
                        } else {
                            fprintf(stderr, "      Unknown I-type instruction\n");
                            return false;
                        }
                    } break;
                    case 0x2: {
                        // printf("      SLTI instruction\n");
                        regs->arr[rd] = ((int32_t)regs->arr[rs1] < imm) ? 1 : 0;
                    } break;
                    case 0x3: {
                        // printf("      SLTIU instruction\n");
                        regs->arr[rd] = ((uint32_t)regs->arr[rs1] < (uint32_t)imm) ? 1 : 0;
                    } break;
                }
            } else if (opcode == 0x03) {
                switch (funct3) {
                    case 0: {
                        // printf("      LB instruction\n");
                        uint8_t byte_val;
                        if (!memory_read_byte(mem, regs->arr[rs1] + imm, &byte_val)) {
                            fprintf(stderr, "Memory read failed\n");
                            return false;
                        }
                        regs->arr[rd] = (int8_t)byte_val;
                    } break;
                    case 0x1: {
                        // printf("      LH instruction\n");
                        uint16_t half_val;
                        if (!memory_read_half(mem, regs->arr[rs1] + imm, &half_val)) {
                            fprintf(stderr, "Memory read failed\n");
                            return false;
                        }
                        regs->arr[rd] = (int16_t)half_val;
                    } break;
                    case 0x2: {
                        // printf("      LW instruction\n");
                        uint32_t word_val;
                        if (!memory_read_word(mem, regs->arr[rs1] + imm, &word_val)) {
                            fprintf(stderr, "Memory read failed\n");
                            return false;
                        }
                        regs->arr[rd] = word_val;
                    } break;
                    case 0x4: {
                        // printf("      LBU instruction\n");
                        uint8_t byte_val_u;
                        if (!memory_read_byte(mem, regs->arr[rs1] + imm, &byte_val_u)) {
                            fprintf(stderr, "Memory read failed\n");
                            return false;
                        }
                        regs->arr[rd] = byte_val_u;
                    } break;
                    case 0x5: {
                        // printf("      LHU instruction\n");
                        uint16_t half_val_u;
                        if (!memory_read_half(mem, regs->arr[rs1] + imm, &half_val_u)) {
                            fprintf(stderr, "Memory read failed\n");
                            return false;
                        }
                        regs->arr[rd] = half_val_u;
                    } break;
                    default: {
                        fprintf(stderr, "Unknown load instruction\n");
                        return false;
                    } break;
                }
            } else if (opcode == 0x67) {
                // printf("    JALR instruction\n");
                uint32_t current_rs1 = regs->arr[rs1];
                regs->arr[rd] = *pc + 4;
                *pc = ((current_rs1 + imm) & ~1) - 4;
            } else if (opcode == 0x73) {
                if (imm == 0) {
                    // printf("      ECALL instruction\n");
                    if (!syscall_handle(regs, mem)) {
                        return false;
                    }
                } else if (imm == 1) {
                    // printf("      EBREAK instruction\n");
                    fprintf(stderr, "EBREAK encountered\n");
                    return false;
                } else {
                    fprintf(stderr, "Unknown SYSTEM instruction\n");
                    return false;
                }
            } else {
                fprintf(stderr, "Unknown I-type instruction\n");
                return false;
            }
        } break;
        case OP_S: {
            uint32_t rs1 = inst_get_rs1(inst);
            uint32_t rs2 = inst_get_rs2(inst);
            uint32_t funct3 = inst_get_funct3(inst);
            int32_t imm = inst_get_imm(inst);
            // printf("    rs1: %u, rs2: %u, funct3: %u, imm: %d\n", rs1, rs2, funct3, imm);

            switch (funct3) {
                case 0x0: {
                    // printf("      SB instruction\n");
                    if (!memory_write_byte(mem, regs->arr[rs1] + imm, regs->arr[rs2] & 0xFF)) {
                        fprintf(stderr, "Memory write failed\n");
                        return false;
                    }
                } break;
                case 0x1: {
                    // printf("      SH instruction\n");
                    if (!memory_write_half(mem, regs->arr[rs1] + imm, regs->arr[rs2] & 0xFFFF)) {
                        fprintf(stderr, "Memory write failed\n");
                        return false;
                    }
                } break;
                case 0x2: {
                    // printf("      SW instruction\n");
                    if (!memory_write_word(mem, regs->arr[rs1] + imm, regs->arr[rs2])) {
                        fprintf(stderr, "Memory write failed\n");
                        return false;
                    }
                } break;
                default: {
                    fprintf(stderr, "Unknown store instruction\n");
                    return false;
                } break;
            }
        } break;
        case OP_B: {
            uint32_t rs1 = inst_get_rs1(inst);
            uint32_t rs2 = inst_get_rs2(inst);
            uint32_t funct3 = inst_get_funct3(inst);
            int32_t imm = inst_get_imm(inst);
            // printf("    rs1: %u, rs2: %u, funct3: %u, imm: %d\n", rs1, rs2, funct3, imm);

            switch (funct3) {
                case 0x0: {
                    // printf("      BEQ instruction\n");
                    if (regs->arr[rs1] == regs->arr[rs2]) {
                        *pc += imm - 4;
                    }
                } break;
                case 0x1: {
                    // printf("      BNE instruction\n");
                    if (regs->arr[rs1] != regs->arr[rs2]) {
                        *pc += imm - 4;
                    }
                } break;
                case 0x4: {
                    // printf("      BLT instruction\n");
                    if ((int32_t)regs->arr[rs1] < (int32_t)regs->arr[rs2]) {
                        *pc += imm - 4;
                    }
                } break;
                case 0x5: {
                    // printf("      BGE instruction\n");
                    if ((int32_t)regs->arr[rs1] >= (int32_t)regs->arr[rs2]) {
                        *pc += imm - 4;
                    }
                } break;
                case 0x6: {
                    // printf("      BLTU instruction\n");
                    if ((uint32_t)regs->arr[rs1] < (uint32_t)regs->arr[rs2]) {
                        *pc += imm - 4;
                    }
                } break;
                case 0x7: {
                    // printf("      BGEU instruction\n");
                    if ((uint32_t)regs->arr[rs1] >= (uint32_t)regs->arr[rs2]) {
                        *pc += imm - 4;
                    }
                } break;
                default: {
                    fprintf(stderr, "      Unknown B-type instruction\n");
                    return false;
                }
            }

        } break;
        case OP_U: {
            uint32_t opcode = inst_get_opcode(inst);
            uint32_t rd = inst_get_rd(inst);
            int32_t imm = inst_get_imm(inst);
            // printf("    rd: %u, imm: %d\n", rd, imm);

            switch (opcode) {
                case 0x37: {
                    // printf("      LUI instruction\n");
                    regs->arr[rd] = imm;
                } break;
                case 0x17: {
                    // printf("      AUIPC instruction\n");
                    regs->arr[rd] = *pc + imm;
                } break;
                default: {
                    fprintf(stderr, "      Unknown U-type instruction\n");
                    return false;
                }
            }
        } break;
        case OP_J: {
            uint32_t rd = inst_get_rd(inst);
            int32_t imm = inst_get_imm(inst);
            // printf("    rd: %u, imm: %d\n", rd, imm);

            // printf("      JAL instruction\n");
            if (imm == 0) {
                // printf("  Jump-to-self, halting\n");
                return false;
            }
            regs->arr[rd] = *pc + 4;
            *pc += imm - 4;
        } break;
    }

    *pc += 4;
    regs->arr[0] = 0;  // x0 is always 0

    return true;
}