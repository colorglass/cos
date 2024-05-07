#include <type.h>
#include <printf.h>
#include <irq.h>
#include <utils.h>

#define PIT_FREQ 1193182

static u32 sys_tick;
static u32 sys_time;

static void irq_handler() {
    sys_tick++;
    if(sys_tick >= 100) {
        sys_tick -= 100;
        sys_time++;
        printf("sys time: %d\n", sys_time);
    }
}

void pit_init() {
    u16 divisor = PIT_FREQ / 100;
    outb(0x43, 0x36);
    outb(0x40, divisor & 0xFF);
    outb(0x40, (divisor >> 8) & 0xFF);

    irq_register(0, irq_handler);
}