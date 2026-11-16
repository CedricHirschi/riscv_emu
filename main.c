#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "elf_parse.h"
#include "instruction.h"
#include "register.h"


static bool mem_ok(size_t mem_size, uint32_t addr, size_t n);
static uint32_t load_word(const uint8_t *mem, uint32_t addr);
static void store_word(uint8_t *mem, uint32_t addr, uint32_t val);

#define MAX_ITER 10000000

int main(int argc, char *argv[]) {
    // Open and read ELF file specified as the first command-line argument
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <elf_file>\n", argv[0]);
        return 1;
    }
    FILE *file = fopen(argv[1], "rb");
    if (!file) {
        fprintf(stderr, "Failed to open %s\n", argv[1]);
        return 1;
    }
    
    uint8_t mem[8192];
    elf_fields_t fields;
    size_t offset;
    size_t read;
    if (!elf_read(file, &fields, mem, sizeof(mem), &offset, &read)) {
        fprintf(stderr, "Failed to parse ELF file\n");
        fclose(file);
        return 1;
    }

    fclose(file);

    register_file_t regs;
    memset(&regs, 0, sizeof(regs));

    uint32_t pc = fields.ehdr.entry; // Program counter in bytes

    printf("\n", pc);

    size_t i = 0;
    while (i++ < MAX_ITER) {
        if (pc >= read) {
            fprintf(stderr, "Program counter out of bounds\n");
            break;
        }

        uint32_t word = *(uint32_t *)(mem + pc);
        // printf("Word @PC=%zu: 0x%08x\n", pc, word);

        if (word == 0) {
            printf("  Encountered zero word, stopping execution\n");
            break;
        }
        
        inst_t inst;
        inst.val = word;
        inst_format_t fmt = inst_get_format(inst);
        // switch (fmt) {
        //     case OP_R: printf("  R-type instruction\n"); break;
        //     case OP_I: printf("  I-type instruction\n"); break;
        //     case OP_S: printf("  S-type instruction\n"); break;
        //     case OP_B: printf("  B-type instruction\n"); break;
        //     case OP_U: printf("  U-type instruction\n"); break;
        //     case OP_J: printf("  J-type instruction\n"); break;
        //     default: printf("  Unknown instruction format\n"); return 1;
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
                            regs.arr[rd] = regs.arr[rs1] + regs.arr[rs2];
                        } else if (funct7 == 0x20) {
                            // printf("      SUB instruction\n");
                            regs.arr[rd] = regs.arr[rs1] - regs.arr[rs2];
                        } else if (funct7 == 0x01) {
                            // printf("      MUL instruction\n");
                            regs.arr[rd] = (uint64_t)(regs.arr[rs1] * regs.arr[rs2]) & 0xFFFFFFFF;
                        } else {
                            fprintf(stderr, "      Unknown R-type instruction\n");
                            return 1;
                        }
                    } break;
                    case 0x1: {
                        if (funct7 == 0x00) {
                            // printf("      SLL instruction\n");
                            regs.arr[rd] = regs.arr[rs1] << (regs.arr[rs2] & 0x1F);
                        } else if (funct7 == 0x01) {
                            // printf("      MULH instruction\n");
                            regs.arr[rd] = ((int64_t)(int32_t)regs.arr[rs1] * (int64_t)(int32_t)regs.arr[rs2]) >> 32;
                        } else {
                            fprintf(stderr, "      Unknown R-type instruction\n");
                            return 1;
                        }
                    } break;
                    case 0x2: {
                        if (funct7 == 0x00) {
                            // printf("      SLT instruction\n");
                            regs.arr[rd] = (int32_t)regs.arr[rs1] < (int32_t)regs.arr[rs2];
                        } else if (funct7 == 0x01) {
                            // printf("      MULHSU instruction\n");
                            regs.arr[rd] = ((int64_t)(int32_t)regs.arr[rs1] * (uint64_t)regs.arr[rs2]) >> 32;
                        } else {
                            fprintf(stderr, "      Unknown R-type instruction\n");
                            return 1;
                        }
                    } break;
                    case 0x3: {
                        if (funct7 == 0x00) {
                            // printf("      SLTU instruction\n");
                            regs.arr[rd] = regs.arr[rs1] < regs.arr[rs2];
                        } else if (funct7 == 0x01) {
                            // printf("      MULHU instruction\n");
                            regs.arr[rd] = ((uint64_t)regs.arr[rs1] * (uint64_t)regs.arr[rs2]) >> 32;
                        } else {
                            fprintf(stderr, "      Unknown R-type instruction\n");
                            return 1;
                        }
                    } break;
                    case 0x4: {
                        if (funct7 == 0x00) {
                            // printf("      XOR instruction\n");
                            regs.arr[rd] = regs.arr[rs1] ^ regs.arr[rs2];
                        } else if (funct7 == 0x01) {
                            // printf("      DIV instruction\n");
                            // RISC-V defines div-by-zero and overflow results; C would trap (SIGFPE)
                            int32_t a = regs.arr[rs1], b = regs.arr[rs2];
                            regs.arr[rd] = b == 0 ? 0xFFFFFFFF : (a == INT32_MIN && b == -1) ? (uint32_t)INT32_MIN : (uint32_t)(a / b);
                        } else {
                            fprintf(stderr, "      Unknown R-type instruction\n");
                            return 1;
                        }
                    } break;
                    case 0x5: {
                        if (funct7 == 0x00) {
                            // printf("      SRL instruction\n");
                            regs.arr[rd] = regs.arr[rs1] >> (regs.arr[rs2] & 0x1F);
                        } else if (funct7 == 0x20) {
                            // printf("      SRA instruction\n");
                            regs.arr[rd] = (int32_t)regs.arr[rs1] >> (regs.arr[rs2] & 0x1F);
                        } else if (funct7 == 0x01) {
                            // printf("      DIVU instruction\n");
                            regs.arr[rd] = regs.arr[rs2] == 0 ? 0xFFFFFFFF : regs.arr[rs1] / regs.arr[rs2];
                        } else {
                            fprintf(stderr, "      Unknown R-type instruction\n");
                            return 1;
                        }
                    } break;
                    case 0x6: {
                        if (funct7 == 0x00) {
                            // printf("      OR instruction\n");
                            regs.arr[rd] = regs.arr[rs1] | regs.arr[rs2];
                        } else if (funct7 == 0x01) {
                            // printf("      REM instruction\n");
                            int32_t a = regs.arr[rs1], b = regs.arr[rs2];
                            regs.arr[rd] = b == 0 ? (uint32_t)a : (a == INT32_MIN && b == -1) ? 0 : (uint32_t)(a % b);
                        } else {
                            fprintf(stderr, "      Unknown R-type instruction\n");
                            return 1;
                        }
                    } break;
                    case 0x7: {
                        if (funct7 == 0x00) {
                            // printf("      AND instruction\n");
                            regs.arr[rd] = regs.arr[rs1] & regs.arr[rs2];
                        } else if (funct7 == 0x01) {
                            // printf("      REMU instruction\n");
                            regs.arr[rd] = regs.arr[rs2] == 0 ? regs.arr[rs1] : regs.arr[rs1] % regs.arr[rs2];
                        } else {
                            fprintf(stderr, "      Unknown R-type instruction\n");
                            return 1;
                        }
                    } break;
                    default: {
                        fprintf(stderr, "      Unknown R-type instruction\n");
                        return 1;
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
                            regs.arr[rd] = regs.arr[rs1] + imm;
                        } break;
                        case 0x4: {
                            // printf("      XORI instruction\n");
                            regs.arr[rd] = regs.arr[rs1] ^ imm;
                        } break;
                        case 0x6: {
                            // printf("      ORI instruction\n");
                            regs.arr[rd] = regs.arr[rs1] | imm;
                        } break;
                        case 0x7: {
                            // printf("      ANDI instruction\n");
                            regs.arr[rd] = regs.arr[rs1] & imm;
                        } break;
                        case 0x1: {
                            // printf("      SLLI instruction\n");
                            regs.arr[rd] = regs.arr[rs1] << (imm & 0x1F);
                        } break;
                        case 0x5: {
                            if ((imm & 0xFE0) == 0) {
                                // printf("      SRLI instruction\n");
                                regs.arr[rd] = regs.arr[rs1] >> (imm & 0x1F);
                            } else if ((imm & 0xFE0) == 0x400) {
                                // printf("      SRAI instruction\n");
                                regs.arr[rd] = ((int32_t)regs.arr[rs1]) >> (imm & 0x1F);
                            } else {
                                fprintf(stderr, "      Unknown I-type instruction\n");
                                return 1;
                            }
                        } break;
                        case 0x2: {
                            // printf("      SLTI instruction\n");
                            regs.arr[rd] = ((int32_t)regs.arr[rs1] < imm) ? 1 : 0;
                        } break;
                        case 0x3: {
                            // printf("      SLTIU instruction\n");
                            regs.arr[rd] = ((uint32_t)regs.arr[rs1] < (uint32_t)imm) ? 1 : 0;
                        } break;
                    }
                } else if (opcode == 0x03) {
                    switch (funct3) {
                        case 0: {
                            // printf("      LB instruction\n");
                            if (!mem_ok(sizeof(mem), regs.arr[rs1] + imm, 1)) {
                                fprintf(stderr, "Memory access out of bounds\n");
                                return 1;
                            }
                            regs.arr[rd] = (int8_t)mem[regs.arr[rs1] + imm];
                        } break;
                        case 0x1: {
                            // printf("      LH instruction\n");
                            if (!mem_ok(sizeof(mem), regs.arr[rs1] + imm, 2)) {
                                fprintf(stderr, "Memory access out of bounds\n");
                                return 1;
                            }
                            regs.arr[rd] = (int16_t)(mem[regs.arr[rs1] + imm] | (mem[regs.arr[rs1] + imm + 1] << 8));
                        } break;
                        case 0x2: {
                            // printf("      LW instruction\n");
                            if (!mem_ok(sizeof(mem), regs.arr[rs1] + imm, 4)) {
                                fprintf(stderr, "Memory access out of bounds\n");
                                return 1;
                            }
                            regs.arr[rd] = load_word(mem, regs.arr[rs1] + imm);
                        } break;
                        case 0x4: {
                            // printf("      LBU instruction\n");
                            if (!mem_ok(sizeof(mem), regs.arr[rs1] + imm, 1)) {
                                fprintf(stderr, "Memory access out of bounds\n");
                                return 1;
                            }
                            regs.arr[rd] = (uint8_t)mem[regs.arr[rs1] + imm];
                        } break;
                        case 0x5: {
                            // printf("      LHU instruction\n");
                            if (!mem_ok(sizeof(mem), regs.arr[rs1] + imm, 2)) {
                                fprintf(stderr, "Memory access out of bounds\n");
                                return 1;
                            }
                            regs.arr[rd] = (uint16_t)(mem[regs.arr[rs1] + imm] | (mem[regs.arr[rs1] + imm + 1] << 8));
                        } break;
                        default: {
                            fprintf(stderr, "Unknown load instruction\n");
                            return 1;
                        } break;
                    }
                } else if (opcode == 0x67) {
                    // printf("    JALR instruction\n");
                    uint32_t current_rs1 = regs.arr[rs1];
                    regs.arr[rd] = pc + 4;
                    pc = ((current_rs1 + imm) & ~1) - 4;
                } else if (opcode == 0x73) {
                    if (imm == 0) {
                        // printf("      ECALL instruction\n");
                        uint32_t syscall = regs.arr[17]; // a7 register holds the syscall number
                        switch (syscall) {
                            case 64: // write a2 bytes from memory address a1 to file descriptor a0
                                {
                                    uint32_t fd = regs.arr[10]; // a0
                                    uint32_t buf = regs.arr[11]; // a1
                                    uint32_t count = regs.arr[12]; // a2
                                    if (!mem_ok(sizeof(mem), buf, count)) {
                                        fprintf(stderr, "Memory access out of bounds\n");
                                        return 1;
                                    }
                                    ssize_t written = write(fd, &mem[buf], count);
                                    if (written < 0) {
                                        perror("write failed");
                                        return 1;
                                    }
                                }
                                break;
                            case 93: // exit
                                {
                                    uint32_t exit_code = regs.arr[10]; // a0
                                    return exit_code;
                                }
                                break;
                        }
                        
                    } else if (imm == 1) {
                        // printf("      EBREAK instruction\n");
                        fprintf(stderr, "EBREAK encountered\n");
                        return 1;
                    } else {
                        fprintf(stderr, "Unknown SYSTEM instruction\n");
                        return 1;
                    }
                } else {
                    fprintf(stderr, "Unknown I-type instruction\n");
                    return 1;
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
                        if (!mem_ok(sizeof(mem), regs.arr[rs1] + imm, 1)) {
                            fprintf(stderr, "Memory access out of bounds\n");
                            return 1;
                        }
                        mem[regs.arr[rs1] + imm] = regs.arr[rs2] & 0xFF;
                    } break;
                    case 0x1: {
                        // printf("      SH instruction\n");
                        if (!mem_ok(sizeof(mem), regs.arr[rs1] + imm, 2)) {
                            fprintf(stderr, "Memory access out of bounds\n");
                            return 1;
                        }
                        mem[regs.arr[rs1] + imm] = regs.arr[rs2] & 0xFF;
                        mem[regs.arr[rs1] + imm + 1] = (regs.arr[rs2] >> 8) & 0xFF;
                    } break;
                    case 0x2: {
                        // printf("      SW instruction\n");
                        if (!mem_ok(sizeof(mem), regs.arr[rs1] + imm, 4)) {
                            fprintf(stderr, "Memory access out of bounds\n");
                            return 1;
                        }
                        mem[regs.arr[rs1] + imm] = regs.arr[rs2] & 0xFF;
                        mem[regs.arr[rs1] + imm + 1] = (regs.arr[rs2] >> 8) & 0xFF;
                        mem[regs.arr[rs1] + imm + 2] = (regs.arr[rs2] >> 16) & 0xFF;
                        mem[regs.arr[rs1] + imm + 3] = (regs.arr[rs2] >> 24) & 0xFF;
                    } break;
                    default: {
                        fprintf(stderr, "Unknown store instruction\n");
                        return 1;
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
                        if (regs.arr[rs1] == regs.arr[rs2]) {
                            pc += imm - 4;
                        }
                    } break;
                    case 0x1: {
                        // printf("      BNE instruction\n");
                        if (regs.arr[rs1] != regs.arr[rs2]) {
                            pc += imm - 4;
                        }
                    } break;
                    case 0x4: {
                        // printf("      BLT instruction\n");
                        if ((int32_t)regs.arr[rs1] < (int32_t)regs.arr[rs2]) {
                            pc += imm - 4;
                        }
                    } break;
                    case 0x5: {
                        // printf("      BGE instruction\n");
                        if ((int32_t)regs.arr[rs1] >= (int32_t)regs.arr[rs2]) {
                            pc += imm - 4;
                        }
                    } break;
                    case 0x6: {
                        // printf("      BLTU instruction\n");
                        if ((uint32_t)regs.arr[rs1] < (uint32_t)regs.arr[rs2]) {
                            pc += imm - 4;
                        }
                    } break;
                    case 0x7: {
                        // printf("      BGEU instruction\n");
                        if ((uint32_t)regs.arr[rs1] >= (uint32_t)regs.arr[rs2]) {
                            pc += imm - 4;
                        }
                    } break;
                    default: {
                        fprintf(stderr, "      Unknown B-type instruction\n");
                        return 1;
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
                        regs.arr[rd] = imm;
                    } break;
                    case 0x17: {
                        // printf("      AUIPC instruction\n");
                        regs.arr[rd] = pc + imm;
                    } break;
                    default: {
                        fprintf(stderr, "      Unknown U-type instruction\n");
                        return 1;
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
                    goto done;
                }
                regs.arr[rd] = pc + 4;
                pc += imm - 4;
            } break;
        }

        pc += 4;
        regs.arr[0] = 0;  // x0 is always 0
    }
done:
    printf("\n");
    if (i == MAX_ITER + 1) {
        fprintf(stderr, "Maximum number of iterations reached\n");
    }
    // printf("Number of iterations: %zu\n", i);

    // for (int i = 0; i < 32; i++) {
    //     printf("x%02d: %d\n", i, regs.arr[i]);
    // }

    printf("> Program returned %d\n", regs.arr[10]);

    return 0;
}

static bool mem_ok(size_t mem_size, uint32_t addr, size_t n) {
    return (uint64_t)addr + n <= mem_size;   // reject out-of-range access
}

static uint32_t load_word(const uint8_t *mem, uint32_t addr) {
    return mem[addr] | mem[addr + 1] << 8 | mem[addr + 2] << 16 | (uint32_t)mem[addr + 3] << 24;
}

static void store_word(uint8_t *mem, uint32_t addr, uint32_t val) {
    mem[addr]     = val & 0xFF;
    mem[addr + 1] = (val >> 8) & 0xFF;
    mem[addr + 2] = (val >> 16) & 0xFF;
    mem[addr + 3] = val >> 24;
}
