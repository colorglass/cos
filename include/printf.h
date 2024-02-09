#ifndef __PRINTF_H__
#define __PRINTF_H__

int display_init();
int display_clear();
void putchar(char c);
void puts(const char *s);
int printf(const char *str, ...);

#endif 