
#include "withdraw.h"

#include <stddef.h>
#include "account.h"

WithdrawResponse withdraw_execute(
    AccountRepository *repository,
    const WithdrawRequest *request)
{
    WithdrawResponse response = {
        .status = WITHDRAW_STORAGE_ERROR,
        .new_balance = 0
    };

    if (repository == NULL || request == NULL ||
        request->account_id == NULL)
        return response;

    if (request->amount <= 0)
    {
        response.status = WITHDRAW_INVALID_AMOUNT;
        return response;
    }

    Account account;

    if (!repository->find_by_id(
            repository->context,
            request->account_id,
            &account))
    {
        response.status = WITHDRAW_ACCOUNT_NOT_FOUND;
        return response;
    }

    AccountResult result =
        account_withdraw(&account, request->amount);

    if (result == ACCOUNT_INVALID_AMOUNT)
    {
        response.status = WITHDRAW_INVALID_AMOUNT;
        return response;
    }

    if (result == ACCOUNT_INSUFFICIENT_FUNDS)
    {
        response.status = WITHDRAW_INSUFFICIENT_FUNDS;
        return response;
    }

    if (!repository->save(repository->context, &account))
    {
        response.status = WITHDRAW_STORAGE_ERROR;
        return response;
    }

    response.status = WITHDRAW_SUCCESS;
    response.new_balance = account.balance;

    return response;
}
