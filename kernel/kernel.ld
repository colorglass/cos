ENTRY(_start)
OUTPUT_FORMAT(elf32-i386)

PHDRS
{
    text    PT_LOAD    FLAGS((1 << 0) | (1 << 2)) ; /* Execute + Read */
    rodata  PT_LOAD    FLAGS((1 << 2)) ;            /* Read only */
    data    PT_LOAD    FLAGS((1 << 1) | (1 << 2)) ; /* Write + Read */
}

SECTIONS
{
    . = 0xc0000000;
    __kernel_start = .;
    .text : { *(.text) } :text
    . += CONSTANT(MAXPAGESIZE);
    .rodata : { *(.rodata) } :rodata
    . += CONSTANT(MAXPAGESIZE);
    .data : { *(.data .data.*) } :data
    .bss : { *(.bss .bss.*) *(.stack) *(.heap) } :data
    __kernel_end = .;
}