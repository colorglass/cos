#include <type.h>

#define BITMAP_UNIT_CAPACITY 32

struct bitmap {
    u32 *bits;
    // bitmap element count
    u32 size;
};

void bitmap_init(struct bitmap *bitmap, void* buffer, u32 size);
void bitmap_set(struct bitmap *bitmap, u32 index);
void bitmap_set_range(struct bitmap *bitmap, u32 index, u32 count);
void bitmap_clear(struct bitmap *bitmap, u32 index);
void bitmap_clear_range(struct bitmap *bitmap, u32 index, u32 count);
int bitmap_test(struct bitmap *bitmap, u32 index);
int bitmap_scan_zero(struct bitmap* bitmap, u32 count);
int bitmap_scan_zero_set(struct bitmap* bitmap, u32 count);

static inline size_t bitmap_buffer_size(struct bitmap* bitmap) 
{
    return (size_t)((bitmap->size + BITMAP_UNIT_CAPACITY - 1) / BITMAP_UNIT_CAPACITY);
}