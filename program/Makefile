CC := riscv riscv32-unknown-elf-gcc
OBJDUMP := riscv riscv32-unknown-elf-objdump

CFLAGS := -march=rv32im -mabi=ilp32 \
          -Os -ffreestanding -fno-builtin \
          -ffunction-sections -fdata-sections -fno-common -Wall -Wextra -Werror

LDFLAGS := -nostdlib \
           -Wl,-T,main.ld \
           -Wl,-z,max-page-size=4 \
           -Wl,--no-relax \
           -Wl,--build-id=none

CFLAGS_NATIVE := -Os -ffunction-sections -fdata-sections -fno-common -Wall -Wextra -Werror

all: main.elf trap_test.elf main.dump

%.elf: %.c lib/print.c lib/trap.c main.ld crt0.S
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(filter-out main.ld,$^) -Ilib -I..

main.dump: main.elf
	$(OBJDUMP) --visualize-jumps -D -s $< > $@

clean:
	rm -f main.elf trap_test.elf main.dump
