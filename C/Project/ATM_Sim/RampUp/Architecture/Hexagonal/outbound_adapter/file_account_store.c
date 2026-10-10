#include <stdio.h>
#include "account_store.h"

/**
 * Adapter - real implementation
 * This is a implementation to store data into a text file.
 */
static int file_load_balance(int *balance)
{
    FILE *fp = fopen("account.txt", "r");

    if (fp == NULL) {
        printf("Open error\n");
        return 0;
    }

    int result = fscanf(fp, "%d", balance);
    fclose(fp);

    return result == 1;
}

static int file_save_balance(int balance)
{
    FILE *fp = fopen("account.txt", "w");

    if (fp == NULL) {
        return 0;
    }

    int result = fprintf(fp, "%d\n", balance);
    int close_result = fclose(fp);

    return result >= 0 && close_result == 0;
}

AccountStore file_account_store_create(void)
{
    AccountStore store = {
        .load_balance = file_load_balance,
        .save_balance = file_save_balance
    };

    return store;
}