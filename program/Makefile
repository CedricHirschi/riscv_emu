# Guest programs: each <name>.c here becomes $(BUILD)/program/<name>.elf,
# linked with the runtime in src/ and crt0.S.

BUILD ?= $(abspath ../build)
OUT   := $(BUILD)/program

CC      := riscv riscv32-unknown-elf-gcc
OBJDUMP := riscv riscv32-unknown-elf-objdump

CFLAGS := -march=rv32im -mabi=ilp32 \
          -Os -ffreestanding -fno-builtin \
          -ffunction-sections -fdata-sections -fno-common -Wall -Wextra -Werror \
          -Iinc -I../inc/devices # device register layouts are shared with the emulator

LDFLAGS := -nostdlib \
           -Wl,-T,main.ld \
           -Wl,-z,max-page-size=4 \
           -Wl,--no-relax \
           -Wl,--build-id=none

RUNTIME  := $(wildcard src/*.c) crt0.S
HEADERS  := $(wildcard inc/*.h) $(wildcard ../inc/devices/*_regs.h)
PROGRAMS := $(patsubst %.c,$(OUT)/%,$(wildcard *.c))

all: $(PROGRAMS:=.elf) $(PROGRAMS:=.dump)

$(OUT)/%.elf: %.c $(RUNTIME) $(HEADERS) main.ld
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(filter %.c %.S,$^)

$(OUT)/%.dump: $(OUT)/%.elf
	$(OBJDUMP) --visualize-jumps -D -s $< > $@

clean:
	rm -rf $(OUT)

.PHONY: all clean
