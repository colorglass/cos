#include <type.h>
#include <boot.h>
#include <utils.h>

// bootloader image memory end address
extern u8 _boot_end[];
extern u8 _mem_map[];

// bootloader entry
void boot_main()
{
    display_init();
    display_clear();

    printf("Hell... Wait! It's not time!\n");
    
    ide_init();
    disk_init();
    mem_map_init();
    page_init();

    int boot_parti = 0;
    u32 kernel_elf_size = 0;

    // warn: kernel elf size should not be larger than the free memory gap in the early 1MB memory
    char* kernel_elf_addr = (char*)ALIGN_UP((u32)&_boot_end[0], PAGE_SIZE);
    while((boot_parti = disk_find_next_bootable(boot_parti)) >= 0)
    {
        printf("Find bootable partition: %d\n", boot_parti);

        // todo: check file system type here
        fat_init(boot_parti);
        fat_print_fat();

        kernel_elf_size = fat_load_kernel((void*)kernel_elf_addr);
        if(kernel_elf_size == -1) {
            printf("not find kernel.elf in the partition %d\n", boot_parti);
            boot_parti++;
            continue;
        }

        u32 mem_size = elf_get_mem_size(kernel_elf_addr);
        u32 kernel_load_paddr = mem_find_kernel_available(mem_size);
        u32 entry = elf_load_kernel(kernel_elf_addr, kernel_load_paddr);

        // not setup kernel stack yet
        // pass the mem map location to os
        // gcc do not preserve eax, ecx, edx so we use ebx to pass arg
        asm volatile ("mov %0, %%ebx"::"r"(_mem_map));
        ((int (*)(void))entry)();
        // shoud not reach here
        break;
    }

    printf("No bootable partition found!\n");

    while(1);
}