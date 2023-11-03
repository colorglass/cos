extern int printf(const char *str, ...);

int kernel_main()
{
    display_init();
    display_clear();
    printf("This is kernel!\n");
    while(1);
}