extern int printf(const char *str, ...);

int kernel_main()
{
    printf("This is kernel!\n");
    while(1);
}