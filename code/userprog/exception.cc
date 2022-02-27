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
#include <stdlib.h>

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
			printf("\n SC_RandomNum\n");
			printf("Generate a random positive integer\n");
			srandom(time(NULL)); // Set random seed
			int r = random(); // Generate a random number
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
				sta++; // The integer start from index 1

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
			char* buffer = User2System(virtualAddr, len+1);
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
			else {
				DEBUG(dbgSys, "Error locating the string from user.\n");
			}

			IncreasePC();
			return;
			ASSERTNOTREACHED();
		}
		break;
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
