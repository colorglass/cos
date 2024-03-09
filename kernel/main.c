#include <printf.h>
#include <utils.h>
#include <type.h>
#include <irq.h>
#include <kalloc.h>

int kernel_main(u32 mem_map_addr)
{
    u32 kernel_paddr;

    display_init();
    display_clear();
    printf("This is kernel!\n");
    
    gdt_init();
    irq_init_idt();

    kalloc_init();
    u32 *array = kalloc(sizeof(u32) * 10);
    // setup gdt
    // setup irq
    // setup mem
    // setup page
    while(1);
}