#include "syscall.h"
// #include "stdio.h"
// #include <stdlib.h>
#define isAscending 1

void bubble_sort(int *arr, int size, int type)
{
    int step;
    for (step = 0; step < size - 1; ++step)
    {

        int swapped = 0;
        int i;
        for (i = 0; i < size - step - 1; ++i)
        {

            if (type == isAscending)
            {
                if (arr[i] > arr[i + 1])
                {

                    int temp = arr[i];
                    arr[i] = arr[i + 1];
                    arr[i + 1] = temp;
                    swapped = 1;
                }
            }
            else
            {
                if (arr[i] < arr[i + 1])
                {

                    int temp = arr[i];
                    arr[i] = arr[i + 1];
                    arr[i + 1] = temp;
                    swapped = 1;
                }
            }
        }
    }
}


int main()
{
    int n = 101,i;
    int type;
    int arr[100];
    while (n > 100 || n < 0)
    {
        PrintString("Input size(0<n<=100): ");
        n = ReadNum();
    }
    
    PrintString("Type sorting (1 for ascending, other input is descending): ");
    type = ReadNum();
    

    for (i = 0; i < n; i++)
    {
        int tmp;
        PrintString("input: ");
        tmp = ReadNum();
        arr[i] = tmp;
    }

    bubble_sort(arr, n, type);
    for (i = 0; i < n; i++){
        PrintNum(arr[i]);
        PrintChar(' ');
    }
    PrintString("\n");
    Halt();
}