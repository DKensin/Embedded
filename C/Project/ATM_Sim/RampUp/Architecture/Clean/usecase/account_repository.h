
#ifndef ACCOUNT_REPOSITORY_H
#define ACCOUNT_REPOSITORY_H

#include <stdbool.h>
#include "account.h"

typedef struct
{
    void *context;

    bool (*find_by_id)(
        void *context,
        const char *account_id,
        Account *out_account);

    bool (*save)(
        void *context,
        const Account *account);
} AccountRepository;    /* port that usecase/appliation use to access Account */

#endif /* ACCOUNT_REPOSITORY_H */
