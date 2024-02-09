#include <type.h>

struct page_dir_entry
{
    u32 present : 1;
    u32 writable : 1;
    u32 super : 1;
    u32 write_through : 1;
    u32 cache_disable : 1;
    u32 accessed : 1;
    u32 resv : 6;
    u32 frame : 20;
} __attribute__((packed));

struct page_table_entry
{
    u32 present : 1;
    u32 writable : 1;
    u32 super : 1;
    u32 write_through : 1;
    u32 cache_disable : 1;
    u32 accessed : 1;
    u32 dirty : 1;
    u32 resv : 5;
    u32 frame : 20;
} __attribute__((packed));

static inline u32 page_get_dir_paddr()
{
    u32 paddr = 0;
    asm volatile("movl %%cr3, %0"
                 : "=r"(paddr));
    return paddr;
}

u32 page_get_paddr(u32 vaddr)
{
    u32 paddr = 0;
    
}