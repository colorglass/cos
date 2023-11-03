#include <type.h>
#include <boot.h>
#include <utils.h>

// bootloader image memory end address
extern u8 _boot_end[];

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
    char* kernel_elf_addr = (char*)ROUND_UP((u32)&_boot_end[0], PAGE_SIZE);
    while((boot_parti = disk_find_next_bootable(boot_parti)) >= 0)
    {
        printf("Find bootable partition: %d\n", boot_parti);

        fat_init(boot_parti);
        fat_print_fat();

        kernel_elf_size = fat_load_kernel((void*)kernel_elf_addr);
        if(kernel_elf_size == 0) {
            printf("not find kernel.elf in the partition %d\n", boot_parti);
            boot_parti++;
            continue;
        }

        elf_load_kernel(kernel_elf_addr);
        break;
    }

    printf("No bootable partition found!\n");

    while(1);
}