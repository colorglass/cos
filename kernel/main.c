#include <printf.h>
#include <utils.h>
#include <type.h>
#include <irq.h>
#include <kmalloc.h>
#include <pmm.h>

int kernel_main(u32 mem_map_addr)
{
    display_init();
    display_clear();
    printf("This is kernel!\n");
    
    gdt_init();
    irq_init_idt();

    kmalloc_init();
    pmm_init((struct mem_map*)mem_map_addr);
    uintptr_t frame1 = pmm_alloc(1);
    uintptr_t frame2 = pmm_alloc(1024);
    pmm_free(frame1, 1);
    uintptr_t frame3 = pmm_alloc(4);

    printf("frame1: %x\n", frame1);
    printf("frame2: %x\n", frame2);
    printf("frame3: %x\n", frame3);
    // setup gdt
    // setup irq
    // setup mem
    // setup page
    while(1);
}