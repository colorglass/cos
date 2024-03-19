#include <bitmap.h>

void bitmap_init(struct bitmap *bitmap, void* buffer, u32 size)
{
    bitmap->bits = (u32*)buffer;
    bitmap->size = size;
    for(int i = 0; i <= bitmap->size / 32; i++) {
        bitmap->bits[i] = 0;
    }
}

void bitmap_set(struct bitmap *bitmap, u32 index)
{
    if (index > bitmap->size)
        return;

    bitmap->bits[index / 32] |= (1 << (index % 32));
}

// slow implementation
void bitmap_set_range(struct bitmap *bitmap, u32 index, u32 count)
{
    for(int i = 0; i < count; i++) {
        bitmap_set(bitmap, index + i);
    }
}

void bitmap_clear(struct bitmap *bitmap, u32 index)
{
    if (index > bitmap->size)
        return;

    bitmap->bits[index / 32] &= ~(1 << (index % 32));
}

// slow implementation
void bitmap_clear_range(struct bitmap *bitmap, u32 index, u32 count)
{
    for(int i = 0; i < count; i++) {
        bitmap_clear(bitmap, index + i);
    }
}

int bitmap_test(struct bitmap *bitmap, u32 index)
{
    if (index > bitmap->size)
        return -1;

    return bitmap->bits[index / 32] & (1 << (index % 32));
}

int bitmap_scan_zero(struct bitmap* bitmap, u32 count)
{
    int i, j;
    for(i = 0; i < bitmap->size / 32; i++) {
        if(bitmap->bits[i] != 0xffffffff) {
            u32 mask = (1 << count) - 1;
            for(j = 0; j <= 32 - count; j++) {
                if((bitmap->bits[i] & (mask << j)) == 0) {
                    return i * 32 + j;
                }
            }
        }
    }
    return -1;
}

int bitmap_scan_zero_set(struct bitmap* bitmap, u32 count) 
{
    int rst = bitmap_scan_zero(bitmap, count);
    if(rst != -1) {
        for(int i = 0; i < count; i++) {
            bitmap_set(bitmap, rst + i);
        }
    }
    return rst;
}