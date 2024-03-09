
    .code16
    .section .bootsector, "awx"
    .global _mbr
_mbr:
    cli

    mov %dl, boot_drive     // save boot drive num
    xor %ax, %ax            // init segment registers
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %ss
    mov $0x7c00, %sp

    // print boot message
    mov $boot_msg, %bp
    mov $boot_msg_len, %cx 
    call print_msg

enable_A20:
    in $0x92, %al
    test $2, %al
    jnz after
    or $2, %al
    and $0xfe, %al
    out %al, $0x92
after:

detect_memory:
    xor %ebx, %ebx
    xor %bp, %bp
    mov $0x14, %ecx     // map struct size = 20 bytes
    mov $map, %di
1:
    mov $0xe820, %eax
    mov $0x534d4150, %edx   // 'SMAP'
    int $0x15
    jc failed_detect
    inc %bp             // map entry count ++
    add $0x14, %di      // move to next map entry
    test %ebx, %ebx     // if detect end ?
    jnz 1b
    movw %bp, entry     // save map entry count

load_disk:
    mov $0x42, %ah
    mov boot_drive, %dl
    mov $disk_addr_packet, %si
    int $0x13
    jc failed_load

enter_pm:
    cli
    lgdt boot_gdtr
    mov %cr0, %eax
    or $1, %eax
    mov %eax, %cr0
    jmp $0x08, $after_pm

// %cx = msg_len, %bp = msg_addr
print_msg:
    push %cx
    push %bp

    // get current cursor pos
    mov $0x3, %ah
    xor %bh, %bh
    int $0x10

    pop %bp
    pop %cx

    mov $0x1301, %ax
    mov $0x000f, %bx
    int $0x10
    ret

failed_detect:
    mov $failed_detect_msg, %bp
    mov $failed_detect_msg_len, %cx
    call print_msg
    jmp failed

failed_load:
    mov $failed_load_msg, %bp
    mov $failed_load_msg_len, %cx
    call print_msg
    jmp failed

failed:
    cli
    hlt
    jmp failed


    .code32
after_pm:
    mov $0x10, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
    mov %ax, %ss
    mov $0x7c00, %esp

    call boot_main

2:
    cli
    hlt
    jmp 2b

boot_drive: .byte 0x00

disk_addr_packet:
    .2byte 0x10
    .2byte 0x40
    .4byte 0x8000
    .8byte 0x2

boot_gdt:
    .4byte 0x0
    .4byte 0x0
    
    .4byte 0x0000ffff
    .4byte 0x00cf9a00

    .4byte 0x0000ffff
    .4byte 0x00cf9200

boot_gdtr:
    .2byte 0x17
    .4byte boot_gdt

boot_msg: .ascii "System booting!\r\n"
boot_msg_len = . - boot_msg

failed_detect_msg: .ascii "Failed to detect memory map!\r\n"
failed_detect_msg_len = . - failed_detect_msg

failed_load_msg: .ascii "Failed to load disk!\r\n"
failed_load_msg_len = . - failed_load_msg

    .org 0x1fe
    .2byte 0xaa55


    .section .mem_map, "aw"
    .global _mem_map
_mem_map:
    entry: .4byte 0x0
    map:
