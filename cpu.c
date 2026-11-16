#include "cpu.h"

#include <stdio.h>

#include "instruction.h"
#include "syscall.h"

step_t cpu_step(cpu_t *cpu, memory_t *mem)
{
    register_file_t *regs = &cpu->regs;
    uint32_t *pc = &cpu->pc;

    uint32_t inst_word = 0;
    bool ok = memory_read_word(mem, *pc, &inst_word);
    if (!ok) {
        fprintf(stderr, "Failed to read memory at PC=0x%08x\n", *pc);
        return STEP_ERROR;
    }

    inst_t inst;
    inst.val = inst_word;
    inst_format_t fmt = inst_get_format(inst);

    switch (fmt) {
        case OP_R: {
            uint32_t rd = inst_get_rd(inst);
            uint32_t rs1 = inst_get_rs1(inst);
            uint32_t rs2 = inst_get_rs2(inst);
            uint32_t funct3 = inst_get_funct3(inst);
            uint32_t funct7 = inst_get_funct7(inst);

            switch (funct3) {
                case 0x0: {
                    if (funct7 == 0x00) {
                        regs->arr[rd] = regs->arr[rs1] + regs->arr[rs2];
                    } else if (funct7 == 0x20) {
                        regs->arr[rd] = regs->arr[rs1] - regs->arr[rs2];
                    } else if (funct7 == 0x01) {
                        regs->arr[rd] = (uint64_t)(regs->arr[rs1] * regs->arr[rs2]) & 0xFFFFFFFF;
                    } else {
                        fprintf(stderr, "      Unknown R-type instruction\n");
                        return STEP_ERROR;
                    }
                } break;
                case 0x1: {
                    if (funct7 == 0x00) {
                        regs->arr[rd] = regs->arr[rs1] << (regs->arr[rs2] & 0x1F);
                    } else if (funct7 == 0x01) {
                        regs->arr[rd] = ((int64_t)(int32_t)regs->arr[rs1] * (int64_t)(int32_t)regs->arr[rs2]) >> 32;
                    } else {
                        fprintf(stderr, "      Unknown R-type instruction\n");
                        return STEP_ERROR;
                    }
                } break;
                case 0x2: {
                    if (funct7 == 0x00) {
                        regs->arr[rd] = (int32_t)regs->arr[rs1] < (int32_t)regs->arr[rs2];
                    } else if (funct7 == 0x01) {
                        regs->arr[rd] = ((int64_t)(int32_t)regs->arr[rs1] * (uint64_t)regs->arr[rs2]) >> 32;
                    } else {
                        fprintf(stderr, "      Unknown R-type instruction\n");
                        return STEP_ERROR;
                    }
                } break;
                case 0x3: {
                    if (funct7 == 0x00) {
                        regs->arr[rd] = regs->arr[rs1] < regs->arr[rs2];
                    } else if (funct7 == 0x01) {
                        regs->arr[rd] = ((uint64_t)regs->arr[rs1] * (uint64_t)regs->arr[rs2]) >> 32;
                    } else {
                        fprintf(stderr, "      Unknown R-type instruction\n");
                        return STEP_ERROR;
                    }
                } break;
                case 0x4: {
                    if (funct7 == 0x00) {
                        regs->arr[rd] = regs->arr[rs1] ^ regs->arr[rs2];
                    } else if (funct7 == 0x01) {
                        // RISC-V defines div-by-zero and overflow results; C would trap (SIGFPE)
                        int32_t a = regs->arr[rs1], b = regs->arr[rs2];
                        regs->arr[rd] = b == 0 ? 0xFFFFFFFF : (a == INT32_MIN && b == -1) ? (uint32_t)INT32_MIN : (uint32_t)(a / b);
                    } else {
                        fprintf(stderr, "      Unknown R-type instruction\n");
                        return STEP_ERROR;
                    }
                } break;
                case 0x5: {
                    if (funct7 == 0x00) {
                        regs->arr[rd] = regs->arr[rs1] >> (regs->arr[rs2] & 0x1F);
                    } else if (funct7 == 0x20) {
                        regs->arr[rd] = (int32_t)regs->arr[rs1] >> (regs->arr[rs2] & 0x1F);
                    } else if (funct7 == 0x01) {
                        regs->arr[rd] = regs->arr[rs2] == 0 ? 0xFFFFFFFF : regs->arr[rs1] / regs->arr[rs2];
                    } else {
                        fprintf(stderr, "      Unknown R-type instruction\n");
                        return STEP_ERROR;
                    }
                } break;
                case 0x6: {
                    if (funct7 == 0x00) {
                        regs->arr[rd] = regs->arr[rs1] | regs->arr[rs2];
                    } else if (funct7 == 0x01) {
                        int32_t a = regs->arr[rs1], b = regs->arr[rs2];
                        regs->arr[rd] = b == 0 ? (uint32_t)a : (a == INT32_MIN && b == -1) ? 0 : (uint32_t)(a % b);
                    } else {
                        fprintf(stderr, "      Unknown R-type instruction\n");
                        return STEP_ERROR;
                    }
                } break;
                case 0x7: {
                    if (funct7 == 0x00) {
                        regs->arr[rd] = regs->arr[rs1] & regs->arr[rs2];
                    } else if (funct7 == 0x01) {
                        regs->arr[rd] = regs->arr[rs2] == 0 ? regs->arr[rs1] : regs->arr[rs1] % regs->arr[rs2];
                    } else {
                        fprintf(stderr, "      Unknown R-type instruction\n");
                        return STEP_ERROR;
                    }
                } break;
                default: {
                    fprintf(stderr, "      Unknown R-type instruction\n");
                    return STEP_ERROR;
                } break;
            }
        } break;
        case OP_I: {
            uint32_t opcode = inst_get_opcode(inst);
            uint32_t rd = inst_get_rd(inst);
            uint32_t rs1 = inst_get_rs1(inst);
            uint32_t funct3 = inst_get_funct3(inst);
            int32_t imm = inst_get_imm(inst);

            if (opcode == 0x13) {
                switch (funct3) {
                    case 0x0: {
                        regs->arr[rd] = regs->arr[rs1] + imm;
                    } break;
                    case 0x4: {
                        regs->arr[rd] = regs->arr[rs1] ^ imm;
                    } break;
                    case 0x6: {
                        regs->arr[rd] = regs->arr[rs1] | imm;
                    } break;
                    case 0x7: {
                        regs->arr[rd] = regs->arr[rs1] & imm;
                    } break;
                    case 0x1: {
                        regs->arr[rd] = regs->arr[rs1] << (imm & 0x1F);
                    } break;
                    case 0x5: {
                        if ((imm & 0xFE0) == 0) {
                            regs->arr[rd] = regs->arr[rs1] >> (imm & 0x1F);
                        } else if ((imm & 0xFE0) == 0x400) {
                            regs->arr[rd] = ((int32_t)regs->arr[rs1]) >> (imm & 0x1F);
                        } else {
                            fprintf(stderr, "      Unknown I-type instruction\n");
                            return STEP_ERROR;
                        }
                    } break;
                    case 0x2: {
                        regs->arr[rd] = ((int32_t)regs->arr[rs1] < imm) ? 1 : 0;
                    } break;
                    case 0x3: {
                        regs->arr[rd] = ((uint32_t)regs->arr[rs1] < (uint32_t)imm) ? 1 : 0;
                    } break;
                }
            } else if (opcode == 0x03) {
                switch (funct3) {
                    case 0: {
                        uint8_t byte_val;
                        if (!memory_read_byte(mem, regs->arr[rs1] + imm, &byte_val)) {
                            fprintf(stderr, "Memory read failed\n");
                            return STEP_ERROR;
                        }
                        regs->arr[rd] = (int8_t)byte_val;
                    } break;
                    case 0x1: {
                        uint16_t half_val;
                        if (!memory_read_half(mem, regs->arr[rs1] + imm, &half_val)) {
                            fprintf(stderr, "Memory read failed\n");
                            return STEP_ERROR;
                        }
                        regs->arr[rd] = (int16_t)half_val;
                    } break;
                    case 0x2: {
                        uint32_t word_val;
                        if (!memory_read_word(mem, regs->arr[rs1] + imm, &word_val)) {
                            fprintf(stderr, "Memory read failed\n");
                            return STEP_ERROR;
                        }
                        regs->arr[rd] = word_val;
                    } break;
                    case 0x4: {
                        uint8_t byte_val_u;
                        if (!memory_read_byte(mem, regs->arr[rs1] + imm, &byte_val_u)) {
                            fprintf(stderr, "Memory read failed\n");
                            return STEP_ERROR;
                        }
                        regs->arr[rd] = byte_val_u;
                    } break;
                    case 0x5: {
                        uint16_t half_val_u;
                        if (!memory_read_half(mem, regs->arr[rs1] + imm, &half_val_u)) {
                            fprintf(stderr, "Memory read failed\n");
                            return STEP_ERROR;
                        }
                        regs->arr[rd] = half_val_u;
                    } break;
                    default: {
                        fprintf(stderr, "Unknown load instruction\n");
                        return STEP_ERROR;
                    } break;
                }
            } else if (opcode == 0x67) {
                uint32_t current_rs1 = regs->arr[rs1];
                regs->arr[rd] = *pc + 4;
                *pc = ((current_rs1 + imm) & ~1) - 4;
            } else if (opcode == 0x73) {
                if (imm == 0) {
                    step_t r = syscall_handle(regs, mem);
                    if (r != STEP_OK) {
                        return r;
                    }
                } else if (imm == 1) {
                    fprintf(stderr, "EBREAK encountered\n");
                    return STEP_ERROR;
                } else {
                    fprintf(stderr, "Unknown SYSTEM instruction\n");
                    return STEP_ERROR;
                }
            } else {
                fprintf(stderr, "Unknown I-type instruction\n");
                return STEP_ERROR;
            }
        } break;
        case OP_S: {
            uint32_t rs1 = inst_get_rs1(inst);
            uint32_t rs2 = inst_get_rs2(inst);
            uint32_t funct3 = inst_get_funct3(inst);
            int32_t imm = inst_get_imm(inst);

            switch (funct3) {
                case 0x0: {
                    if (!memory_write_byte(mem, regs->arr[rs1] + imm, regs->arr[rs2] & 0xFF)) {
                        fprintf(stderr, "Memory write failed\n");
                        return STEP_ERROR;
                    }
                } break;
                case 0x1: {
                    if (!memory_write_half(mem, regs->arr[rs1] + imm, regs->arr[rs2] & 0xFFFF)) {
                        fprintf(stderr, "Memory write failed\n");
                        return STEP_ERROR;
                    }
                } break;
                case 0x2: {
                    if (!memory_write_word(mem, regs->arr[rs1] + imm, regs->arr[rs2])) {
                        fprintf(stderr, "Memory write failed\n");
                        return STEP_ERROR;
                    }
                } break;
                default: {
                    fprintf(stderr, "Unknown store instruction\n");
                    return STEP_ERROR;
                } break;
            }
        } break;
        case OP_B: {
            uint32_t rs1 = inst_get_rs1(inst);
            uint32_t rs2 = inst_get_rs2(inst);
            uint32_t funct3 = inst_get_funct3(inst);
            int32_t imm = inst_get_imm(inst);

            switch (funct3) {
                case 0x0: {
                    if (regs->arr[rs1] == regs->arr[rs2]) {
                        *pc += imm - 4;
                    }
                } break;
                case 0x1: {
                    if (regs->arr[rs1] != regs->arr[rs2]) {
                        *pc += imm - 4;
                    }
                } break;
                case 0x4: {
                    if ((int32_t)regs->arr[rs1] < (int32_t)regs->arr[rs2]) {
                        *pc += imm - 4;
                    }
                } break;
                case 0x5: {
                    if ((int32_t)regs->arr[rs1] >= (int32_t)regs->arr[rs2]) {
                        *pc += imm - 4;
                    }
                } break;
                case 0x6: {
                    if ((uint32_t)regs->arr[rs1] < (uint32_t)regs->arr[rs2]) {
                        *pc += imm - 4;
                    }
                } break;
                case 0x7: {
                    if ((uint32_t)regs->arr[rs1] >= (uint32_t)regs->arr[rs2]) {
                        *pc += imm - 4;
                    }
                } break;
                default: {
                    fprintf(stderr, "      Unknown B-type instruction\n");
                    return STEP_ERROR;
                }
            }

        } break;
        case OP_U: {
            uint32_t opcode = inst_get_opcode(inst);
            uint32_t rd = inst_get_rd(inst);
            int32_t imm = inst_get_imm(inst);

            switch (opcode) {
                case 0x37: {
                    regs->arr[rd] = imm;
                } break;
                case 0x17: {
                    regs->arr[rd] = *pc + imm;
                } break;
                default: {
                    fprintf(stderr, "      Unknown U-type instruction\n");
                    return STEP_ERROR;
                }
            }
        } break;
        case OP_J: {
            uint32_t rd = inst_get_rd(inst);
            int32_t imm = inst_get_imm(inst);

            regs->arr[rd] = *pc + 4;
            *pc += imm - 4;
        } break;
        default: {
            fprintf(stderr, "Illegal instruction 0x%08x at PC=0x%08x\n", inst_word, *pc);
            return STEP_ERROR;
        }
    }

    *pc += 4;
    regs->arr[0] = 0;  // x0 is always 0

    return STEP_OK;
}