#include <type.h>
#include <boot.h>

#define KERNEL_ELF 0x20000
static unsigned char buff[512] = {0};

void boot_main()
{
    display_init();
    display_clear();
    printf("Hell... Wait! It's not time!\n");
    ide_init();
    disk_init();
    disk_print_info();

    int boot_parti = 0;
    u32 kernel_elf_size = 0;
    while((boot_parti = disk_find_next_bootable(boot_parti)) >= 0)
    {
        printf("Find bootable partition: %d\n", boot_parti);
        fat_init(boot_parti);
        fat_print_fat();
        kernel_elf_size = fat_load_kernel(KERNEL_ELF);
        elf_load_kernel((void*)KERNEL_ELF, (void*)0x100000);

        boot_parti++;
    }
    while(1);
}