ENTRY(_mbr)
OUTPUT_FORMAT(elf32-i386)
SECTIONS
{
    . = 0x7c00;
    .bootsector : { *(.bootsector) }
    .mem_map : { *(.mem_map) }
    . = 0x8000;
    .text : { *(.text) }
    .rodata : { *(.rodata) }
    .data : { *(.data) }
    .bss : { *(.bss) }
}