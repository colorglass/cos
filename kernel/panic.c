#include <printf.h>

void panic(char* msg) {
    printf("Kernel panic! --> %s\n", msg);
    asm volatile ("cli; hlt");
    while(1);
}