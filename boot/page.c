#include <boot.h>
#include <type.h>
#include <mem.h>

#define PAGE_SIZE_BIT 12
#define PAGE_NUM(addr) ((addr) >> PAGE_SIZE_BIT)
#define PAGE_OFF(addr) ((addr) & ((1 << PAGE_SIZE_BIT) - 1))
#define PAGE_FRAME(addr) PAGE_NUM(addr)

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

static struct page_dir_entry page_dir[1024] __attribute__((aligned(4096)));
static struct page_table_entry page_table[1024] __attribute__((aligned(4096)));

// identical map the first 1MB memory
// no irq handler now, so be sure there will be no pagefault
void page_init()
{

    // the struct may already be zeroed
    // memset(page_dir, 0, sizeof(page_dir));
    // memset(page_table, 0, sizeof(page_table));

    // setup page table
    // identity map low 1MB memory
    for (int page = 0; page < PAGE_NUM(0x100000); page++) {
        page_table[page].present = 1;
        page_table[page].writable = 1;
        page_table[page].super = 1;

        page_table[page].frame = page; // identity mapping
    }

    // setup page directory
    page_dir[0].present = 1;
    page_dir[0].writable = 1;
    page_dir[0].super = 1;
    page_dir[0].frame = PAGE_FRAME((u32)page_table); // for now we are still in the physical world, so we use the address directly

    // enable paging
    u32 cr3 = PAGE_FRAME((u32)page_dir) << 12;
    asm volatile("movl %0, %%cr3" ::"r"(cr3));
    asm volatile("movl %%cr0, %%eax \n"
                 "orl $0x80000000, %%eax \n"
                 "movl %%eax, %%cr0 \n" ::: "eax");
}

// map identiacal virtual address to physical address
u32 page_map_identical(u32 paddr)
{
    u32 vaddr = paddr;
    u32 page = PAGE_NUM(vaddr);

    // not support memory upper than 4MB
    if(page >= 1024)
        return 0;

    // if the page is already mapped, return the virtual address
    if(page_table[page].present)
        return vaddr;

    page_table[page].present = 1;
    page_table[page].writable = 1;
    page_table[page].super = 1;
    page_table[page].frame = PAGE_FRAME(paddr);

    return vaddr;
}