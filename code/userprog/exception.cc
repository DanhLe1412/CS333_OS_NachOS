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
		// New code from here
		/*
		case SC_ReadInt:
			int input = 0, last = 0, i = 0, j, m;
			bool pos = true, isInt = true;
			char c;
			char* buffer = new char[256];

			DEBUG(dbgSys, "\n SC_ReadInt\n");
			DEBUG(dbgSys, "Reading an integer from input, please start typing:\n");
			printf("\n SC_ReadInt\n");
			printf("Reading an integer from input, please start typing:\n");

			SynchConsoleInput *sci = new SynchConsoleInput(NULL);
			// Read the input to a buffer.
			c = sci->GetChar();
			while(c != '\n' && c != EOF) {
				buffer[last++] = c;
				c = sci->GetChar();
			}
			buffer[last] = '\0';

			// Preprocess the buffer
			if (buffer[0] == '-') {
				pos = false;
				i = 1;
			}

			m = i;
			for (i; i < last; i++) {

				if (buffer[0] < '0' || buffer[0] > '9') {
					printf("This is not an integer\n");
					DEBUG(dbgSys, "This is not an integer\n");
					isInt = false;
				}

				if (!isInt) {
					kernel->machine->WriteRegister(2, 0);
					IncreasePC();
					delete sci;
					delete buffer;
					return;
				}
			}

			for (m; m < last; m++) {
				input = input * 10 + (int)(buffer[m] - '0');
			}

			if(!pos)
				input *= -1;

			// Prepare for return
			DEBUG(dbgSys, "System has read the integer\n");
			printf("System has read integer: %d\n", input);
			kernel->machine->WriteRegister(2, input);
			IncreasePC();
			delete sci;
			delete buffer;
			return;
			ASSERTNOTREACHED();
			break;

		case SC_PrintInt:
			DEBUG(dbgSys, "\n SC_PrintInt\n");
			DEBUG(dbgSys, "Printing an integer\n");
			printf("\n SC_PrintInt\n");
			printf("Printing an integer.\n");

			SynchConsoleOutput *sco = new SynchConsoleOutput(NULL);
			int output = kernel->machine->ReadRegister(4);

			if (output == 0)
			{
				sco->PutChar('0');
				IncreasePC();
				delete sco;
				return;
			}

			if (output < 0)
				sco->PutChar('-');
			output = abs(output);
			int temp;
			while (output > 0)
			{
				temp = output % 10;
				output /= 10;
				sco->PutChar((char)(temp + '0'));
			}
			IncreasePC();
			delete sco;
			return;
			ASSERTNOTREACHED();
			break;

		case SC_RandomNum:
			DEBUG(dbgSys, "\n SC_RandomNum\n");
			DEBUG(dbgSys, "Generate a random positive integer\n");
			printf("\n SC_RandomNum\n");
			printf("Generate a random positive integer\n");
			int r = random();
			kernel->machine->WriteRegister(2, r);
			IncreasePC();
			return;
			ASSERTNOTREACHED();
			break;
			*/
		case SC_ReadChar:
		{
			SynchConsoleInput *synchConsoleInput = new SynchConsoleInput(NULL);

			char c = synchConsoleInput->GetChar();
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

			delete synchConsoleInput;
			IncreasePC();
			return;
			ASSERTNOTREACHED();
		}
		break;
		case SC_PrintChar:
		{
			char c = (char)kernel->machine->ReadRegister(4);
			SynchConsoleOutput *synchConsoleOutput = new SynchConsoleOutput(NULL);
			if(c >= '!' && c <= '~') // only considering displayable ASCII characters
			{
				DEBUG(dbgSys, "Received character \'" << c << "\' as the first parameter.\n");
				synchConsoleOutput->PutChar(c);
				synchConsoleOutput->PutChar('\n');
			}
			else {
				DEBUG(dbgSys, "Received a non-displayable character as the first parameter\n");
				synchConsoleOutput->PutChar('\0');
				synchConsoleOutput->PutChar('\n');
			}
			delete synchConsoleOutput;
			IncreasePC();
			return;
			ASSERTNOTREACHED();
		}
		break;
		case SC_ReadString:
		{

		}
		break;
		case SC_PrintString:
		{

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
		break;
	}
	ASSERTNOTREACHED();
}
