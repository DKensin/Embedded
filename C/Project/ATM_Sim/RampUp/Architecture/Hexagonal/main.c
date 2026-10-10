#include <stdio.h>
#include <string.h>
#include "file_account_store.h"
#include "memory_account_store.h"
#include "console.h"

/**
 * The main itself is the Inbound adapter: read user input, call business logic
 */
int main(int argc, char *argv[])
{
    AccountStore store;

    /**
     * One Port can have multiple Adapters.
     * Based on the condition, switch the appropriate Adapter.
     */
    if (argc > 1 &&
        strcmp(argv[1], "memory") == 0)
    {
        printf("Memory Adapter calling...\n");
        store = memory_account_store_create();
    }
    else
    {
        printf("File Adapter calling...\n");
        store = file_account_store_create();
    }

    console_run(&store);

    return 0;
}