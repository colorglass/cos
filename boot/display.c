#include <type.h>
#include <mem.h>

static int cursor_row;
static int cursor_col;
static u8 c_attribute;

// row - column : 25 - 80
static volatile u16 (*v_buffer)[80];

int display_init()
{
    cursor_row = 0;
    cursor_col = 0;
    c_attribute = 0x0f;
    v_buffer = (volatile u16(*)[80])0xB8000;
    return 0;
}

int display_clear()
{
    memset((void *)v_buffer, 0, 80 * 25 * 2);
    cursor_col = 0;
    cursor_row = 0;
    return 0;
}

static void scroll_up()
{
    memcpy((void *)v_buffer, (void *)(v_buffer + 1), 80 * 24 * 2);
    memset((void *)(v_buffer + 24), 0, 80 * 2);
    cursor_row--;
}

void putchar(char c)
{
    // the char cursor out of row, scroll screen up
    if (cursor_row >= 25)
    {
        scroll_up();
    }

    if (c == '\n')
    {
        cursor_row++;
        cursor_col = 0;
        return;
    }

    v_buffer[cursor_row][cursor_col] = (u16)(c | c_attribute << 8);
    cursor_col++;

    // the cursor out of col, wrap to next line start
    if (cursor_col >= 80)
    {
        cursor_row++;
        cursor_col = 0;
    }
}

void puts(const char *s)
{
    for (int i = 0; s[i] != '\0'; i++)
    {
        putchar(s[i]);
    }
}


