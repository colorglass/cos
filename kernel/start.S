
    .global _start, _stack_bottom, _stack_top

    .text
_start:
    mov $_stack_top, %esp
    mov $_stack_top, %ebp
    push %ebx
    call kernel_main

1:
    hlt
    jmp 1b

    .section .stack, "aw", @nobits
    .align 0x1000
_stack_bottom:
    .skip 0x4000
_stack_top:
