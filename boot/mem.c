#include <type.h>
#include <utils.h>
#include <boot.h>

extern u8 _mem_map[];

static struct mem_map* mem_map;


// init memory map from bios
int mem_map_init()
{
    mem_map = (struct mem_map*)&_mem_map[0];

    if(!mem_map || mem_map->size == 0)
        return -1;

    // reserved all the memory in the first 1MB
    for(int i=0;i<mem_map->size;i++) {
        if(mem_map->maps[i].base < 0x100000)
            mem_map->maps[i].type = MEM_TYPE_RESERVED;
        printf("base: %x, length: %x, type: %x\n", (u32)mem_map->maps[i].base, (u32)mem_map->maps[i].length, mem_map->maps[i].type);
    }
    
    return 0;
}

static int mem_find_available(u32 size)
{
    for(int i = 0; i < mem_map->size; i++) {
        if(mem_map->maps[i].type != MEM_TYPE_AVAILABLE)
            continue;

        if(mem_map->maps[i].length >= size) {
            return i;
        }
    }
    return -1;
}

// assert all the memory value (size, boundary) is aligned to page size
u32 mem_find_kernel_available(u32 size)
{
    int idx = mem_find_available(size);

    if(idx < 0)
        return -1;

    u32 region_end = mem_map->maps[idx].base + size;

    // now we only want kernel to be loaded under the first 4MB memory region
    if(region_end > 0x400000)
        return -1;

    if(mem_map->maps[idx].length > size) {
        for(int i = mem_map->size; i > idx + 1; i--) {
            mem_map->maps[i] = mem_map->maps[i - 1];
        }

        mem_map->maps[idx + 1].base = mem_map->maps[idx].base + size;
        mem_map->maps[idx + 1].length = mem_map->maps[idx].length - size;
        mem_map->maps[idx + 1].type = MEM_TYPE_AVAILABLE;

        mem_map->maps[idx].length = size;
        mem_map->size++;
    }

    mem_map->maps[idx].type = MEM_TYPE_KERNEL;
    return mem_map->maps[idx].base;
}