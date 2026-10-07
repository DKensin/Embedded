#include <stdio.h>
#include <stdbool.h>
#include "array.h"

static void display_menu(void)
{
    printf("======ARRAY OPERANTION=========\n");
    printf("1. Enter position\n");
    printf("2. Display value\n");
    printf("3. Quit\n");
    printf("Enter your choice (1-3): ");
}

void show_menu(void)
{
    int choice;
    int pos;
    int value;
    int run = 1;

    do
    {
        display_menu();
        if(scanf("%d", &choice) != 1)
        {
            printf("Invalid input! Please enter a number (1-3)\n");

            while (getchar() != '\n');
            continue;
        }

        switch (choice)
        {
        case 1:
            printf("Enter position: ");
            if(scanf("%d", &pos) != 1)
            {
                printf("Invalid number!\n");
                while (getchar() != '\n');
                break;
            }
            printf("Enter value: ");
            if(scanf("%d", &value) != 1)
            {
                printf("Invalid number!\n");
                while (getchar() != '\n');
                break;
            }
            if (enter_arr(pos, value))
            {
                printf("Added successfull\n");
            }
            else
            {
                printf("Position already or invalid position\n");
            }
            break;
        case 2:
            print_arr();
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
