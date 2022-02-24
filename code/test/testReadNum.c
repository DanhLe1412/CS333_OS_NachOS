#include <syscall.h>

int main()
{
    int i = ReadNum();
    PrintNum(i);
    Halt();
    return 0;
}