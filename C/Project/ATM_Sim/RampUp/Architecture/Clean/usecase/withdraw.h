
#ifndef WITHDRAW_USE_CASE_H
#define WITHDRAW_USE_CASE_H

#include "account_repository.h"

typedef enum
{
    WITHDRAW_SUCCESS,
    WITHDRAW_INVALID_AMOUNT,
    WITHDRAW_ACCOUNT_NOT_FOUND,
    WITHDRAW_INSUFFICIENT_FUNDS,
    WITHDRAW_STORAGE_ERROR
} WithdrawStatus;

typedef struct
{
    const char *account_id;
    long long amount;
} WithdrawRequest;

typedef struct
{
    WithdrawStatus status;
    long long new_balance;
} WithdrawResponse;

WithdrawResponse withdraw_execute(
    AccountRepository *repository,
    const WithdrawRequest *request);

#endif
