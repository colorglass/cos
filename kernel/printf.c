#include <type.h>
#include <utils.h>

extern void putchar(char c);
extern void puts(const char *);

static char str_map[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};
static char str_buff[64];

static void to_str(int num, u8 base)
{
    int i = 0;
    int minus = 0;
    if (base == 10 && num < 0)
    {
        minus = 1;
        num = -num;
    }

    u32 n = num;

    do
    {
        str_buff[i++] = str_map[n % base];
        n /= base;
    } while (n);

    if (minus)
    {
        str_buff[i++] = '-';
    }

    int j = i;
    while (j-- > i / 2)
    {
        char t = str_buff[j];
        str_buff[j] = str_buff[i - j - 1];
        str_buff[i - j - 1] = t;
    }

    str_buff[i] = '\0';
}

int printf(const char *str, ...)
{

    if (!str)
        return -1;

    va_list args;
    va_start(args, str);

    // %d, %x, %s
    int base = 0;
    const char *s = str;
    while (*s)
    {
        if (*s != '%')
        {
            putchar(*s);
            goto next;
        }

        s++;
        if (*s == 's')
        {
            const char *ss = va_arg(args, const char *);
            puts(ss);
        }

        switch (*s)
        {
        case 'x':
            base = 16;
            break;
        case 'd':
            base = 10;
            break;
        case 'b':
            base = 2;
            break;
        default:
            base = 0;
            break;
        }

        if (base)
        {
            int num = va_arg(args, int);
            to_str(num, base);
            puts(str_buff);
        }

    next:
        s++;
    }

    return 0;
}