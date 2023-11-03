#include <type.h>
#include <boot.h>

static unsigned char buff_1[512] = {[0 ... 511] = 0x55};
static unsigned char buff_2[512] = {0};

void boot_main()
{
    display_init();
    display_clear();
    printf("Hell... Wait! It's not time!\n");
    ide_init();
    disk_mbr_init();
    disk_print_info();
    while(1);
}