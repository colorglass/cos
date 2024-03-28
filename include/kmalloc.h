#pragma once
#include <type.h>

#define ALLOC_REGION_TOP 0xf8000000

int kheap_init();
void* kmalloc(u32 size);
void kfree(void* ptr);
void vmm_init();
void* vmm_alloc(u32 size);
void vmm_free(void* ptr);

