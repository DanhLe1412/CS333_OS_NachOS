// a place to test out newly created system calls

#include "syscall.h"

int main()
{
    int a, b, state, fileID1, fileID2, written, read;
    int filesize;
    char c;
    char str[255], str2[255];
    char anotherName[] = "Hello everyone!";

    // enter your testing code from here
    // PrintString("Enter a number: ");
    // a = ReadNum();
    // PrintString("The number entered: ");
    // PrintNum(a);
    // PrintString("\n");

    // PrintString("A random number: ");
    // b = RandomNum();
    // PrintNum(b);
    // PrintString("\n");

    // PrintString("Enter a character: ");
    // c = ReadChar();
    // ReadChar();
    // PrintString("The character entered: ");
    // PrintChar(c);
    // PrintString("\n");

    // PrintString("Enter a string: ");
    // ReadString(str, 255);
    // PrintString("The string entered: ");
    // PrintString(str);
    // PrintString("\n");

    PrintString("Filename: ");

    ReadString(str, 33);
    state = Create(str);
    if (state >= 0)
        PrintString("File created successfully!\n");
    else
        PrintString("Failed to create file!\n");

    fileID1 = Open(str);

    written = Write(anotherName, 29, fileID1);

    filesize = Seek(-1, fileID1);
    PrintNum(filesize);
    written = Write(anotherName, 29, fileID1);

    Seek(0, fileID1);
    read = Read(str2, 29, fileID1);

    PrintString(str2);
    PrintString("\n");
    PrintNum(read);
    PrintString("\n");

    Close(fileID1);

    // Remove(str);

    Halt();
    // end of testing code
}