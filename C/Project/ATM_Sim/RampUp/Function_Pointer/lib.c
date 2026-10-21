#include <stdio.h>

static void default_operation(void)
{
    printf("this is a default function\n");
}

static void (*local_ptr)(void) = default_operation; /*declare variable*/

void register_operation(void (*ptr)(void))
{
    local_ptr = ptr;
}
void operation(void)
{
    local_ptr();
}