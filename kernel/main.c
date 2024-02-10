#include <printf.h>
#include <utils.h>
#include <type.h>

int kernel_main(u32 mem_map_addr)
{
    u32 kernel_paddr;

    display_init();
    display_clear();
    gdt_init();
    printf("This is kernel!\n");

    struct mem_map* mem_map = (struct mem_map*)mem_map_addr;
    for(int i = 0; i < mem_map->size; i++) {
        if(mem_map->maps[i].type == MEM_TYPE_KERNEL)
            kernel_paddr = mem_map->maps[i].base;
    }

    union {
        struct {
            u32 eax;
            u32 edx;
            u32 ecx; 
        } regs;
        char str[13];
    } cpuid;

    asm volatile(
        "mov $0, %%eax;"
        "cpuid;"
        "mov %%ebx, %0;"
        "mov %%edx, %1;"
        "mov %%ecx, %2;"
        :"=m"(cpuid.regs.eax),"=m"(cpuid.regs.edx),"=m"(cpuid.regs.ecx)
        ::
    );

    cpuid.str[12] = '\0';
    printf("get cpuid: %s\n", cpuid.str);
    // setup gdt
    // setup irq
    // setup mem
    // setup page
    while(1);
}