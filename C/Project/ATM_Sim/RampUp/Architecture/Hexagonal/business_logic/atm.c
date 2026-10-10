#include "atm.h"
#include <stdio.h>

/**
 * The Business Logic interact with external system via Ports
 * In the business logic, we don't see any direct access to external world,
 * like console (to get amount), or open text file (access database)
 * so the business logic decouple with UI or database
 */
AtmResult atm_withdraw(AccountStore *store, int amount)
{
    AtmResult result = ATM_OK;
    int balance;

    if (!store->load_balance(&balance))             /* report storage error if cannot retrieve data */
    {
        printf("Load error\n");
        result = ATM_STORAGE_ERROR;
    }
    else if (amount < 0)                            /* check if amount invalid */
    {
        result = ATM_INVALID_AMOUNT;
    }
    else if (amount > balance)
    {
        result = ATM_INSUFFICIENT_FUNDS;             /* check if amount exceed the current balance */
    }
    else
    {
        balance -= amount;                          /* updat amount after withdraw */
        if (!store->save_balance(balance))          /* request save the new balance */
        {
            printf("Save error\n");
            result = ATM_STORAGE_ERROR;
        }
    }

    return result;
}