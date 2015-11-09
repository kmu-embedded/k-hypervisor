int init_khypervisor(void)
{
    int num; 
    //long long base;
    long base;
    num = 10;
    base = 2;
    num = num / base;
    num = num % base;

    init_print();
    printH("test\n");

    return 0;
}
