# riscv_emu

A simple RISC-V (RV32IM) ELF emulator written in C.

## Features

- 32-bit RISC-V ELF loader (`elf_parse.c`, `memory.c`)
- RV32IM instruction decoding and execution with a pre-decoded instruction cache (`cpu.c`, `instruction.c`)
- Basic syscall support (`write` = 64, `exit` = 93) (`syscall.c`)
- Sample freestanding RISC-V program (`program/`) and unit tests (`tests/`)

## Building & Running

Build the emulator and run the sample program (`program/main.elf`):

```sh
make
```

Run an arbitrary 32-bit RISC-V ELF binary:

```sh
./main <elf_file>
```

Run the test suite:

```sh
make run_tests
```

Clean build artifacts:

```sh
make clean
```

> **Note:** Rebuilding `program/main.elf` requires a `riscv32-unknown-elf-gcc` toolchain (configured in `program/Makefile`).

## License

Licensed under the Apache License, Version 2.0 (Apache-2.0).
