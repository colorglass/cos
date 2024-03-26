#pragma once

#include <type.h>

#define PAGE_PRESENT (1 << 0)
#define PAGE_WRITE (1 << 1)
#define PAGE_USER (1 << 2)
#define KERNEL_PAGE_FLAGS (PAGE_PRESENT | PAGE_WRITE)
#define KERNEL_RO_PAGE_FLAGS (PAGE_PRESENT)
#define USER_PAGE_FLAGS (PAGE_PRESENT | PAGE_WRITE | PAGE_USER)
#define USER_RO_PAGE_FLAGS (PAGE_PRESENT | PAGE_USER)

void page_init();
void page_map(uintptr_t vaddr, uintptr_t paddr, u32 flags);
void page_unmap(uintptr_t vaddr);