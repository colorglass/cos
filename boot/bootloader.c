#include <type.h>
#include <boot.h>

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
    while((boot_parti = disk_find_next_bootable(boot_parti)) >= 0)
    {
        printf("Find bootable partition: %d\n", boot_parti);
        fat_init(boot_parti);
        fat_print_fat();
        fat_load_kernel();
        boot_parti++;
    }
    while(1);
}