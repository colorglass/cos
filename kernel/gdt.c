#include <type.h>
#include <mem.h>

struct gdt_entry {
    u32 bits_low;
    u32 bits_high;
}__attribute__((packed));

struct gdt_ptr{
    // use padding to offer better alignment
    u16 paddding;
    u16 limit;
    u32 base;
}__attribute__((packed));

static struct gdt_entry gdt[5] __attribute__((aligned(8)));
static struct gdt_ptr gdt_ptr __attribute__((aligned(4)));

int gdt_init()
{
    memset(gdt, 0, sizeof(struct gdt_entry));

    gdt[1].bits_low = 0x0000ffff;
    gdt[1].bits_high = 0x00cf9a00;

    gdt[2].bits_low = 0x0000ffff;
    gdt[2].bits_high = 0x00cf9200;

    gdt[3].bits_low = 0x0000ffff;
    gdt[3].bits_high = 0x00cffa00;

    gdt[4].bits_low = 0x0000ffff;
    gdt[4].bits_high = 0x00cff200;

    gdt_ptr.limit = sizeof(gdt) - 1;
}