#include <printf.h>
#include <utils.h>
#include <type.h>
#include <irq.h>
#include <kmalloc.h>

int kernel_main(u32 mem_map_addr)
{
    u32 kernel_paddr;

    display_init();
    display_clear();
    printf("This is kernel!\n");
    
    gdt_init();
    irq_init_idt();

    kmalloc_init();
    u32 *a1 = kmalloc(sizeof(u32) * 10);
    for(int i = 0; i < 10; i++) {
        a1[i] = 1;
    }
    u32 *a2 = kmalloc(sizeof(u32) * 4);
    for(int i = 0; i < 4; i++) {
        a2[i] = 2;
    }
    kfree(a1);
    u32 *a3 = kmalloc(sizeof(u32));
    *a3 = 3;
    kfree(a3);
    u32 *a4 = kmalloc(sizeof(u32));
    printf("%d",*a4); 
    // setup gdt
    // setup irq
    // setup mem
    // setup page
    while(1);
}