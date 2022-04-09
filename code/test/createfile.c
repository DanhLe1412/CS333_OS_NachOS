#include "syscall.h"
#define MAX_LENGTH 32

int main()
{
    char str[255];
    int state;
    PrintString("Filename: ");

    ReadString(str, 33);
    state = Create(str);
    if(state >= 0)
        PrintString("File created successfully!\n");
    else
        PrintString("Failed to create file!\n");
}