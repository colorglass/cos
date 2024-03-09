#include <type.h>
#include <utils.h>
#include <sys.h>
#include <bitmap.h>
#include <list.h>
#include <mem.h>

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

// preserved buffer for setup physical memory manager
// 4KB bitmap can represent 4 * 8 * 4KB = 128MB memory
#define PRE_RAM_BUFFER_SIZE 1024
static u8 pre_ram_buffer[PRE_RAM_BUFFER_SIZE] __attribute__((aligned(4096)));
static u8 pre_ram_bitmap_buffer[PAGE_NUM(PRE_RAM_BUFFER_SIZE) / 8];
static struct ram_block pre_ram_block;
static struct ram_info ram_info;

void* p2v(u32 paddr) {
    return (void*)(paddr + pv_offset);
}

u32 v2p(void* vaddr) {
    return (u32)vaddr - pv_offset;
}

u32 ram_get_pv_offset(struct mem_map* mem_map)
{
    for(int i = 0; i < mem_map->size; i++) {
        if(mem_map->maps[i].type == MEM_TYPE_KERNEL) {
            return KERNEL_VADDR_BASE - (u32)mem_map->maps[i].base;
        }
    }
    return 0;
}

void* frame_alloc(u32 num)
{
    u32 alloced_frame_start = 0;
    if(ram_info.frames < num) {
        return NULL;
    }

    struct ram_block* block;
    list_for_each_entry(block, &ram_info.head, list) {
        if(block->avail_frames < num)
            continue;

        int bit_offset = bitmap_scan_zero_set(&block->bitmap, num);
        if(bit_offset < 0)
            continue;

        alloced_frame_start = bit_offset + block->start;
        block->avail_frames -= num;
        break;
    }

    // if block is not found
    if(alloced_frame_start == 0) 
        return NULL;

    // here block is always significant
    if(list_is_last(&block->list, &ram_info.head)) {
        return p2v(alloced_frame_start);
    }

    // [todo] map frame into kernel's virtual space
    return NULL;
}

void ram_pre_init(struct mem_map* mem_map)
{
}
