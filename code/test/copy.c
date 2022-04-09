#include "syscall.h"
#define MAX_LENGTH 32

int main(){
    int srcFileID;
    int destFileID;
    int fileSize;
    int i;
    char c;
    char source[MAX_LENGTH];
    char dest[MAX_LENGTH];

    PrintString("Input src file: ");
    ReadString(source, MAX_LENGTH); 
	
	PrintString("Input dest file: ");
	ReadString(dest, MAX_LENGTH);
	srcFileID = Open(source);

    if (srcFileID != -1){
        destFileID = Create(dest);
        Close(destFileID);

        destFileID = Open(dest);
        if (destFileID != -1){
            fileSize = Seek(-1, srcFileID);
			

			Seek(0, srcFileID); 
			Seek(0, destFileID); 
			i = 0;
			
			for (; i < fileSize; i++) 
			{
				Read(&c, 1, srcFileID); 
				Write(&c, 1, destFileID); 
			}
			
			PrintString(" -> Copied successfully.\n\n");
			Close(destFileID); 
        }
        else{
            PrintString("Failed create dest file.");
        }
        Close(srcFileID);
    }
    else{
        PrintString("Invalid src file\n");
    }
}