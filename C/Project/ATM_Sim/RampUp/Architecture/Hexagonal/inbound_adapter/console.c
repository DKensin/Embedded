#include <stdio.h>
#include "console.h"

void console_run(AccountStore *store)
{
    int amount;

    printf("Enter withdrawal amount: ");

    if (scanf("%d", &amount) != 1) {
        printf("Invalid input.\n");
        return;
    }

    AtmResult result = atm_withdraw(store, amount);

    switch (result) {
    case ATM_OK:
        printf("Withdrawal successful.\n");
        break;

    case ATM_INVALID_AMOUNT:
        printf("Invalid amount.\n");
        break;

    case ATM_INSUFFICIENT_FUNDS:
        printf("Insufficient funds.\n");
        break;

    case ATM_STORAGE_ERROR:
        printf("Storage error.\n");
        break;
    }
}