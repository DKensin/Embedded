
#include "withdraw.h"

WithdrawResponse withdraw_controller_handle(
    AccountRepository *repository,
    const char *account_id,
    long long amount)
{
    WithdrawRequest request = {
        .account_id = account_id,
        .amount = amount
    };

    return withdraw_execute(repository, &request);
}
