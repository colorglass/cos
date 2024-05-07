#include <printf.h>
#include <utils.h>
#include <type.h>
#include <irq.h>
#include <kmalloc.h>
#include <pmm.h>
#include <page.h>
#include <gdt.h>

void kernel_main(u32 mem_map_addr)
{
    display_init();
    display_clear();
    printf("This is kernel!\n");
    
    gdt_init();
    irq_init();

    kheap_init();
    pmm_init((struct mem_map*)mem_map_addr);
    page_init();
    vmm_init();

    pit_init();

    // init drivers
    

    printf("Kernel end\n");
    while(1) {
        asm volatile ("hlt");
    }
}