#define TEST() asm("mov $0x01, %%eax; int $0x30" ::);

void main()
{
    TEST();

    while (1);
}