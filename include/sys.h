#define PAGE_SIZE 4096
#define KERNEL_DPL 0
#define USER_DPL 3
#define KERNEL_CODE_SEL (0x08 | KERNEL_DPL)
#define KERNEL_DATA_SEL (0x10 | KERNEL_DPL)
#define USER_CODE_SEL (0x18 | USER_DPL)
#define USER_DATA_SEL (0x20 | USER_DPL)

#define PAGE_SIZE_BIT 12
#define PAGE_NUM(addr) ((addr) >> PAGE_SIZE_BIT)
#define PAGE_OFF(addr) ((addr) & ((1 << PAGE_SIZE_BIT) - 1))
#define PAGE_FRAME(addr) PAGE_NUM(addr)
#define PAGE_CNT(size) (ROUND_UP(size, PAGE_SIZE) >> PAGE_SIZE_BIT)

#define KERNEL_VADDR_BASE 0xc0000000

void panic(char* msg);