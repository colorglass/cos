#include <type.h>
#include <utils.h>

#define PIC1 0x20
#define PIC2 0xA0
#define PIC1_COMMAND PIC1
#define PIC1_DATA (PIC1 + 1)
#define PIC2_COMMAND PIC2
#define PIC2_DATA (PIC2 + 1)

#define ICW1_ICW4 0x01
#define ICW1_INIT 0x10
#define ICW4_8086 0x01

void pic_init(u8 offset) {

    // send init cmd
    outb(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4);
    outb(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);

    // set vector offset
    outb(PIC1_DATA, offset);
    outb(PIC2_DATA, offset + 8);

    // set cascade identity
    outb(PIC1_DATA, 0b100);
    outb(PIC2_DATA, 2);

    // set 8086 mode
    outb(PIC1_DATA, ICW4_8086);
    outb(PIC2_DATA, ICW4_8086);
}

void pic_eoi(u8 irq) {
    if(irq >= 8) {
        outb(PIC2_COMMAND, 0x20);
    }
    outb(PIC1_COMMAND, 0x20);
}

void pic_mask_irq(u8 irq, bool mask) {
    u16 port;
    u8 value;
    if(irq < 8) {
        port = PIC1_DATA;
    } else {
        port = PIC2_DATA;
        irq -= 8;
    }

    value = inb(port);
    value = mask ? value | (1 << irq) : value & ~(1 << irq);
    outb(port, value);
}

void pic_mask_all() {
    outb(PIC1_DATA, 0xff);
    outb(PIC2_DATA, 0xff);
}