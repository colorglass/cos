#include <boot.h>
#include <type.h>
#include <mem.h>
#include <utils.h>

#define PAGE_SIZE_BIT 12
#define PAGE_NUM(addr) ((addr) >> PAGE_SIZE_BIT)
#define PAGE_OFF(addr) ((addr) & ((1 << PAGE_SIZE_BIT) - 1))
#define KERNEL_VADDR_BASE 0xc0000000

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

static struct page_dir_entry page_dir[1024] __attribute__((aligned(PAGE_SIZE)));
static struct page_table_entry page_table[1024] __attribute__((aligned(PAGE_SIZE)));
static struct page_table_entry page_table_kernel[1024] __attribute__((aligned(PAGE_SIZE)));

// identical map the first 1MB memory
// no irq handler now, so be sure there will be no pagefault
void page_init()
{

    // the struct may already be zeroed
    memset(page_dir, 0, sizeof(page_dir));
    memset(page_table, 0, sizeof(page_table));
    memset(page_table_kernel, 0, sizeof(page_table_kernel));

    // setup page table
    // identity map low 4MB memory
    for (int page = 0; page < PAGE_NUM(0x400000); page++) {
        page_table[page].present = 1;
        page_table[page].writable = 1;
        page_table[page].super = 1;

        page_table[page].frame = page; // identity mapping
    }

    // setup page directory
    page_dir[0].present = 1;
    page_dir[0].writable = 1;
    page_dir[0].super = 1;
    page_dir[0].frame = PAGE_NUM((u32)page_table); // for now we are still in the physical world, so we use the address directly

    // map kernel page table
    u32 kernel_page_table_idx = PAGE_NUM(0xc0000000) / 1024;
    page_dir[kernel_page_table_idx].present = 1;
    page_dir[kernel_page_table_idx].writable = 1;
    page_dir[kernel_page_table_idx].super = 1;
    page_dir[kernel_page_table_idx].frame = PAGE_NUM((u32)page_table_kernel);

    // enable paging
    u32 cr3 = PAGE_NUM((u32)page_dir) << 12;
    asm volatile("movl %0, %%cr3" ::"r"(cr3));
    asm volatile("movl %%cr0, %%eax \n"
                 "orl $0x80000000, %%eax \n"
                 "movl %%eax, %%cr0 \n" ::: "eax");
}

// temporarily map kernel into the max 4MB from 0xc0000000
u32 page_map_kernel_pages(u32 paddr, u32 vaddr, u32 size, bool writeable)
{
    u32 start_page = PAGE_NUM(vaddr) - PAGE_NUM(KERNEL_VADDR_BASE);
    u32 page_nums = PAGE_NUM(ALIGN_UP(size, PAGE_SIZE));

    if(start_page + page_nums > 1024)
        return 0;

    for (int i = 0; i < page_nums; i++) {
        page_table_kernel[start_page + i].present = 1;
        page_table_kernel[start_page + i].writable = writeable? 1 : 0;
        page_table_kernel[start_page + i].super = 1;
        page_table_kernel[start_page + i].frame = PAGE_NUM(paddr) + i;
    }

    return vaddr;
}