#include <type.h>
#include <boot.h>

extern u8 _mem_map[];

struct mem_map_bios {
    u64 base;
    u64 length;
    u32 type;
} __attribute__((packed));

struct mem_map {
    u32 size;
    struct mem_map_bios maps[];
} __attribute__((packed));

static struct mem_map* mem_map;

int mem_map_init()
{
    mem_map = (struct mem_map*)&_mem_map[0];

    if(!mem_map || mem_map->size == 0)
        return -1;

    for(int i=0;i<mem_map->size;i++) {
        if(mem_map->maps[i].base < 0x100000)
            mem_map->maps[i].type = 2;
        printf("base: %x, length: %x, type: %x\n", (u32)mem_map->maps[i].base, (u32)mem_map->maps[i].length, mem_map->maps[i].type);
    }
    
    return 0;
}

static int i = 0, j = 0;

void* mem_frame_alloc()
{
    for(; i < mem_map->size; i++) {
        if(mem_map->maps[i].type != 1)
            continue;

        if((j << 12) >= mem_map->maps[i].length) {
            j = 0;
            continue;
        }

        return (void*)(mem_map->maps[i].base + (j++ << 12));
    }
}