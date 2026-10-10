#include "memory_account_store.h"

static int memory_balance = 1000000;

static int memory_load_balance(int *balance)
{
    *balance = memory_balance;
    return 1;
}

static int memory_save_balance(int balance)
{
    memory_balance = balance;
    return 1;
}

/**
 * Adapter - real implementation
 * This is a implementation to store data into a memory (RAM).
 */
AccountStore memory_account_store_create(void)
{
    AccountStore store = {
        .load_balance = memory_load_balance,
        .save_balance = memory_save_balance
    };

    return store;
}