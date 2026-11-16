ENTRY(_start)

PHDRS {
    load PT_LOAD FLAGS(7);
}

SECTIONS {
    . = 0;

    .text : {
        KEEP(*(.text.startup))
        *(.text .text.*)
    } :load

    .rodata : { *(.rodata .rodata.*) } :load

    . = ALIGN(16);
    .bss (NOLOAD) : {
        *(.bss .bss.*)
        . = ALIGN(16);
        . += 4096;
        __stack_top = .;
    } :load

    /DISCARD/ : {
        *(.comment) *(.note*)
        *(.eh_frame*) *(.riscv.attributes)
    }
}
