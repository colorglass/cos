#include <type.h>

#define SECTOR_SIZE 512

int display_init();
int display_clear();
int printf(const char *str, ...);

int ide_init();
u32 ide_pio_read(u32 dest, u64 sec_start, u16 sec_count);
u32 ide_pio_write(u32 src, u64 sec_start, u16 sec_count);

int disk_mbr_init();
void disk_print_info();
int disk_find_next_bootable(int next);
int disk_read(int parti_id, u32 dest, u64 sec_start, u16 sec_count);
int disk_write(int parti_id, u32 src, u32 size, u64 sec_start);
