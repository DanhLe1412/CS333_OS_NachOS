// a place to test out newly created system calls

#include "syscall.h"

int main()
{
    // enter your testing code from here
    char str[255];

    ReadString(str, 255);
    PrintString("The string that we read: ");
    PrintString(str);
    Halt();
    // end of testing code
}