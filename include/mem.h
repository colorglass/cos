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

static void *memset(void *s, unsigned char c, int n)
{
    char *p = (char *)s;
    while (n--)
        *p++ = c;
    return s;
}

static void* memcmp(const void* s1, const void* s2, int n)
{
    const char *p1 = (const char *)s1;
    const char *p2 = (const char *)s2;
    while (n--)
    {
        if (*p1 != *p2)
            return (void *)p1;
        p1++;
        p2++;
    }
    return 0;
}

#endif