#include <syscall.h>

int main()
{
    int i = RandomNum();
    PrintNum(i);
    Halt();
    return 0;
}