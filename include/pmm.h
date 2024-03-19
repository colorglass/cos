#include <type.h>
#include <sys.h>

extern size_t kpv_off;

static inline void* KP2V(uintptr_t paddr) {
    return (void*)(paddr + kpv_off);
}

static inline uintptr_t KV2P(void* vaddr) {
    return (uintptr_t)vaddr - kpv_off;
}

void pmm_init(struct mem_map* mem_map);
uintptr_t pmm_alloc(u32 count);
void pmm_free(uintptr_t addr, u32 count);
void pmm_debug();