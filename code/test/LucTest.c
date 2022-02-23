#include "syscall.h"

int main() { 
    int i = ReadInt();
    PrintInt(i);
    int r = RandomNum();
    PrintInt(r);
    Halt();
    return 0;
}