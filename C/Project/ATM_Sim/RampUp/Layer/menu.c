#include <stdio.h>
#include <stdbool.h>
#include "array.h"

static void display_menu(void)
{
    printf("======ARRAY OPERATION=========\n");
    printf("1. Enter position\n");
    printf("2. Display value\n");
    printf("3. Quit\n");
    printf("Enter your choice (1-3): ");
}

static int get_number(void)
{
    int number_input;
    char next_char;

    while (scanf("%d%c", &number_input, &next_char) != 2 || next_char != '\n')
    {
        printf("Input value. Please try again\n");
        while (getchar() != '\n');
    }

    return number_input;
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
        choice = get_number();

        switch (choice)
        {
        case 1:
            printf("Enter position: ");
            pos = get_number();

            printf("Enter value: ");
            value = get_number();

            if (enter_array(pos, value))
            {
                printf("Added successfull\n\n");
            }
            else
            {
                printf("Position already or invalid position\n\n");
            }
            break;
        case 2:
            print_array();
            printf("\n\n");
            break;
        case 3:
            run = 0;
            break;
        default:
            printf("Please enter value(1-3) \n\n");
            break;
        }
    } while (run);
}
