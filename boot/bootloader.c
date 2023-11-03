
extern int printf(const char *str, ...);
extern int display_init();
extern int display_clear();

void boot_main()
{
    display_init();
    display_clear();
    const char *str4 = "%d %d %d %x %x %x %b %b %b\n";

    printf(str4, -1, -80, 79, 0xfffacc00, 65536, 0xffffffff, 0xf000, 0x0, 0x55aa);
    display_clear();
    while (1)
        ;
}