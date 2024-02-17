.macro ISR_NOERRCODE isr_num
.global isr_\isr_num
isr_\isr_num:
    push $0
    push $\isr_num
    jmp isr_handler_stub
.endm

.macro ISR_ERRCODE isr_num
.global isr_\isr_num
isr_\isr_num:
    push $\isr_num
    jmp isr_handler_stub
.endm

.macro IRQ irq_num
.global irq_\irq_num
irq_\irq_num:
    push $0
    push $(\irq_num + 32)
    jmp isr_handler_stub
.endm

// exceptions with no error code
.irp i, 0, 1, 2, 3, 4, 5, 6, 7, 9, 15, 16, 18, 19, 20, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31
    ISR_NOERRCODE \i
.endr

// exceptions with error code
.irp i, 8, 10, 11, 12, 13, 14, 17, 21
    ISR_ERRCODE \i
.endr

// IRQs
.irp i, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15
    IRQ \i
.endr

.extern mystack

.text
isr_handler_stub:
    pusha

    // store the ds in case in case it's from userland
    push %ds

    // switch to kernel data segment
    mov $0x10, %eax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs

    // call into the C handler
    call isr_handler

    // restore the ds
    pop %eax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs

    popa

    // pop the error code and irq number
    add $8, %esp

    iret