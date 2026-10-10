#ifndef WITHDRAW_CONTROLLER_H
#define WITHDRAW_CONTROLLER_H

#include "withdraw.h"

WithdrawResponse withdraw_controller_handle(
    AccountRepository *repository,
    const char *account_id,
    long long amount);

#endif
