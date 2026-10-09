#include <stdio.h>
#include <stdbool.h>

#define SIZE        10

static int arr[SIZE] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

bool enter_array(int pos, int value)
{
    bool result = false;

    if ((pos >= 0) && (pos < SIZE))
    {
        if (0xFF == arr[pos])
        {
            arr[pos] = value;
            result = true;
        }
    }
    return result;
}

void print_array(void)
{
    printf("Current value:: ");

    for (int i = 0; i < SIZE; i++)
    {
        if ((arr[i]) != 0xFF)
        {
            printf("%d ", arr[i]);
        }
    }
}