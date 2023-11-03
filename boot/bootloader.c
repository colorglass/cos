
extern int printf(const char *str, ...);
extern int display_init();
extern int display_clear();

void boot_main()
{
    display_init();
    display_clear();
    printf("Hell... Wait! It's not time!\n");

}