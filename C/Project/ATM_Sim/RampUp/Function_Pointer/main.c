#include <stdio.h>
#include "lib.h"

void customer_operation(void)
{
    printf("Hello, this is my custom function\n");
}

int main(void)
{
    operation();
    register_operation(customer_operation);
    operation();

    return 0;
}