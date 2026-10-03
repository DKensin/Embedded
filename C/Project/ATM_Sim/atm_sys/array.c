#include <stdio.h>
#include <stdbool.h>

#define SIZE        10

int arr[SIZE] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

bool enter_pos(int pos, int value)
{
    bool result = false; //duplicate position
    if (0xFF == arr[pos])
    {
        arr[pos] = value;
        result = true;
    }

    return result;
}

void display_value(void)
{
    printf("Printed value: ");

    for (int i = 0; i < SIZE; i++)
    {
        if ((arr[i]) != 0xFF)
        {
            printf("%d ", arr[i]);
        }
    }
}