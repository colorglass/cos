
    .code16
    .section .bootsector, "awx"
    .global _start
_start:
    mov %dl, boot_drive
    xor %ax, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %ss
    mov $0x7c00, %sp

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
    mov $0xe820, %eax
    mov $0x534d4150, %edx
    xor %ebx, %ebx
    mov $0x14, %ecx
    mov $map, %di
1:
    int $0x15
    jc failed_detect
    incb entry
    add $0x14, %di
    cmp $0, %ebx
    jne 1b

load_disk:
    mov $0x42, %ah
    mov boot_drive, %dl
    mov disk_addr_packet, %si
    jc failed_load

enter_pm:
    cli
    lgdt boot_gdtr
    mov %cr0, %eax
    or $1, %eax
    mov %eax, %cr0
    jmp $0x08, $after_pm


    .code32
after_pm:
    mov $0x10, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
    mov %ax, %ss
    mov $0x7c00, %esp


    jmp failed

print_msg:
    mov $0x1301, %ax
    mov $0x000f, %bx
    xor %dx, %dx
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

boot_drive: .byte 0x00

disk_addr_packet:
    .2byte 0x10
    .2byte 0x200
    .4byte 0x8000
    .8byte 0x1

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

boot_msg: .string "System booting!\n"
boot_msg_len = . - boot_msg

failed_detect_msg: .string "Failed to detect memory map!\n"
failed_detect_msg_len = . - failed_detect_msg

failed_load_msg: .string "Failed to load disk!\n"
failed_load_msg_len = . - failed_load_msg

    .org 510
    .2byte 0xaa55


    .section .mem_map, "aw"
    .global _mem_map
_mem_map:
    entry: .2byte 0x0
    map: