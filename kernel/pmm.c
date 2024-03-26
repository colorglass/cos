#include <type.h>
#include <utils.h>
#include <sys.h>
#include <bitmap.h>
#include <list.h>
#include <mem.h>
#include <kmalloc.h>
#include <panic.h>
#include <pmm.h>

struct pmm {
    u32 ram_frames;
    u32 avails;
    struct bitmap bitmap;
    struct pmem_map* mem_maps;
    size_t map_entries;
};

// 4GB / 4KB = 1M
#define PMM_FRAME_NUMS 0x100000
#define PMM_BITMAP_SIZE (PMM_FRAME_NUMS / 8)
static u8 bits[PMM_BITMAP_SIZE] __attribute__((aligned(PAGE_SIZE)));
static struct pmm pmm;

void pmm_init(struct mem_map* mem_map)
{
    bitmap_init(&pmm.bitmap, bits, PMM_FRAME_NUMS);
    bitmap_set_all(&pmm.bitmap);

    pmm.mem_maps = (struct pmem_map*)kmalloc(sizeof(struct pmem_map) * mem_map->size);
    pmm.map_entries = mem_map->size;
    for(int i = 0; i < mem_map->size; i++) {
        u32 start = (u32)mem_map->maps[i].base;
        u32 length = (u32)mem_map->maps[i].length;

        pmm.mem_maps[i].start = start;
        pmm.mem_maps[i].length = length;
        pmm.mem_maps[i].type = mem_map->maps[i].type;

        if(mem_map->maps[i].type == MEM_TYPE_AVAILABLE) {
            bitmap_clear_range(&pmm.bitmap, PAGE_NUM(start), PAGE_CNT(length));
            pmm.ram_frames += PAGE_CNT(length);
        }
    }
    pmm.avails = pmm.ram_frames;
}

uintptr_t pmm_alloc(u32 count)
{
    if(count > pmm.avails) {
        panic("frame not enough");
    }
    int index = bitmap_scan_zero_set(&pmm.bitmap, count);
    if(index == -1) {
        panic("can not find continuous frames");
    }

    pmm.avails -= count;
    return index * PAGE_SIZE;
}

void pmm_free(uintptr_t addr, u32 count)
{
    bitmap_clear_range(&pmm.bitmap, PAGE_NUM(addr), count);
    pmm.avails += count;
}

void pmm_debug()
{
    printf("pmm: %d/%d\n", pmm.avails, pmm.ram_frames);
    printf("physical memory map:\n");
    for(int i = 0; i < pmm.map_entries; i++) {
        printf("start: 0x%x, length: 0x%x, type: %d\n", pmm.mem_maps[i].start, pmm.mem_maps[i].length, pmm.mem_maps[i].type);
    }
}

struct pmem_map* pmm_get_kernel_pmap()
{
    for(int i = 0; i < pmm.map_entries; i++) {
        if(pmm.mem_maps[i].type == MEM_TYPE_KERNEL) {
            return &pmm.mem_maps[i];
        }
    }
}
