#ifndef __MEM_H__
#define __MEM_H__

static void *memcpy(void *dest, const void *src, int n)
{
    char *d = (char *)dest;
    const char *s = (const char *)src;
    while (n--)
        *d++ = *s++;
    return dest;
}

static void *memset(void *s, char c, int n)
{
    char *p = (char *)s;
    while (n--)
        *p++ = c;
    return s;
}

#endif