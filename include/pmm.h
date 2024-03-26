#include <type.h>
#include <sys.h>

struct pmem_map {
    u32 start;
    u32 length;
    u32 type;
};

void pmm_init(struct mem_map* mem_map);
uintptr_t pmm_alloc(u32 count);
void pmm_free(uintptr_t addr, u32 count);
struct pmem_map* pmm_get_kernel_pmap();
void pmm_debug();