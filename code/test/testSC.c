// a place to test out newly created system calls

#include "syscall.h"

int main()
{
    int a, b;
    char c;
    char str[255];

    // enter your testing code from here
    PrintString("Enter a number: ");
    a = ReadNum();
    PrintString("The number entered: ");
    PrintNum(a);
    PrintString("\n");

    PrintString("A random number: ");
    b = RandomNum();
    PrintNum(b);
    PrintString("\n");

    PrintString("Enter a character: ");
    c = ReadChar();
    ReadChar();
    PrintString("The character entered: ");
    PrintChar(c);
    PrintString("\n");

    PrintString("Enter a string: ");
    ReadString(str, 255);
    PrintString("The string entered: ");
    PrintString(str);
    PrintString("\n");

    Halt();
    // end of testing code
}