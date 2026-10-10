#ifndef ATM_H
#define ATM_H

#include "account_store.h"

typedef enum
{
    ATM_OK = 0,
    ATM_INVALID_AMOUNT,
    ATM_INSUFFICIENT_FUNDS,
    ATM_STORAGE_ERROR
} AtmResult;

AtmResult atm_withdraw(AccountStore *store, int amount);

#endif /* ATM_H */
