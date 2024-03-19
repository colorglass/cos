ENTRY(_start)
OUTPUT_FORMAT(elf32-i386)
SECTIONS
{
    . = 0xc0000000;
    __kernel_start = .;
    .stack : { *(.stack )}
    .text : { *(.text) }
    .rodata : { *(.rodata) }
    .data : { *(.data) }
    .bss : { *(.bss) }
    .heap : {*(.heap)}
    __kernel_end = .;
}