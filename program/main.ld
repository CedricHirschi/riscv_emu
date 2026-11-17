ENTRY(_start)

/* Must match MEMORY_*_BASE / MEMORY_*_SIZE in memory.h */
MEMORY {
    ROM (rx)  : ORIGIN = 0x00000000, LENGTH = 0x00100000
    RAM (rw)  : ORIGIN = 0x80000000, LENGTH = 0x20000000
}

PHDRS {
    text PT_LOAD FLAGS(5); /* R+X */
    data PT_LOAD FLAGS(6); /* R+W */
}

SECTIONS {
    .text : {
        KEEP(*(.text.startup))
        *(.text .text.*)
    } > ROM :text

    .rodata : { *(.rodata .rodata.* .srodata .srodata.*) } > ROM :text

    /* The emulator loads segments straight to their vaddr, so .data needs no copy from ROM */
    .data : { *(.data .data.* .sdata .sdata.*) } > RAM :data

    .bss (NOLOAD) : { *(.bss .bss.* .sbss .sbss.*) } > RAM :data

    __stack_top = ORIGIN(RAM) + LENGTH(RAM);

    /DISCARD/ : {
        *(.comment) *(.note*)
        *(.eh_frame*) *(.riscv.attributes)
    }
}