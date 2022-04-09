#include "syscall.h"

int main(){

    char str[255];
    int fileID;
    PrintString("Filename: ");
    ReadString(str, 33);

    fileID = Open(str);
    
    if (fileID != -1){
        Close(fileID);
        Remove(str);
        PrintString("Remove file successfully\n");
    }
    else{
        PrintString("Invalid file\n");
    }

}