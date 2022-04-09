// exception.cc
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.
//
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1996 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "main.h"
#include "syscall.h"
#include "ksyscall.h"
#include "synchconsole.h"
#include "filesys.h"
#include <stdlib.h>
#include <string.h>

void IncreasePC()
{
	/* set previous programm counter (debugging only)*/
	kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));

	/* set programm counter to next instruction (all Instructions are 4 byte wide)*/
	kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);

	/* set next programm counter for brach execution */
	kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg) + 4);
}

/*
Input: - User space address (int)
 - Limit of buffer (int)
Output:- Buffer (char*)
Purpose: Copy buffer from User memory space to System memory space
*/
char *User2System(int virtAddr, int limit)
{
	int i; // index
	int oneChar;
	char *kernelBuf = NULL;

	kernelBuf = new char[limit + 1]; // need for terminal string
	if (kernelBuf == NULL)
		return kernelBuf;
	memset(kernelBuf, 0, limit + 1);

	for (i = 0; i < limit; i++)
	{
		kernel->machine->ReadMem(virtAddr + i, 1, &oneChar);
		kernelBuf[i] = (char)oneChar;
		if (oneChar == 0)
			break;
	}
	return kernelBuf;
}

/*
Input: - User space address (int)
 - Limit of buffer (int)
 - Buffer (char[])
Output:- Number of bytes copied (int)
Purpose: Copy buffer from System memory space to User memory space
*/
int System2User(int virtAddr, int len, char *buffer)
{
	if (len < 0)
		return -1;
	if (len == 0)
		return len;
	int i = 0;
	int oneChar = 0;
	do
	{
		oneChar = (int)buffer[i];
		kernel->machine->WriteMem(virtAddr + i, 1, oneChar);
		i++;
	} while (i < len && oneChar != 0);
	return i;
}

//----------------------------------------------------------------------
// ExceptionHandler
// 	Entry point into the Nachos kernel.  Called when a user program
//	is executing, and either does a syscall, or generates an addressing
//	or arithmetic exception.
//
// 	For system calls, the following is the calling convention:
//
// 	system call code -- r2
//		arg1 -- r4
//		arg2 -- r5
//		arg3 -- r6
//		arg4 -- r7
//
//	The result of the system call, if any, must be put back into r2.
//
// If you are handling a system call, don't forget to increment the pc
// before returning. (Or else you'll loop making the same system call forever!)
//
//	"which" is the kind of exception.  The list of possible exceptions
//	is in machine.h.
//----------------------------------------------------------------------

void ExceptionHandler(ExceptionType which)
{
	int type = kernel->machine->ReadRegister(2);

	DEBUG(dbgSys, "Received Exception " << which << " type: " << type << "\n");

	switch (which)
	{
	case SyscallException:
		switch (type)
		{
		case SC_Halt:
			DEBUG(dbgSys, "Shutdown, initiated by user program.\n");

			SysHalt();
			ASSERTNOTREACHED();
			break;

		case SC_Add:
			DEBUG(dbgSys, "Add " << kernel->machine->ReadRegister(4) << " + " << kernel->machine->ReadRegister(5) << "\n");

			/* Process SysAdd Systemcall*/
			int result;
			result = SysAdd(/* int op1 */ (int)kernel->machine->ReadRegister(4),
							/* int op2 */ (int)kernel->machine->ReadRegister(5));

			DEBUG(dbgSys, "Add returning with " << result << "\n");
			/* Prepare Result */
			kernel->machine->WriteRegister(2, (int)result);

			IncreasePC();
			return;

			ASSERTNOTREACHED();

			break;
		// New code starts from here
		case SC_PrintNum:
		{
			int output = kernel->machine->ReadRegister(4);

			if (output == 0) // If the integer is 0, simply output 0.
			{
				kernel->synchConsoleOut->PutChar('0');
				IncreasePC();
				return;
			}

			if (output < 0) // If the integer is negative, add a '-' sign
				kernel->synchConsoleOut->PutChar('-');
			output = abs(output); // Make sure the integer is now positive
			int temp;
			char *buf = new char[12];
			int counter = 0;
			while (output > 0) // We store the integer starting from the lowest digit
			{
				temp = output % 10;
				output /= 10;
				buf[counter] = (char)(temp + '0');
				counter++;
			}
			while (counter >= 0)
				kernel->synchConsoleOut->PutChar((char)buf[--counter]); // Simply output from the end of the buffer (from the largest digit)
			IncreasePC();
			delete buf;
			return;
			ASSERTNOTREACHED();
		}
		break;
		case SC_RandomNum:
		{
			srandom(time(NULL)); // Set random seed
			int r = random();	 // Generate a random number
			kernel->machine->WriteRegister(2, r);
			IncreasePC();
			return;
			ASSERTNOTREACHED();
		}
		break;
		case SC_ReadNum:
		{
			int sz = 0, sta = 0;
			bool isNum = true;
			char str[13]; // maximum length of an integer is 9, +2 for the terminating null and the sign

			char c = (char)kernel->synchConsoleIn->GetChar();
			while (c != '\n')
			{
				str[sz++] = c;
				if (sz > 11) // If there are over 11 characters in the input, then it is overflowed
				{
					isNum = false;
					DEBUG(dbgSys, "Integer overflow\n");
					break;
				}
				c = (char)kernel->synchConsoleIn->GetChar();
			}
			str[sz] = '\0'; // Terminating null

			if (str[0] == '-') // Check if the user enter a negative integer
				sta++;		   // The integer start from index 1

			// check valid all-number string
			for (int i = sta; i < sz; i++)
				if (!(str[i] >= '0' && str[i] <= '9')) // If the input contains characters other than numbers, break
				{
					isNum = false;
					break;
				}

			if (isNum)
			{
				long num = 0; // long, not int, a number with 9 digits can still cause integer overflow, long long will help containing the value
				for (int i = sta; i < sz; i++)
					num = num * 10 + (long)(str[i] - '0');
				if (sta == 1)
					num = -num;

				if (num >= (long)(-2147483648) && num <= (long)(2147483647)) // the value is in the integer range in C
				{
					DEBUG(dbgSys, "Received a number: " << num << endl);
				}
				else
				{
					DEBUG(dbgSys, "Integer overflow\n");
					num = 0;
				}
				kernel->machine->WriteRegister(2, (int)num);
			}
			else
			{
				DEBUG(dbgSys, "Not a valid number.\n");
				kernel->machine->WriteRegister(2, 0);
			}
			IncreasePC();
			return;
			ASSERTNOTREACHED();
		}
		break;
		case SC_ReadChar:
		{
			char c = (char)kernel->synchConsoleIn->GetChar();
			if (c >= '!' && c <= '~') // only considering displayable ASCII characters
			{
				DEBUG(dbgSys, "The system read: \'" << c << "\' from the console.\n");
				kernel->machine->WriteRegister(2, (char)c);
			}
			else
			{
				DEBUG(dbgSys, "Not a displayable character!");
				kernel->machine->WriteRegister(2, (char)'\0');
			}

			IncreasePC();
			return;
			ASSERTNOTREACHED();
		}
		break;
		case SC_PrintChar:
		{
			char c = (char)kernel->machine->ReadRegister(4);
			if (c >= ' ' && c <= '~') // only considering displayable ASCII characters
			{
				DEBUG(dbgSys, "Received character \'" << c << "\' as the first parameter.\n");
				kernel->synchConsoleOut->PutChar(c);
			}
			else
			{
				DEBUG(dbgSys, "Received a non-displayable character as the first parameter\n");
				kernel->synchConsoleOut->PutChar('\0');
			}
			// kernel->synchConsoleOut->PutChar('\n');
			IncreasePC();
			return;
			ASSERTNOTREACHED();
		}
		break;
		case SC_ReadString:
		{
			int virtualAddr = (int)kernel->machine->ReadRegister(4);
			int len = (int)kernel->machine->ReadRegister(5);
			if (len < 0)
			{
				DEBUG(dbgSys, "Invalid Parameter!\n");
			}
			char *buffer = User2System(virtualAddr, len + 1);
			int pos = 0;
			char c = (char)kernel->synchConsoleIn->GetChar();
			while (c != '\n' && c != '\0' && pos < len)
			{
				buffer[pos] = c;
				pos++;
				c = (char)kernel->synchConsoleIn->GetChar();
			}
			buffer[pos] = '\0';

			System2User(virtualAddr, len, buffer);

			delete[] buffer;

			IncreasePC();
			return;
			ASSERTNOTREACHED();
		}
		break;
		case SC_PrintString:
		{
			// get string address from r4
			int strAddr = (int)kernel->machine->ReadRegister(4);
			// get the string from User memory space to System memory space
			char *buffer = User2System(strAddr, 255);

			if (buffer != NULL)
			{
				int pos = 0;
				while (buffer[pos] != 0)
				{
					kernel->synchConsoleOut->PutChar(buffer[pos]);
					pos++;
				}
				// kernel->synchConsoleOut->PutChar(end);
			}
			else
			{
				DEBUG(dbgSys, "Error locating the string from user.\n");
			}
			IncreasePC();
			return;
			ASSERTNOTREACHED();
		}
		break;

		case SC_Create:
		{
			// input: arg1 as the file name address
			// output: successful: 0, failed: -1
			// function: create a empty file with given file name from the user.
			// get string address from r4
			int strAddr = (int)kernel->machine->ReadRegister(4);
			// get the string from User memory space to System memory space
			char *kernelBuffer = User2System(strAddr, 33);

			if (strlen(kernelBuffer) == 0)
			{
				DEBUG(dbgSys, "Invalid file name (file name with size 0)!\n");
				kernel->machine->WriteRegister(2, -1);
			}
			else
			{
				if (kernelBuffer != NULL)
				{
					if (!kernel->fileSystem->Create(kernelBuffer)) // return true if the file is created successfully
					{
						DEBUG(dbgSys, "Failed to create file\n");
						kernel->machine->WriteRegister(2, -1);
					}
					else
						kernel->machine->WriteRegister(2, 0);
				}
				else
				{
					DEBUG(dbgSys, "Error locating the string from user or not enough memory in the system.\n");
					kernel->machine->WriteRegister(2, -1);
				}
			}
			delete kernelBuffer;
			IncreasePC();
			return;
			ASSERTNOTREACHED();
		}
		break;

		case SC_Open:
		{
			// input: arg1(reg4) as the file name address
			// output: int - the OpenFileId of the file that just got opened, -1 if failed
			// function: open a file with a given file name from the user, return the OpenFileId of that file
			// OpenFileId for stdinput: input 0
			// OpenFileId for stdoutput: output 1

			const char *stdin = "stdin";
			const char *stdout = "stdout";
			int strAddr = (int)kernel->machine->ReadRegister(4);
			// get the string from User memory space to System memory space
			char *kernelBuffer = User2System(strAddr, 33);
			int freeSlot = (int)kernel->fileSystem->findFreeSlot();

			DEBUG(dbgSys, "\nAttempt to Open " << kernelBuffer << "$\n");
			DEBUG(dbgSys, "\nfree slot:  " << freeSlot << "\n");

			// if (strcmp(kernelBuffer, stdin)) // the user's input is "stdin" or "stdout"
			// 	kernel->machine->WriteRegister(2, 0);
			// else
			// {
			// 	if (strcmp(kernelBuffer, stdout))
			// 		kernel->machine->WriteRegister(2, 1);
			// 	else
			{
				if (freeSlot > 1) // not -1, 0 and 1
				{
					if ((kernel->fileSystem->openedFiles[freeSlot] = kernel->fileSystem->Open(kernelBuffer)) != NULL)
					{
						kernel->machine->WriteRegister(2, (int)freeSlot);
						DEBUG(dbgSys, "Open " << kernelBuffer << " successfully!\n");
					}
					else
					{
						kernel->machine->WriteRegister(2, (int)-1);
						DEBUG(dbgSys, "Failed to open " << kernelBuffer << "!\n");
					}
				}
				else
				{
					DEBUG(dbgSys, "Too much file is opened right now!\n");
					kernel->machine->WriteRegister(2, (int)-1);
				}
				//}
			}
			delete[] kernelBuffer;
			IncreasePC();
			return;
			ASSERTNOTREACHED();
		}
		break;

		case SC_Close:
		{
			// input: arg1(reg4) as the OpenFileId of the file that needs to be closed.
			// output: 0 if successful, -1 if failed
			// function: close a file with the specific OpenFileId
			// OpenFileId for stdinput: input 0
			// OpenFileId for stdoutput: output 1

			int fileID = (int)kernel->machine->ReadRegister(4);

			if (fileID >= 0 && fileID <= 14) // in the right range of value
			{
				if (kernel->fileSystem->openedFiles[fileID])
				{
					DEBUG(dbgSys, "\nfree slot:  " << fileID << "\n");
					delete kernel->fileSystem->openedFiles[fileID];
					kernel->fileSystem->openedFiles[fileID] = NULL;
					DEBUG(dbgSys, "File closed successfully\n");
					kernel->machine->WriteRegister(2, (int)0);
				}
				else
				{
					DEBUG(dbgSys, "The file with the given ID is already closed.\n");
					kernel->machine->WriteRegister(2, (int)-1);
				}
			}
			else
			{
				DEBUG(dbgSys, "Invalid file ID.\n");
				kernel->machine->WriteRegister(2, (int)-1);
			}

			IncreasePC();
			return;
			ASSERTNOTREACHED();
		}
		break;

		case SC_Write:
		{
			// input: arg1(reg4) - string buffer from user-space, arg2(reg5) - the length of that string, arg3(reg6) integer index of an opened file.
			// output: success: the number of bytes written, failure: -1
			// function: write content into a file

			int strAddr = (int)kernel->machine->ReadRegister(4);
			int strLen = (int)kernel->machine->ReadRegister(5);
			int fileID = (int)kernel->machine->ReadRegister(6);
			char *kernelBuffer;
			bool checked = false;
			int bytesWritten;

			if (fileID < 0 || fileID > 14)
			{
				DEBUG(dbgSys, "Out of range: file ID.\n");
				kernel->machine->WriteRegister(2, -1);
				checked = true;
			}

			if (!checked && kernel->fileSystem->openedFiles[fileID] == NULL)
			{
				DEBUG(dbgSys, "The file ID does not exist.\n");
				kernel->machine->WriteRegister(2, -1);
				checked = true;
			}

			if (!checked && fileID == 0) // stdin
			{
				DEBUG(dbgSys, "Can't write to stdin\n");
				kernel->machine->WriteRegister(2, -1);
				checked = true;
			}

			if (checked)
			{
				IncreasePC();
				return;
			}

			kernelBuffer = User2System(strAddr, strLen);
			strLen = min(strLen, (int)strlen(kernelBuffer));

			if (fileID == 1) // stdout - write to stdout
			{
				int rs = 0;
				if (kernelBuffer != NULL)
				{
					while (kernelBuffer[rs] != 0 && kernelBuffer[rs] != '\n')
					{
						kernel->synchConsoleOut->PutChar(kernelBuffer[rs]);
						rs++;
					}
					kernel->synchConsoleOut->PutChar('\n');
				}
				kernel->machine->WriteRegister(2, rs - 1);
				DEBUG(dbgSys, "Wrote to stdout successfully!\n");
			}
			else // write to an opened file
			{
				bytesWritten = kernel->fileSystem->openedFiles[fileID]->Write(kernelBuffer, strLen);
				if (bytesWritten >= 0)
				{
					kernel->machine->WriteRegister(2, bytesWritten);
					DEBUG(dbgSys, "Wrote to file successfully!\n");
				}
				else
				{
					kernel->machine->WriteRegister(2, -1);
					DEBUG(dbgSys, "Failed to write to file.\n");
				}
			}
			delete[] kernelBuffer;
			IncreasePC();
			return;
			ASSERTNOTREACHED();
		}
		break;

		case SC_Read:
		{
			// input: arg1(reg4) - string buffer from user-space, arg2(reg5) - size of the content the the user wants to read from the opened file, arg3(reg6) - integer index of the opended file
			// output: the number of bytes read, can be less than the required size. Larger than 1 for I/O device.
			// function: read a string of character from a file.

			int strAddr = (int)kernel->machine->ReadRegister(4);
			int strLen = (int)kernel->machine->ReadRegister(5);
			int fileID = (int)kernel->machine->ReadRegister(6);
			char *kernelBuffer;
			bool checked = false;
			int bytesRead;

			if (fileID < 0 || fileID > 14)
			{
				DEBUG(dbgSys, "Out of range: file ID.\n");
				kernel->machine->WriteRegister(2, -1);
				checked = true;
			}

			if (!checked && kernel->fileSystem->openedFiles[fileID] == NULL)
			{
				DEBUG(dbgSys, "The file ID does not exist.\n");
				kernel->machine->WriteRegister(2, -1);
				checked = true;
			}

			if (!checked && fileID == 1) // stdout
			{
				DEBUG(dbgSys, "Can't read from stdout\n");
				kernel->machine->WriteRegister(2, -1);
				checked = true;
			}

			if (checked)
			{
				IncreasePC();
				return;
			}

			kernelBuffer = User2System(strAddr, strLen);

			if (fileID == 0) // read from the stdin
			{
				int rs = 0;
				char c = (char)kernel->synchConsoleIn->GetChar();
				while (c != '\n' && c != '\0' && rs < strLen)
				{
					kernelBuffer[rs] = c;
					rs++;
					c = (char)kernel->synchConsoleIn->GetChar();
				}
				kernelBuffer[rs] = '\0';
				System2User(strAddr, rs, kernelBuffer);
				kernel->machine->WriteRegister(2, rs);
				DEBUG(dbgSys, "Read from stdin successfully!\n");
			}
			else // read from an opened file
			{
				bytesRead = kernel->fileSystem->openedFiles[fileID]->Read(kernelBuffer, strLen);
				if (bytesRead >= 0)
				{
					System2User(strAddr, bytesRead, kernelBuffer);
					kernel->machine->WriteRegister(2, bytesRead);
					DEBUG(dbgSys, "Read from file successfully!\n");
				}
				else
				{
					kernel->machine->WriteRegister(2, -1);
					DEBUG(dbgSys, "Failed to read from file.\n");
				}
			}

			delete[] kernelBuffer;
			IncreasePC();
			return;
			ASSERTNOTREACHED();
		}
		break;

		case SC_Seek:
		{
			int position = kernel->machine->ReadRegister(4);
			OpenFileId id = kernel->machine->ReadRegister(5);

			// Assuming there is a function call openf with a length of sector size of 16

			if (position < -1 || id < 2 || id > 16 || kernel->fileSystem->openedFiles[id] == NULL)
			{
				kernel->machine->WriteRegister(2, -1);
				DEBUG(dbgSys, "position < 0 || id < 2 || id > 16 .\n");
				IncreasePC();
				return;
			}

			if (position == -1)
			{
				position = kernel->fileSystem->openedFiles[id]->Length();
				DEBUG(dbgSys, "position == -1.\n");
			}

			if (position > kernel->fileSystem->openedFiles[id]->Length() || position < 0)
			{
				kernel->machine->WriteRegister(2, -1);
				DEBUG(dbgSys, "position > kernel->fileSystem->openedFiles[id]->Length().\n");
			}
			else
			{
				kernel->fileSystem->openedFiles[id]->Seek(position);
				kernel->machine->WriteRegister(2, position);
				DEBUG(dbgSys, "mental breakdown.\n");
			}

			IncreasePC();
			return;
			ASSERTNOTREACHED();
		}
		break;

		case SC_Remove:
		{
			int virtualAddr = (int)kernel->machine->ReadRegister(4);
			char *filename;
			filename = User2System(virtualAddr, 256);

			if (filename == NULL || strlen(filename) < 0)
			{
				DEBUG(dbgSys, "Invalid file name.");
				kernel->machine->WriteRegister(2, -1);
				IncreasePC();
				return;
			}

			// int freeSlot = kernel->fileSystem->findFreeSlot();

			// Need to check if the file is opening

			if (kernel->fileSystem->Remove(filename))
			{
				DEBUG(dbgSys, "Succesfully removed.");
				kernel->machine->WriteRegister(2, 1);
			}
			else
			{
				DEBUG(dbgSys, "Failed to remove.");
				kernel->machine->WriteRegister(2, -1);
			}

			delete[] filename;
			IncreasePC();
			return;
			ASSERTNOTREACHED();
		}
		default:
			cerr << "Unexpected system call " << type << "\n";
			break;
		}

	case PageFaultException:
		printf("\nNo valid translation found.\n");
		SysHalt();
		break;

	case ReadOnlyException:
		printf("\nWrite attempted to page marked \"read-only\".\n");
		SysHalt();
		break;

	case BusErrorException:
		printf("\nTranslation resulted in an invalid physical address.\n");
		SysHalt();
		break;

	case AddressErrorException:
		printf("\nUnaligned reference or one that was beyond the end of the address space.\n");
		SysHalt();
		break;

	case OverflowException:
		printf("\nInteger overflow in add or sub.\n");
		SysHalt();
		break;

	case IllegalInstrException:
		printf("\nUnimplemented or reserved instr\n");
		SysHalt();
		break;

	case NumExceptionTypes:
		printf("\nNumExceptionTypes\n");
		SysHalt();
		break;
	default:
		cerr << "Unexpected user mode exception" << (int)which << "\n";
		SysHalt();
		break;
	}
	ASSERTNOTREACHED();
}
