#include "syscall.h"
// #include "stdio.h"
// #include <stdlib.h>
#define isAscending 1

void bubble_sort(int *arr, int size, int type)
{

    for (int step = 0; step < size - 1; ++step)
    {

        int swapped = 0;

        for (int i = 0; i < size - step - 1; ++i)
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

void printarr(int arr[], int size)
{
    // printf("arr: \n");
    int i;
    for (i = 0; i < size; i++)
        PrintNum(arr[i]);
    // printf("\n");
}

int main()
{
    int n = 101;
    while (n > 100 || n < 0)
    {
        // printf("Input size(0<n<=100): ");
        n = ReadNum();
    }
    int type;
    // printf("Type sorting (1:ascending, 0:descending): ");
    type = ReadNum();
    int *arr = (int *)malloc(n * sizeof(int));
    if (arr == NULL)
    {
        // printf("Memory not allocated.\n");
        exit(0);
    }

    for (int i = 0; i < n; i++)
    {
        int tmp;
        // printf("input: ");
        tmp = ReadNum();
        arr[i] = tmp;
    }

    bubble_sort(arr, n, type);
    printarr(arr, n);
    Halt();
}