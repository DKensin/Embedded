
#include <stdio.h>
#include "withdraw.h"

void withdraw_presenter_show(
    const WithdrawResponse *response)
{
    if (response == NULL)
        return;

    switch (response->status)
    {
        case WITHDRAW_SUCCESS:
            printf("Withdrawal successful.\n");
            printf("New balance: %lld cents\n",
                   response->new_balance);
            break;

        case WITHDRAW_INVALID_AMOUNT:
            printf("Invalid withdrawal amount.\n");
            break;

        case WITHDRAW_ACCOUNT_NOT_FOUND:
            printf("Account not found.\n");
            break;

        case WITHDRAW_INSUFFICIENT_FUNDS:
            printf("Insufficient funds.\n");
            break;

        case WITHDRAW_STORAGE_ERROR:
            printf("Unable to process transaction.\n");
            break;
    }
}
