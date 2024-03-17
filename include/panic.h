#include <printf.h>

static inline void panic(char* msg)
{
    printf("Kernel panic! --> %s\n", msg);
    asm volatile ("cli; hlt");
    while(1);
}

static inline void assert(int cond, char* msg)
{
    if(!cond)
        panic(msg);
}