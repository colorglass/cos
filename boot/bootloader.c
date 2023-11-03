#include <type.h>

extern int printf(const char *str, ...);
extern int display_init();
extern int display_clear();

extern int ide_init();
extern u32 ide_pio_read(u32 dest, u64 sec_start, u16 sec_count);
extern u32 ide_pio_write(u32 src, u32 size, u64 sec_start);

static unsigned char buff_1[512] = {[0 ... 511] = 0x55};
static unsigned char buff_2[512] = {0};

void boot_main()
{
    display_init();
    display_clear();
    printf("Hell... Wait! It's not time!\n");
    ide_init();
    ide_pio_write((u32)buff_1, 512, 0);
    ide_pio_read(buff_2, 0, 1);
    while(1);
}