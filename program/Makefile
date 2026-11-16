CC := riscv riscv32-unknown-elf-gcc

CFLAGS := -march=rv32i -mabi=ilp32 \
          -Os -ffreestanding -fno-builtin \
          -fno-stack-protector \
          -fno-unwind-tables -fno-asynchronous-unwind-tables

LDFLAGS := -nostdlib \
           -Wl,-T,main.ld \
           -Wl,-z,max-page-size=4 \
           -Wl,--no-relax \
           -Wl,--build-id=none

main.elf: main.c main.ld
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $<

clean:
	rm -f main.elf