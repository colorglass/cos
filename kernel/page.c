#include <type.h>
#include <sys.h>
#include <utils.h>
#include <pmm.h>
#include <panic.h>
#include <page.h>

#define PAGE_DIR_IDX(vaddr) ((vaddr) >> 22)
#define PAGE_TABLE_IDX(vaddr) (((vaddr) >> 12) & 0x3ff)
#define PAGE_DIR_ADDR 0xfffff000
#define PAGE_TABLE_ADDR(vaddr) ((((vaddr) >> 10) | 0xffc00000) & 0xfffff000)

struct page_dir_entry
{
    u32 present : 1;
    u32 writable : 1;
    u32 user : 1;
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
    u32 user : 1;
    u32 write_through : 1;
    u32 cache_disable : 1;
    u32 accessed : 1;
    u32 dirty : 1;
    u32 resv : 5;
    u32 frame : 20;
} __attribute__((packed));

static struct page_dir_entry page_dir[1024] __attribute__((aligned(4096)));
static struct page_table_entry kpt[1024] __attribute__((aligned(4096)));
static struct page_table_entry lopt[1024] __attribute__((aligned(4096)));

static inline void TLB_invalidate_page(uintptr_t vaddr)
{
    asm volatile ("invlpg (%0)" ::"r" (vaddr) : "memory");
}

void page_init()
{
    struct pmem_map* kernel_pmap = pmm_get_kernel_pmap();
    size_t kpv_off = KERNEL_VADDR_BASE - kernel_pmap->start;
    u32 kwin_page_nums = PAGE_NUM(kernel_pmap->length);
    u32 pd_base_idx = PAGE_DIR_IDX(KERNEL_VADDR_BASE);

    assert(kwin_page_nums <= 1024, "kernel window too large");

    // set up kernel map window
    for(int i = 0; i < kwin_page_nums; i++) {
        kpt[i].present = 1;
        kpt[i].writable = 1;
        kpt[i].frame = PAGE_NUM(kernel_pmap->start) + i;
    }

    page_dir[pd_base_idx].present = 1;
    page_dir[pd_base_idx].writable = 1;
    page_dir[pd_base_idx].frame = PAGE_NUM((uintptr_t)kpt - kpv_off);

    // set up recursive mapping
    page_dir[1023].present = 1;
    page_dir[1023].writable = 1;
    page_dir[1023].frame = PAGE_NUM((uintptr_t)page_dir - kpv_off);

    // [todo]: remap the vga buffer into kernel space
    // here map the lower 1MB identically
    for(int i = 0; i < 256; i++) {
        lopt[i].present = 1;
        lopt[i].writable = 1;
        lopt[i].frame = i;
    }
    page_dir[0].present = 1;
    page_dir[0].writable = 1;
    page_dir[0].frame = PAGE_NUM((uintptr_t)lopt - kpv_off);

    // reload page directory
    uintptr_t pd_paddr = (uintptr_t)page_dir - kpv_off;
    asm volatile("mov %0, %%cr3" ::"r" (pd_paddr));
}

void page_map(uintptr_t vaddr, uintptr_t paddr, u32 flags)
{
    int pd_idx = PAGE_DIR_IDX(vaddr);
    int pt_idx = PAGE_TABLE_IDX(vaddr);

    struct page_dir_entry* cur_page_dir = (struct page_dir_entry*)PAGE_DIR_ADDR;

    if(cur_page_dir[pd_idx].present == 0) {
        uintptr_t page_table = pmm_alloc(1);
        cur_page_dir[pd_idx].present = 1;
        cur_page_dir[pd_idx].writable = 1;
        cur_page_dir[pd_idx].frame = PAGE_NUM(page_table);
    }

    struct page_table_entry* page_table = (struct page_table_entry*)PAGE_TABLE_ADDR(vaddr);

    if(page_table[pt_idx].present == 1) {
        panic("page already mapped");
    }

    page_table[pt_idx].present = 1;
    page_table[pt_idx].writable = !!(flags & PAGE_WRITE);
    page_table[pt_idx].user = !!(flags & PAGE_USER);
    page_table[pt_idx].frame = PAGE_NUM(paddr);
}

void page_unmap(uintptr_t vaddr)
{
    int pd_idx = PAGE_DIR_IDX(vaddr);
    int pt_idx = PAGE_TABLE_IDX(vaddr);

    struct page_dir_entry* cur_page_dir = (struct page_dir_entry*)PAGE_DIR_ADDR;
    if(cur_page_dir[pd_idx].present == 0) {
        panic("page not mapped");
    }

    struct page_table_entry* page_table = (struct page_table_entry*)PAGE_TABLE_ADDR(vaddr);
    if(page_table[pt_idx].present == 0) {
        panic("page not mapped");
    }

    page_table[pt_idx] = (struct page_table_entry){0};
    TLB_invalidate_page(vaddr);
}