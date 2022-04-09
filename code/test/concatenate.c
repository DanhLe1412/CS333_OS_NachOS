#include "syscall.h"

#define MAXSIZE 32
int main()
{
    char c;
    char filename_src1[255];
    char filename_src2[255];
    char filename_dst[255];
    char buffer[500];
    int filenameLength;
    int fileid;
    int filesize;
    int i;
    int j;

    PrintString("Input filename 1: ");
    ReadString(filename_src1, MAXSIZE);
    PrintString("Input filename 2: ");
    ReadString(filename_src2, MAXSIZE);
    PrintString("Input dst: ");
    ReadString(filename_dst, MAXSIZE);

    fileid = Open(filename_src1);
    if (fileid == -1)
    {
        PrintString("Error reading file: ");
        PrintString(filename_src1);
        Halt();
    }

    filesize = Seek(-1, fileid);
    Seek(0, fileid);

    i = 0;
    for (; i < filesize; i++)
    {
        Read(&c, 1, fileid);
        buffer[i] = c;
    }

    Close(fileid);

    fileid = Open(filename_src2);
    if (fileid == -1)
    {
        PrintString("Error reading file: ");
        PrintString(filename_src2);
        Halt();
    }
    filesize = Seek(-1, fileid);
    Seek(0, fileid);

    j = 0;
    for (; j < filesize; j++)
    {
        Read(&c, 1, fileid);
        buffer[i + j] = c;
    }
    Close(fileid);

    fileid = Open(filename_dst);
    if (fileid == -1)
    {
        if (Create(filename_dst) == -1) // try to create file
        {
            PrintString("Error reading and creating file: ");
            PrintString(filename_dst);
            PrintString("\n");
            Halt();
        }
    }

    Write(buffer, i + j, fileid);

    Close(fileid);

    Halt();
}