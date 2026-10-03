#include <stdio.h>
#include <stdbool.h> /*tell the complier to include the stdio.h header file*/
#include "array.h"

void display_menu(void)
{
    printf("======ARRAY OPERANTION=========\n");
    printf("1. Enter position\n");
    printf("2. Display value\n");
    printf("3. Quit\n");
    printf("Enter your choice (1-3): ");
}

void menu(void)
{
    int choice;
    int pos;
    int value;
    int run = 1;

    do
    {
        display_menu();
        scanf("%d", &choice);

        switch (choice)
        {
        case 1:
            printf("Enter position: ");
            scanf("%d", &pos);
            printf("Enter value: ");
            scanf("%d", &value);
            if (enter_pos(pos, value))
            {
                printf("Added successfull\n");
            }
            else
            {
                printf("Position already has a value\n");
            }
            break;
        case 2:
            display_value();
            printf("\n");
            break;
        case 3:
            run = 0;
            break;
        default:
            printf("Please enter value(1-3): \n");
            break;
        }

    } while (run);
}
