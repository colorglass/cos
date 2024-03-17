#include <type.h>
#include <utils.h>
#include <sys.h>
#include <bitmap.h>
#include <list.h>
#include <mem.h>
#include <kmalloc.h>
#include <panic.h>

struct ram_block {
    u32 start;
    u32 frames;
    u32 avail_frames;
    struct bitmap bitmap;

    struct list_head list;
};

struct ram_info {
    u32 frames;

    struct list_head head;
};

u32 pv_offset;
struct mem_map* pmem_map;

extern u8 __kernel_end[];
static struct ram_info ram_info;

void* p2v(u32 paddr) {
    return (void*)(paddr + pv_offset);
}

u32 v2p(void* vaddr) {
    return (u32)vaddr - pv_offset;
}

void ram_init(struct mem_map* mem_map)
{
    // copy mem_map from bootloader into kernel
    int map_size = mem_map->size * sizeof(struct mem_region) + sizeof(struct mem_map);
    pmem_map = (struct mem_map*)kmalloc(map_size);
    assert(pmem_map, "alloc pmem_map failed");
    memcpy(pmem_map, mem_map, map_size);

    u32* bitmap_base = (u32*)ALIGN_UP((size_t)__kernel_end, PAGE_SIZE);
    list_init_head(&ram_info.head);

    for(int i = 0; i < pmem_map->size; i++) {
        if(pmem_map->maps[i].type == MEM_TYPE_AVAILABLE) {
            u32 frames = pmem_map->maps[i].length / PAGE_SIZE;  // assert length aligned to pagesize
            struct ram_block* ram_block = (struct ram_block*)kmalloc(sizeof(struct ram_block));
            assert(ram_block,"failed to alloc ram_block");
            ram_block->frames = frames;
            ram_block->avail_frames = frames;
            ram_block->start = pmem_map->maps[i].base;   // assert base aligned to pagesize
            bitmap_init(&ram_block->bitmap, bitmap_base, frames);
            bitmap_base += bitmap_buffer_size(&ram_block->bitmap);

            list_init_head(&ram_block->list);
            list_add(&ram_block->list, &ram_info.head);
            ram_info.frames += ram_block->frames;
        }
    }

    struct ram_block* block;
    list_for_each_entry(block, &ram_info.head, list) {
        if(block->start)
    }


}
