#include <type.h>
#include <boot.h>

static unsigned char buff[512] = {0};

void boot_main()
{
    display_init();
    display_clear();
    printf("Hell... Wait! It's not time!\n");
    ide_init();
    disk_mbr_init();
    disk_print_info();
    disk_read(0, (void*)buff, 0, 1);
    disk_write(0, (void*)buff, 1, 1);
    while(1);
}