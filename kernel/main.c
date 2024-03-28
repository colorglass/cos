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
    irq_init_idt();

    kheap_init();
    pmm_init((struct mem_map*)mem_map_addr);
    page_init();
    vmm_init();

    uintptr_t frame1 = pmm_alloc(1);
    page_map(0xf1000000, frame1, KERNEL_PAGE_FLAGS);
    u32* p = (u32*)0xf1000000;
    *p = 0x12345678;
    printf("p: %x\n", *p);
    page_unmap(0xf1000000);
    uintptr_t frame2 = pmm_alloc(16);
    pmm_free(frame1, 1);
    uintptr_t frame3 = pmm_alloc(4);

    u8* p1 = (u8*)vmm_alloc(0x1000);
    for(int i = 0; i < 0x1000; i++) {
        p1[i] = i;
    }
    u8* p2 = (u8*)vmm_alloc(0x3000);
    for(int i = 0; i < 0x3000; i++) {
        p2[i] = i;
    }
    u8* p3 = (u8*)vmm_alloc(0x2000);
    for(int i = 0; i < 0x2000; i++) {
        p3[i] = i;
    }
    vmm_free(p1);
    u8* p4 = vmm_alloc(0x1000);
    for(int i = 0; i < 0x1000; i++) {
        p4[i] = i;
    }
    printf("p4: %x\n", p4);
    printf("p4[0x1000]: %x\n", *(u32*)(p4 + 0xffc));

    printf("frame1: %x\n", frame1);
    printf("frame2: %x\n", frame2);
    printf("frame3: %x\n", frame3);
    // setup gdt
    // setup irq
    // setup mem
    // setup page
    panic("kernel end");
}