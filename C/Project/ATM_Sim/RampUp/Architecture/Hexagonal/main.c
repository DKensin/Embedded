#include <stdio.h>
#include "atm.h"
#include "file_account_store.h"

/**
 * The main itself is the Inbound adapter: read user input, call business logic
 */
int main(void)
{
    AccountStore store = file_account_store_create();
    int amount;

    printf("Enter withdrawal amount: ");

    if (scanf("%d", &amount) != 1)
    {
        printf("Invalid input.\n");
        return 1;
    }

    AtmResult result = atm_withdraw(&store, amount);    /* Inbound Port: call business logic */

    switch (result)                                     /* display the result */
    {
    case ATM_OK:
        printf("Withdrawal successful.\n");
        break;

    case ATM_INVALID_AMOUNT:
        printf("Invalid withdrawal amount.\n");
        break;

    case ATM_INSUFFICIENT_FUNDS:
        printf("Insufficient funds.\n");
        break;

    case ATM_STORAGE_ERROR:
        printf("Storage error.\n");
        break;
    }

    return 0;
}