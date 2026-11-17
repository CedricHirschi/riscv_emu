# riscv_emu

A simple RISC-V (RV32IM) ELF emulator written in C.

## Features

- 32-bit RISC-V ELF loader (`elf_parse.c`)
- Guest memory map (`memory.c`), mirrored by the linker script `program/main.ld`:

  | Region | Base         | Size    | Access                                  |
  |--------|--------------|---------|-----------------------------------------|
  | ROM    | `0x00000000` | 1 MiB   | read + execute (code, `.rodata`)        |
  | MMIO   | `0x10000000` | 256 MiB | read + write (reserved for peripherals) |
  | RAM    | `0x80000000` | 512 MiB | read + write (`.data`, `.bss`, stack)   |
- RV32IM instruction decoding and execution with a pre-decoded instruction cache (`cpu.c`, `instruction.c`)
- Basic syscall support (`write` = 64, `exit` = 93) (`syscall.c`)
- Sample freestanding RISC-V program (`program/`) and unit tests (`tests/`)

## Building & Running

All build output goes to `build/`. The root `Makefile` builds the emulator; `program/` and `tests/` have their own Makefiles that can be used with `make -C`:

```sh
make                 # build the emulator: build/main
make -C program      # build the guest programs: build/program/*.elf
make -C tests run    # build and run all tests (builds emulator and programs as needed)
make clean           # remove build/
```

Shortcuts from the root: `make run_main` runs `build/program/main.elf`, `make run_tests` runs the test suite.

Run an arbitrary 32-bit RISC-V ELF binary:

```sh
build/main <elf_file>
```

> **Note:** Building the guest programs requires a `riscv32-unknown-elf-gcc` toolchain (configured in `program/Makefile`).

## License

Licensed under the Apache License, Version 2.0 (Apache-2.0).
