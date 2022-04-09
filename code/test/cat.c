#include "syscall.h"
#define BUFSIZE 100
int main()
{
    OpenFileId fid;
    char buffer[BUFSIZE];
    int n;

    char str[255];
    PrintString("Filename: ");

    ReadString(str, 33);
    fid = Open(str);
    while ((n = Read(buffer, BUFSIZE, fid)) > 0)
    {
        Write(buffer, n, ConsoleOut);
    }
    Close(fid);
    Halt();
}