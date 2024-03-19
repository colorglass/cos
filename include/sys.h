#pragma once
#define KERNEL_DPL 0
#define USER_DPL 3
#define KERNEL_CODE_SEL (0x08 | KERNEL_DPL)
#define KERNEL_DATA_SEL (0x10 | KERNEL_DPL)
#define USER_CODE_SEL (0x18 | USER_DPL)
#define USER_DATA_SEL (0x20 | USER_DPL)

#define PAGE_SIZE_BIT 12
#define PAGE_SIZE (1 << PAGE_SIZE_BIT)
#define PAGE_NUM(addr) ((addr) >> PAGE_SIZE_BIT)
#define PAGE_OFF(addr) ((addr) & (PAGE_SIZE - 1))
#define PAGE_CNT(size) (ALIGN_UP(size, PAGE_SIZE) >> PAGE_SIZE_BIT)

#define KERNEL_VADDR_BASE 0xc0000000