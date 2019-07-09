OUTPUT_FORMAT("elf32-littlearm", "elf32-littlearm", "elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(reset_handler)

SECTIONS {

    .text :
    {
        KEEP(*(.cortex_vectors))
        KEEP(*(.vendor_vectors))
        *(.text .text.* .gnu.linkonce.t.*)
        *(.glue_7t) *(.glue_7)
        *(.rodata .rodata* .gnu.linkonce.r.*)
        *(.ARM.extab* .gnu.linkonce.armextab.*)
         . = ALIGN(4);
        _end_text = .;
    } >flash

    . = ALIGN(4);

    .data :
    {
        _start_data = .;
        *(.ramfunc .ramfunc.*);
        *(.data .data.*);
        . = ALIGN(4);
         _end_data = .;
    } >sram AT >flash

    . = ALIGN(4);

    .bss (NOLOAD) :
    {
        . = ALIGN(4);
        _start_bss = .;
        *(.bss .bss.*)
        *(COMMON)
        . = ALIGN(4);
        _end_bss = .;
    } >sram

    . = ALIGN(4);

    _start_stack = .;
    PROVIDE(_end_stack = ORIGIN(sram) + LENGTH(sram));
}

_end = .;
