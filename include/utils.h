#ifndef __UTILS_H__
#define __UTILS_H__

#include "type.h"

enum mem_type {
    MEM_TYPE_KERNEL = 0,
    MEM_TYPE_AVAILABLE = 1,
    MEM_TYPE_RESERVED = 2,
    MEM_TYPE_ACPI_RECLAIMABLE = 3,
    MEM_TYPE_ACPI_NVS = 4,
};

// should not use the u64 member directly since the architecure is 32 bits
struct mem_region {
    u64 base;
    u64 length;
    u32 type;
} __attribute__((packed));

struct mem_map {
    u32 size;
    struct mem_region maps[];
} __attribute__((packed));

#define NULL 0

#define BIT(n) (1 << (n))
#define BITS(l, r) ((BIT((l) - (r) + 1) - 1) << (r))

#define ALIGN_UP(x, n) (((x) + (n) - 1) & ~((n) - 1))
#define ALIGN_DOWN(x, n) ((x) & ~((n) - 1))

#define outb(port, val) asm ("outb %%al, %%dx" : : "a"(val), "d"(port))
#define outw(port, val) asm ("outw %%ax, %%dx" : : "a"(val), "d"(port))
#define outl(port, val) asm ("outl %%eax, %%dx" : : "a"(val), "d"(port))
#define inb(port) ({ u8 __v; asm ("inb %%dx, %%al" : "=a"(__v) : "d"(port)); __v; })
#define inw(port) ({ u16 __v; asm ("inw %%dx, %%ax" : "=a"(__v) : "d"(port)); __v; })
#define inl(port) ({ u32 __v; asm ("inl %%dx, %%eax" : "=a"(__v) : "d"(port)); __v; })

typedef char *va_list;
#define va_arg(ap, T) (*(T *)(((ap) += sizeof(T)) - sizeof(T)))
#define va_end(ap) ((ap) = (void *)NULL)
#define va_start(ap, parmN) ((ap) = (char *)(&(parmN)) + sizeof(parmN))
#define va_copy(dest, src) ((dest) = (src))

#define ctzl(x) __builtin_ctzl(x)
#define clzl(x) __builtin_clzl(x)

#endif