#include <printf.h>
#include <utils.h>
#include <type.h>
#include <irq.h>

int kernel_main(u32 mem_map_addr)
{
    u32 kernel_paddr;

    display_init();
    display_clear();
    printf("This is kernel!\n");
    
    gdt_init();
    irq_init_idt();

    struct mem_map* mem_map = (struct mem_map*)mem_map_addr;
    for(int i = 0; i < mem_map->size; i++) {
        if(mem_map->maps[i].type == MEM_TYPE_KERNEL)
            kernel_paddr = mem_map->maps[i].base;
    }

    
    // setup gdt
    // setup irq
    // setup mem
    // setup page
    while(1);
}