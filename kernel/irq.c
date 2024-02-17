#include <type.h>
#include <mem.h>
#include <sys.h>
#include <irq.h>

struct idt_entry {
    u16 base_lo;
    u16 sel;
    u8 zero;
    u8 flags;
    u16 base_hi;
} __attribute__((packed));

struct idt_ptr {
    u16 padding;
    u16 limit;
    u32 base;
} __attribute__((packed));

struct irq_frame {
    u32 ds;
    u32 edi, esi, ebp, esp, ebx, edx, ecx, eax;
    u32 int_no, err_code;
    u32 eip, cs, eflags, useresp, ss;
};

static struct idt_entry idt[256] __attribute__((aligned(PAGE_SIZE)));
static struct idt_ptr idt_ptr __attribute__((aligned((4))));

static struct idt_entry idt_make_entry(u32 offset, bool user_access)
{
    struct idt_entry entry;
    entry.base_lo = offset & 0xffff;
    entry.base_hi = (offset >> 16) & 0xffff;
    entry.sel = KERNEL_CODE_SEL;
    entry.zero = 0;
    entry.flags = 0x8e | (user_access ? (USER_DPL << 5) : (KERNEL_DPL << 5));
    return entry;
}

void irq_init_idt()
{
    memset(&idt, 0, sizeof(idt));

    idt[0] = idt_make_entry((u32)isr_0, false);
    idt[1] = idt_make_entry((u32)isr_1, false);
    idt[2] = idt_make_entry((u32)isr_2, false);
    idt[3] = idt_make_entry((u32)isr_3, false);
    idt[4] = idt_make_entry((u32)isr_4, false);
    idt[5] = idt_make_entry((u32)isr_5, false);
    idt[6] = idt_make_entry((u32)isr_6, false);
    idt[7] = idt_make_entry((u32)isr_7, false);
    idt[8] = idt_make_entry((u32)isr_8, false);
    idt[9] = idt_make_entry((u32)isr_9, false);
    idt[10] = idt_make_entry((u32)isr_10, false);
    idt[11] = idt_make_entry((u32)isr_11, false);
    idt[12] = idt_make_entry((u32)isr_12, false);
    idt[13] = idt_make_entry((u32)isr_13, false);
    idt[14] = idt_make_entry((u32)isr_14, false);
    idt[15] = idt_make_entry((u32)isr_15, false);
    idt[16] = idt_make_entry((u32)isr_16, false);
    idt[17] = idt_make_entry((u32)isr_17, false);
    idt[18] = idt_make_entry((u32)isr_18, false);
    idt[19] = idt_make_entry((u32)isr_19, false);
    idt[20] = idt_make_entry((u32)isr_20, false);
    idt[21] = idt_make_entry((u32)isr_21, false);
    idt[22] = idt_make_entry((u32)isr_22, false);
    idt[23] = idt_make_entry((u32)isr_23, false);
    idt[24] = idt_make_entry((u32)isr_24, false);
    idt[25] = idt_make_entry((u32)isr_25, false);
    idt[26] = idt_make_entry((u32)isr_26, false);
    idt[27] = idt_make_entry((u32)isr_27, false);
    idt[28] = idt_make_entry((u32)isr_28, false);
    idt[29] = idt_make_entry((u32)isr_29, false);
    idt[30] = idt_make_entry((u32)isr_30, false);
    idt[31] = idt_make_entry((u32)isr_31, false);

    idt_ptr.limit = sizeof(idt) - 1;
    idt_ptr.base = (u32)&idt[0];
    asm volatile ("lidt %0"::"m"(idt_ptr.limit));
}

void isr_handler(struct irq_frame frame)
{
    printf("isr_handler: int_no=%d, err_code=%d\n", frame.int_no, frame.err_code);
}