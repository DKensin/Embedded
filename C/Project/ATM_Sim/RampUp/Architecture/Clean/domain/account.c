#include "account.h"
#include <stddef.h>

AccountResult account_withdraw(
    Account *account,
    long long amount)
{
    if (account == NULL || amount <= 0)
        return ACCOUNT_INVALID_AMOUNT;

    if (account->balance < amount)
        return ACCOUNT_INSUFFICIENT_FUNDS;

    account->balance -= amount;

    return ACCOUNT_OK;
}
