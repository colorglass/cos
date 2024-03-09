#include <type.h>

struct bitmap {
    u32 *bits;
    // bitmap element count
    u32 size;
};

void bitmap_init(struct bitmap *bitmap, void* buffer, u32 size);
void bitmap_set(struct bitmap *bitmap, u32 index);
void bitmap_clear(struct bitmap *bitmap, u32 index);
int bitmap_test(struct bitmap *bitmap, u32 index);
int bitmap_scan_zero(struct bitmap* bitmap, u32 count);
int bitmap_scan_zero_set(struct bitmap* bitmap, u32 count);