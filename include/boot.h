#include <type.h>

#define SECTOR_SIZE 512
#define PAGE_SIZE 0x1000

int display_init();
int display_clear();
void putchar(char c);
void puts(const char *s);
int printf(const char *str, ...);

int ide_init();
u32 ide_pio_read(void* dest, u64 sec_start, u16 sec_count);
u32 ide_pio_write(void* src, u64 sec_start, u16 sec_count);

int disk_init();
int disk_find_next_bootable(int next);
int disk_read(int parti_id, void* dest, u64 sec_start, u16 sec_count);
int disk_write(int parti_id, void* src, u64 sec_start, u16 sec_count);

void fat_init(int parti_id);
void fat_print_fat();
u32 fat_load_kernel();

u32 elf_get_mem_size(char* file);
u32 elf_load_kernel(char* file, u32 load_paddr);

int mem_map_init();
u32 mem_find_kernel_available(u32 size);

void page_init();
u32 page_map_kernel_pages(u32 paddr, u32 vaddr, u32 size, bool writeable);