
#ifndef ACCOUNT_H
#define ACCOUNT_H

#include <stdbool.h>

typedef struct
{
    char account_id[32];
    long long balance;
} Account;

typedef enum
{
    ACCOUNT_OK,
    ACCOUNT_INVALID_AMOUNT,
    ACCOUNT_INSUFFICIENT_FUNDS
} AccountResult;

AccountResult account_withdraw(
    Account *account,
    long long amount
);

#endif /* ACCOUNT_H */
